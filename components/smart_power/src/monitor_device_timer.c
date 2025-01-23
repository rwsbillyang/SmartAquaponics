#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <inttypes.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include "common.h"
#include "monitor.h"
#include "monitor_device_timer.h"
#include "queue_task_loop.h"


const int32_t TimerMockGpioNum = -1; // no gpio input, mock it


//********************************************************************//
//*********************** Based on FreeRTOS Timer ****************************//
//********************************************************************//
// FreeRTOS Timer API reference: https://www.freertos.org/Documentation/02-Kernel/04-API-references/11-Software-timers/01-xTimerCreate

static void serialize(TimerSubObject *self)
{
    //TODO
}

static TimerSubObject *deserialize()
{
    //TODO
    return NULL;
}

static void createShiftTimers(TimerHandle_t xTimer);
static void init(Monitor *monitor)
{
    TimerSubObject *sub = (TimerSubObject *)(monitor->subObject);
    TimerHandle_t delayTimer = xTimerCreate("delayTimer",
                                            pdMS_TO_TICKS(sub->startDelayMs), // 只有在 configTICK_RATE_HZ 小于等于 1000 时，pdMS_TO_TICKS() 才可用
                                            pdFALSE,                          // true: The timers will auto-reload themselves when they expire.
                                            (void *)monitor,
                                            createShiftTimers // Each timer calls the same callback when it expires
    );
    if (delayTimer == NULL)
    {
        ESP_LOGW(TAG, "start: fail to create a delay timer of TimerMonitor");
        return;
    }
    sub->delayTimer = delayTimer;
}
static void start(Monitor *monitor){
    TimerSubObject *sub = (TimerSubObject *)(monitor->subObject);
    if (xTimerStart(sub->delayTimer, 0) != pdPASS)
    {
        /* The timer could not be set into the Active state. */
        ESP_LOGW(TAG, "init: fail to start a delay timer of TimerMonitor");
    }
}

/**
 * @brief start timer
 * @param monitor
 * @return
 */
static void uninit(Monitor *monitor)
{
    TimerSubObject *sub = (TimerSubObject *)(monitor->subObject);
    TimerHandle_t xTimer = sub->delayTimer;
    if (xTimer != NULL)
    {
        if (xTimerStop(xTimer, 0) != pdPASS)
        {
            ESP_LOGW(TAG, "cleanTimerTrigger: fail to xTimerStop delayTimer of TimerMonitor");
        }
        if (xTimerDelete(xTimer, 0) != pdPASS)
        {
            ESP_LOGW(TAG, "cleanTimerTrigger: fail to xTimerDelete delayTimer of TimerMonitor");
        }
        sub->delayTimer = NULL;
    }

    xTimer = sub->turnOnTimer;
    if (xTimer != NULL)
    {
        if (xTimerStop(xTimer, 0) != pdPASS)
        {
            ESP_LOGW(TAG, "cleanTimerTrigger: fail to xTimerStop turnOnTimer of TimerMonitor");
        }
        if (xTimerDelete(xTimer, 0) != pdPASS)
        {
            ESP_LOGW(TAG, "cleanTimerTrigger: fail to xTimerDelete turnOnTimer of TimerMonitor");
        }
        sub->turnOnTimer = NULL;
    }
    xTimer = sub->turnOffTimer;
    if (xTimer != NULL)
    {
        if (xTimerStop(xTimer, 0) != pdPASS)
        {
            ESP_LOGW(TAG, "closeTimerSwitch: fail to xTimerStop turnOffTimer of TimerMonitor");
        }
        if (xTimerDelete(xTimer, 0) != pdPASS)
        {
            ESP_LOGW(TAG, "closeTimerSwitch: fail to xTimerDelete turnOffTimer of TimerMonitor");
        }
        sub->turnOffTimer = NULL;
    }
}

static enum PowerState currentState;
static void setAndSendDataReady(enum PowerState state)
{
    currentState = state;
    sendDataReady(TimerMockGpioNum);
}
static enum PowerState readAndTransform(Monitor *monitor)
{
    return currentState;
}

