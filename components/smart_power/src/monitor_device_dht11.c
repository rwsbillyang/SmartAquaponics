#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_log.h"
#include "driver/gpio.h"

#include "common.h"
#include "monitor.h"
#include "dht11.h"
#include "queue_task_loop.h"
#include "monitor_data_to_powerstate.h"

static const int ReadInterval = 2000; // unit: ms
static TimerHandle_t xTimer = NULL;

static void timerCallback(TimerHandle_t xTimer)
{
    if (xTimer != NULL)
    {
        Monitor *monitor = (Monitor *)pvTimerGetTimerID(xTimer);
        sendDataReady(monitor->gpioNum); // xQueueSend(xDataReadyQueue, &monitor->gpioNum, NULL);
    }
    else
    {
        ESP_LOGW(TAG, "no xTimer in timerCallback?");
    }
}

static void init(Monitor *monitor)
{
    dht11Init(monitor->gpioNum);

    xTimer = xTimerCreate("dht11ReadTimer",
                          pdMS_TO_TICKS(ReadInterval), // 只有在 configTICK_RATE_HZ 小于等于 1000 时，pdMS_TO_TICKS() 才可用
                          pdTRUE,                      // The timers will auto-reload themselves when they expire.
                          (void *)monitor,
                          timerCallback // Each timer calls the same callback when it expires
    );
    if (xTimer == NULL)
    {
        ESP_LOGW(TAG, "fail to create dht11ReadTimer");
    }
}
static void start(Monitor *monitor)
{
    if (xTimerStart(xTimer, 0) != pdPASS)
    {
        /* The timer could not be set into the Active state. */
        ESP_LOGW(TAG, "init: fail to start a delay timer of ds18b20ReadTimer");
    }
}
static void uninit(Monitor *monitor)
{
    int32_t gpioNum = monitor->gpioNum;
    gpio_reset_pin(gpioNum);
    if (xTimer != NULL)
    {
        if (xTimerStop(xTimer, 0) != pdPASS)
        {
            ESP_LOGW(TAG, "cleanTimerTrigger: fail to xTimerStop timer of monitor_dht11");
        }
        if (xTimerDelete(xTimer, 0) != pdPASS)
        {
            ESP_LOGW(TAG, "cleanTimerTrigger: fail to xTimerDelete timer of monitor_dht11");
        }
        xTimer = NULL;
    }
}

static enum PowerState readAndTransform(Monitor *monitor)
{
    int temprature;
    int humidity;
    dht11Read(monitor->gpioNum, &temprature, &humidity);

    DataToPowerState **array = ((DataToPowerStateMap *)(monitor->subObject))->array;
    uint16_t count = ((DataToPowerStateMap *)(monitor->subObject))->count;

    return data2PowerState(humidity, array, count);
}

IDevice monitor_device_dht11 = {init, start, readAndTransform, uninit};
