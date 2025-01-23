
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_check.h"
#include "driver/gpio.h"

#include "onewire_bus.h"
#include "ds18b20.h"

#include "common.h"
#include "monitor.h"
#include "monitor_data_to_powerstate.h"
#include "queue_task_loop.h"


/**
 * Refer to:
 * 
// 不带屏蔽输出引线：红色(VCC)，黄色（白色）(DATA)，黑色(GND)
// 带屏蔽的输出引线：红色(VCC电源线)，黄色(DATA信号线)，白色(GND地线)
// 带屏蔽线的，红色VCC接ESP32的VIN口(5V供电)，白色GND接ESP32的GND，黄色信号线接ESP32的D4。
// https://www.codeleading.com/article/56276530597/

https://components.espressif.com/components/espressif/onewire_bus/versions/1.0.2
> idf.py add-dependency "espressif/onewire_bus^1.0.2"

https://components.espressif.com/components/espressif/onewire_bus/versions/1.0.2
>idf.py add-dependency "espressif/ds18b20^0.1.1"
 *
 */
#define ONEWIRE_MAX_DS18B20 2
static const int ReadInterval = 2000; // unit: ms

static int ds18b20_device_num = 0;
static ds18b20_device_handle_t ds18b20s[ONEWIRE_MAX_DS18B20];

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
static TimerHandle_t timer = NULL;
static void init(Monitor *monitor)
{
    int32_t gpioNum = monitor->gpioNum;
    // install new 1-wire bus
    onewire_bus_handle_t bus;
    onewire_bus_config_t bus_config = {
        .bus_gpio_num = gpioNum,
    };
    onewire_bus_rmt_config_t rmt_config = {
        .max_rx_bytes = 10, // 1byte ROM command + 8byte ROM number + 1byte device command
    };
    ESP_ERROR_CHECK(onewire_new_bus_rmt(&bus_config, &rmt_config, &bus));
    ESP_LOGI(TAG, "1-Wire bus installed on GPIO %" PRId32, gpioNum);

    onewire_device_iter_handle_t iter = NULL;
    onewire_device_t next_onewire_device;
    esp_err_t search_result = ESP_OK;

    // create 1-wire device iterator, which is used for device search
    ESP_ERROR_CHECK(onewire_new_device_iter(bus, &iter));
    ESP_LOGI(TAG, "Device iterator created, start searching...");
    ds18b20_device_num = 0;
    do
    {
        search_result = onewire_device_iter_get_next(iter, &next_onewire_device);
        if (search_result == ESP_OK)
        { // found a new device, let's check if we can upgrade it to a DS18B20
            ds18b20_config_t ds_cfg = {};
            if (ds18b20_new_device(&next_onewire_device, &ds_cfg, &ds18b20s[ds18b20_device_num]) == ESP_OK)
            {
                ESP_LOGI(TAG, "Found a DS18B20[%d], address: %016llX", ds18b20_device_num, next_onewire_device.address);
                ds18b20_device_num++;
                if (ds18b20_device_num >= ONEWIRE_MAX_DS18B20)
                {
                    ESP_LOGI(TAG, "Max DS18B20 number reached, stop searching...");
                    break;
                }
            }
            else
            {
                ESP_LOGI(TAG, "Found an unknown device, address: %016llX", next_onewire_device.address);
            }
        }
    } while (search_result != ESP_ERR_NOT_FOUND);
    ESP_ERROR_CHECK(onewire_del_device_iter(iter));
    ESP_LOGI(TAG, "Searching done, %d DS18B20 device(s) found", ds18b20_device_num);

    // set resolution for all DS18B20s
    for (int i = 0; i < ds18b20_device_num; i++)
    {
        // set resolution
        ESP_ERROR_CHECK(ds18b20_set_resolution(ds18b20s[i], DS18B20_RESOLUTION_12B));
    }

    timer = xTimerCreate("ds18b20ReadTimer",
                                       pdMS_TO_TICKS(ReadInterval), // 只有在 configTICK_RATE_HZ 小于等于 1000 时，pdMS_TO_TICKS() 才可用
                                       pdTRUE,                      // The timers will auto-reload themselves when they expire.
                                       (void *)monitor,
                                       timerCallback // Each timer calls the same callback when it expires
    );
    if (timer == NULL)
    {
        ESP_LOGW(TAG, "fail to create ds18b20ReadTimer");
    }
}
static void start(Monitor *monitor){
    if (xTimerStart(timer, 0) != pdPASS)
    {
        /* The timer could not be set into the Active state. */
        ESP_LOGW(TAG, "init: fail to start a delay timer of ds18b20ReadTimer");
    }
}
static void uninit(Monitor *monitor)
{
    int32_t gpioNum = monitor->gpioNum;
    gpio_reset_pin(gpioNum);
}

static enum PowerState readAndTransform(Monitor *monitor)
{
    // get temperature from sensors one by one
    float temperature[ds18b20_device_num];
    float sum = 0.0;
    for (int i = 0; i < ds18b20_device_num; i++)
    {
        ESP_ERROR_CHECK(ds18b20_trigger_temperature_conversion(ds18b20s[i]));
        ESP_ERROR_CHECK(ds18b20_get_temperature(ds18b20s[i], &(temperature[i])));
        ESP_LOGI(TAG, "temperature read from DS18B20[%d]: %.2fC", i, temperature[i]);
        sum += temperature[i];
    }

    DataToPowerState **array = ((DataToPowerStateMap *)(monitor->subObject))->array;
    uint16_t count = ((DataToPowerStateMap *)(monitor->subObject))->count;

    float avg = ds18b20_device_num > 0 ? sum / ds18b20_device_num : sum;
    int signal = (int)avg;
    return data2PowerState(signal, array, count);
}


IDevice monitor_device_ds18b20 = {init, start, readAndTransform, uninit};
