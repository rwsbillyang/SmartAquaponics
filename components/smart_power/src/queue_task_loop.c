#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <inttypes.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "esp_check.h"

#include "common.h"
#include "monitor.h"
#include "power_switch.h"
#include "power_manager.h"
#include "queue_task_loop.h"

static QueueHandle_t xDataReadyQueue = NULL;
static TaskHandle_t xReadDataHandle = NULL;

static QueueHandle_t xPowerSwitchQueue = NULL;
static TaskHandle_t xSwitchHandle = NULL;

inline void sendDataReady(int32_t monitorGpio)
{
    // BaseType_t xQueueSend(QueueHandle_t xQueue,const void * pvItemToQueue,TickType_t xTicksToWait);
    xQueueSend(xDataReadyQueue, &monitorGpio, xBlockTime);
}
inline void sendDataReadyISR(int32_t monitorGpio)
{
    // BaseType_t xQueueSendFromISR(QueueHandle_t xQueue,const void *pvItemToQueue, BaseType_t *pxHigherPriorityTaskWoken);
    BaseType_t high_task_wakeup = pdFALSE;
    xQueueSendFromISR(xDataReadyQueue, &monitorGpio, &high_task_wakeup);
}

inline void sendPowerStateMsg(PowerSwitch *powerSwitch, enum PowerState state)
{
    PowerSwitchMsg msg = {powerSwitch, state};
    xQueueSend(xPowerSwitchQueue, &msg, xBlockTime);
}

static void readDataLoop(void *pvParameters);
void startReadDataTask(uint8_t isCreateNewTask)
{
    if (xDataReadyQueue == NULL)
    {
        xDataReadyQueue = xQueueCreate(10, sizeof(int32_t));
    }

    if (xDataReadyQueue != NULL)
    {
        if (isCreateNewTask == 1 && xReadDataHandle == NULL)
        {
            // BaseType_t xTaskCreate( TaskFunction_t pvTaskCode,const char * const pcName,const configSTACK_DEPTH_TYPE uxStackDepth,void *pvParameters, UBaseType_t uxPriority,TaskHandle_t *pxCreatedTask);
            // 如果任务创建成功，则返回 pdPASS，否则返回 errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY
            if (xTaskCreate(readDataLoop, "readDataLoop", 2048, NULL, 8, &xReadDataHandle) != pdPASS)
            {
                // printf("fail to xTaskCreate readDataLoop");
                ESP_LOGE(TAG, "fail to xTaskCreate readDataLoop");
                return;
            }
        }
        else
        {
            readDataLoop(NULL);
        }
    }
    else
    {
        // printf("fail to xQueueCreate xDataReadyQueue, not create readDataLoop");
        ESP_LOGE(TAG, "fail to xQueueCreate xDataReadyQueue, not create readDataLoop");
        return;
    }
}

static void switchLoop(void *pvParameters);
void startSwitchTask()
{
    if (xPowerSwitchQueue == NULL)
    {
        xPowerSwitchQueue = xQueueCreate(10, sizeof(PowerSwitchMsg));
    }

    if (xPowerSwitchQueue != NULL)
    {
        if (xSwitchHandle == NULL)
        {
            // BaseType_t xTaskCreate( TaskFunction_t pvTaskCode,const char * const pcName,const configSTACK_DEPTH_TYPE uxStackDepth,void *pvParameters, UBaseType_t uxPriority,TaskHandle_t *pxCreatedTask);
            // 如果任务创建成功，则返回 pdPASS，否则返回 errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY
            if (xTaskCreate(switchLoop, "switchLoop", 2048, NULL, 4, &xSwitchHandle) != pdPASS)
            {
                ESP_LOGE(TAG, "fail to xTaskCreate switchLoop");
                return;
            }
        }
    }
    else
    {
        ESP_LOGE(TAG, "fail to xQueueCreate xDataReadyQueue, not create switchLoop");
        return;
    }
}

static void readDataLoop(void *pvParameters)
{
    int32_t gpio_num;
    for (;;)
    {
        if (xQueueReceive(xDataReadyQueue, &gpio_num, portMAX_DELAY))
        {
            SignalDataSource *r = findPowerSwitchByGpio(gpio_num);
            if (r != NULL)
            {
                enum PowerState state = r->monitor->device->readAndTransform(r->monitor);
                sendPowerStateMsg(r->ps, state);
                free(r);
            }
            else
            {
                ESP_LOGE(TAG, "readDataLoop: Not found PowerSwitch by input gpio=%" PRId32 ", please check config correct?", gpio_num);
            }
        }
        else
        {
            ESP_LOGE(TAG, "readDataLoop: fail to xQueueReceive event");
        }
    }
}

// handle turn on/off powerState switch events
static void switchLoop(void *pvParameters)
{
    PowerSwitchMsg msg = {};
    for (;;)
    {
        if (xQueueReceive(xPowerSwitchQueue, &msg, portMAX_DELAY))
        {
            msg.powerSwitch->hwSwitch(msg.powerSwitch, msg.state);
        }
        else
        {
            ESP_LOGW(TAG, "fail to xQueueReceive PowerSwitchMsg");
        }
    }
}

void releaseReadDataTask()
{
    if (xReadDataHandle != NULL)
    {
        vTaskDelete(xReadDataHandle);
        xReadDataHandle = NULL;
    }
}

void releaseSwitchTask()
{
    if (xSwitchHandle != NULL)
    {
        vTaskDelete(xSwitchHandle);
        xSwitchHandle = NULL;
    }
}