static void turnOffTimerCallback(TimerHandle_t xTimer)
{
    if (xTimer != NULL)
    {
        Monitor *monitor = (Monitor *)pvTimerGetTimerID(xTimer);
        TimerSubObject *sub = (TimerSubObject *)(monitor->subObject);

        if (sub->turnOnTimer != NULL)
        {
            if (xTimerReset(sub->turnOnTimer, xBlockTime) == pdPASS)
            {
                ESP_LOGI(TAG, "turnOffTimer timeout, turn off, and reset turnOnTimer: after %" PRIu32 "ms, will be on", sub->offMs);
                setAndSendDataReady(PowerState_Off);
                // sendPowerStateMsg(powerSwitch, Off);
            }
            else
            {
                ESP_LOGW(TAG, "turnOffTimerCallback: fail to xTimerReset turnOnTimer of  TimerMonitor");
            }
        }
        else
        {
            ESP_LOGW(TAG, "turnOffTimerCallback: no turnOnTimer of  TimerMonitor");
        }
    }
    else
    {
        ESP_LOGW(TAG, "turnOffTimerCallback: no turnOffTimer");
    }
}

static void turnOnTimerCallback(TimerHandle_t xTimer)
{
    if (xTimer != NULL)
    {
        Monitor *monitor = (Monitor *)pvTimerGetTimerID(xTimer);
        TimerSubObject *sub = (TimerSubObject *)(monitor->subObject);

        if (sub->turnOffTimer != NULL)
        {
            if (xTimerReset(sub->turnOffTimer, xBlockTime) == pdPASS)
            {
                ESP_LOGI(TAG, "turnOnTimer timeout, turn on, and reset turnOffTimer: after %" PRIu32 "ms, will be off", sub->onMs);
                setAndSendDataReady(PowerState_On);
                // sendPowerStateMsg(powerSwitch, On);
            }
            else
            {
                ESP_LOGW(TAG, "turnOnTimerCallback: fail to xTimerReset turnOffTimerof  TimerMonitor");
            }
        }
        else
        {
            ESP_LOGW(TAG, "turnOnTimerCallback: no turnOffTimer of TimerMonitor");
        }
    }
    else
    {
        ESP_LOGW(TAG, "turnOnTimerCallback: no turnOnTimer");
    }
}

static void createShiftTimers(TimerHandle_t xTimer)
{
    if (xTimer == NULL)
    {
        ESP_LOGW(TAG, "createShisubimers: should not come here, no delayTimer, ignore");
        return;
    }

    Monitor *monitor = (Monitor *)pvTimerGetTimerID(xTimer);
    TimerSubObject *sub = (TimerSubObject *)(monitor->subObject);

    sub->turnOffTimer = xTimerCreate("turnOffTimer",
                                     pdMS_TO_TICKS(sub->onMs), // 只有在 configTICK_RATE_HZ 小于等于 1000 时，pdMS_TO_TICKS() 才可用
                                     pdFALSE,                  // The timers will auto-reload themselves when they expire.
                                     (void *)monitor,
                                     turnOffTimerCallback // Each timer calls the same callback when it expires
    );
    if (sub->turnOffTimer != NULL)
    {
        if (xTimerStart(sub->turnOffTimer, xBlockTime) != pdPASS)
        {
            ESP_LOGW(TAG, "createShiftTimers: fail to start turnOffTimer of TimerMonitor");
        }
        else
        {
            ESP_LOGI(TAG, "turn on, and turnOffTimer started: after %" PRIu32 "ms, will be off", sub->onMs);
            setAndSendDataReady(PowerState_On); // sendPowerStateMsg(powerSwitch, On); // turn on immediatly
        }
    }
    else
    {
        ESP_LOGW(TAG, "createShiftTimers: fail to create turnOffTimer of TimerMonitor");
    }

    sub->turnOnTimer = xTimerCreate("turnOnTimer",
                                    pdMS_TO_TICKS(sub->offMs), // 只有在 configTICK_RATE_HZ 小于等于 1000 时，pdMS_TO_TICKS() 才可用
                                    pdFALSE,                   // The timers will auto-reload themselves when they expire.
                                    (void *)monitor,
                                    turnOnTimerCallback // Each timer calls the same callback when it expires
    );
    if (sub->turnOnTimer == NULL)
    {
        ESP_LOGW(TAG, "createShiftTimers: fail to xTimerCreate turnOnTimer of TimerMonitor");
        return;
    }

    if (xTimerStop(xTimer, xBlockTime) != pdPASS)
    {
        ESP_LOGW(TAG, "createShiftTimers: fail to xTimerStop delayTimer of TimerMonitor");
    }
    if (xTimerDelete(xTimer, xBlockTime) != pdPASS)
    {
        ESP_LOGW(TAG, "createShiftTimers: fail to xTimerDelete delayTimer of TimerMonitor");
    }
    sub->delayTimer = NULL;
}

