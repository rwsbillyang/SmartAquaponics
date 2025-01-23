#include <stdint.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

#include "monitor.h"
#include "monitor_data_to_powerstate.h"
#include "queue_task_loop.h"

/**
 * voltage level(High/Low) monitor: if voltage of the speicified gpio changes, interupt occurs,
 * then read its volt level
 *
 */

// all gpio share interutpt service routine, so need get power switch according to gpio number
static void IRAM_ATTR gpioIsrHandler(void *arg)
{
    int32_t gpio_num = (int32_t)arg;
    sendDataReadyISR(gpio_num); // xQueueSendFromISR(xDataReadyQueue, &gpio_num, NULL);
}

static void init(Monitor *monitor)
{
    int32_t gpioNum = monitor->gpioNum;

    // zero-initialize the config structure.
    gpio_config_t io_conf = {};

    // interrupt of rising edge
    io_conf.intr_type = GPIO_INTR_LOW_LEVEL | GPIO_INTR_HIGH_LEVEL;
    // bit mask of the pins, use GPIO4/5 here
    io_conf.pin_bit_mask = (1ULL << gpioNum);
    // set as input mode
    io_conf.mode = GPIO_MODE_INPUT;
    // disable pull-down mode
    io_conf.pull_down_en = 0;
    // enable pull-up mode
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);

}
static void start(Monitor *monitor){
    gpio_install_isr_service(ESP_INTR_FLAG_LEVEL3);
    // hook isr handler for specific gpio pin
    gpio_isr_handler_add(monitor->gpioNum, gpioIsrHandler, (void *)monitor->gpioNum);
}
static void uninit(Monitor *monitor)
{
    int32_t gpioNum = monitor->gpioNum;
    gpio_reset_pin(gpioNum);
    gpio_isr_handler_remove(gpioNum); // remove isr handler for gpio number.
}

static enum PowerState readAndTransform(Monitor *monitor)
{
    int32_t gpioNum = monitor->gpioNum;
    int signal = gpio_get_level(gpioNum);

    DataToPowerState **array = ((DataToPowerStateMap *)(monitor->subObject))->array;
    uint16_t count = ((DataToPowerStateMap *)(monitor->subObject))->count;
    return data2PowerState(signal, array, count);
}

IDevice monitor_device_volt_level = {init, start, readAndTransform, uninit};