/**
 * Take effect after reboot
 */
static int changeStartDelay(TimerSubObject *self, uint32_t newValue)
{
    if (newValue == self->startDelayMs)
    {
        ESP_LOGI(TAG, "changeStartDelay: startAfterDelayMs is same, ignore");
        return 0;
    }

    self->startDelayMs = newValue;

    serialize(self);

    ESP_LOGI(TAG, "changeStartDelay done! Take effect after reboot");
    // if (self->delayTimer == NULL)
    // {
    //     ESP_LOGI(TAG, "changeStartDelay: delayTimer is out of date, ignore");
    //     return 0;
    // }

    // //xTimerChangePeriod() can be called to change the period of an active or dormant state timer.
    // //Changing the period of a dormant timers will also start the timer.
    // if (xTimerChangePeriod(self->delayTimer, pdMS_TO_TICKS(newValue), xBlockTime) == pdPASS)
    // {
    //     return 0;
    // }
    // else
    // {
    //     /* The command could not be sent, even after waiting for 1000 ticks0 to pass. Take appropriate action here. */
    //     ESP_LOGW(TAG, "changeStartDelay: xTimerChangePeriod failed");
    //     return -1;
    // }
    return 0;
}

/**
 * Take effect after reboot
 */
static int changeOnMs(TimerSubObject *self, uint32_t newValue)
{
    if (newValue == self->onMs)
    {
        ESP_LOGI(TAG, "changeOnMs: turnOnTimer is same, ignore");
        return 0;
    }

    self->onMs = newValue;

    serialize(self);

    ESP_LOGI(TAG, "changeOnMs done! Take effect after reboot");
    // if (self->turnOnTimer == NULL)
    // {
    //     ESP_LOGI(TAG, "changeOnMs: no turnOnTimer, ignore");
    //     return 0;
    // }
    // //xTimerChangePeriod() can be called to change the period of an active or dormant state timer.
    // //Changing the period of a dormant timers will also start the timer.
    // if (xTimerChangePeriod(self->turnOnTimer, pdMS_TO_TICKS(newValue), xBlockTime) == pdPASS)
    // {
    //     return 0;
    // }
    // else
    // {
    //     /* The command could not be sent, even after waiting for 1000 ticks0 to pass. Take appropriate action here. */
    //     ESP_LOGW(TAG, "changeOnMs: xTimerChangePeriod failed");
    //     return -1;
    // }
    return 0;
}

/**
 * Take effect after reboot
 */
static int changeOffMs(TimerSubObject *self, uint32_t newValue)
{
    if (newValue == self->offMs)
    {
        ESP_LOGI(TAG, "changeOffMs: turnOffTimer is same, ignore");
        return 0;
    }

    self->offMs = newValue;

    serialize(self);

    ESP_LOGI(TAG, "changeOffMs done! Take effect after reboot");

    // if (self->turnOffTimer == NULL)
    // {
    //     ESP_LOGI(TAG, "changeOffMs: no turnOffTimer, ignore");
    //     return 0;
    // }
    // //xTimerChangePeriod() can be called to change the period of an active or dormant state timer.
    // //Changing the period of a dormant timers will also start the timer.
    // if (xTimerChangePeriod(self->turnOffTimer, pdMS_TO_TICKS(newValue), xBlockTime) == pdPASS)
    // {
    //     return 0;
    // }
    // else
    // {
    //     /* The command could not be sent, even after waiting for 1000 ticks0 to pass. Take appropriate action here. */
    //     ESP_LOGW(TAG, "changeOffMs: xTimerChangePeriod failed");
    //     return -1;
    // }
    return 0;
}


TimerSubObject* createTimerSubObject(uint32_t startDelayMs, uint32_t onMs,uint32_t offMs)
{
    TimerSubObject* me = (TimerSubObject*)malloc(sizeof(TimerSubObject));
    me->startDelayMs = startDelayMs;
    me->onMs = onMs;
    me->offMs = offMs;
    me->delayTimer= NULL;
    me->turnOffTimer = NULL;
    me->turnOnTimer = NULL;
    me->changeStartDelay = changeStartDelay;
    me->changeOffMs = changeOffMs;
    me->changeOnMs = changeOnMs;
    me->serialize = serialize;
    me->deserialize = deserialize;

    return me;
}

IDevice monitor_device_timer = {init, start, readAndTransform, uninit};