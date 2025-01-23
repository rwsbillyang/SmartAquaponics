#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <inttypes.h>

#include "esp_log.h"
#include "driver/gpio.h"

#include "common.h"
#include "power_switch.h"
#include "monitor.h"
#include "monitor_complex.h"
#include "serialize.h"

//********************************************************//
//******************** PowerSwitch implementation ******************//
//********************************************************//

static void serialize(PowerSwitch *ps)
{
}

static PowerSwitch *deserialize()
{
    return NULL;
}

static void start(PowerSwitch *ps)
{
    if (ps->monitorType == Type_Complex)
    {
        ComplexMonitor *cm = (ComplexMonitor *)ps->monitor;
        Monitor *m = NULL;
        for (int i = 0; i < cm->count; i++)
        {
            m = cm->monitors[i];
            m->device->start(m);
        }
    }
    else
    {
        Monitor *m = (Monitor *)ps->monitor;
        m->device->start(m);
    }
}

static void release(PowerSwitch *ps)
{
    gpio_reset_pin(ps->outputGpio);
    if (ps->monitorType == Type_Complex)
    {
        ComplexMonitor *cm = (ComplexMonitor *)ps->monitor;
        cm->release(cm);
    }
    else
    {
        Monitor *m = (Monitor *)ps->monitor;
        m->release(m);
    }
    free(ps->name);
    free(ps);
}

/**
 *
 */
static void changeMonitor(PowerSwitch *ps, enum MonitorType monitorType, void *newMonitor)
{
    if (ps->monitor == newMonitor)
    {
        ESP_LOGI(TAG, "Monitor is same, ignore");
        return;
    }
    if (ps->monitorType == Type_Complex)
    {
        ComplexMonitor *cm = (ComplexMonitor *)ps->monitor;
        cm->release(cm);
    }
    else
    {
        Monitor *m = (Monitor *)ps->monitor;
        m->release(m);
    }
    ps->monitorType = monitorType;
    ps->monitor = newMonitor;

    serialize(ps); // serialization
}

static void hwConfigGpioOutput(int32_t gpioNum)
{
    // zero-initialize the config structure.
    gpio_config_t io_conf = {};
    // disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    // set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    // bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = (1ULL << gpioNum);
    // disable pull-down mode
    io_conf.pull_down_en = 0;
    // disable pull-up mode
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);
}

// output: hardware switch
static void hwSwitch(PowerSwitch *ps, enum PowerState state)
{
    if (state == PowerState_On)
    {
        if (ps->vLevelOfOn == HighLevel) // if set pin high voltate, it means turn on
        {
            gpio_set_level(ps->outputGpio, 1);
        }
        else
        {
            gpio_set_level(ps->outputGpio, 0);
        }
    }
    else if (state == PowerState_Off)
    {
        if (ps->vLevelOfOn == LowLevel) // if set pin high voltate, it means turn on
        {
            gpio_set_level(ps->outputGpio, 1);
        }
        else
        {
            gpio_set_level(ps->outputGpio, 0);
        }
    }
}

/**
 * @brief
 * @param name
 * @param outputPin
 * @param  vLevelOfOn
 * @param  workMode
 * @param trigger
 * @return
 */
PowerSwitch *createPowerSwitch(
    char *name,
    int32_t outputGpio,
    enum VoltLevel vLevelOfOn,
    void *monitor,
    enum MonitorType monitorType)
{
    char *p = (char *)malloc(strlen(name) * sizeof(char));
    strcpy(p, name);

    PowerSwitch *ps = malloc(sizeof(PowerSwitch));
    ps->name = p;
    ps->outputGpio = outputGpio;
    ps->vLevelOfOn = vLevelOfOn;
    ps->monitorType = monitorType;
    ps->monitor = monitor;
    ps->serialize = serialize;
    ps->deserialize = deserialize;
    ps->release = release;
    ps->start = start;
    ps->changeMonitor = changeMonitor;
    ps->hwSwitch = hwSwitch;

    hwConfigGpioOutput(outputGpio);

    return ps;
}

// static uint16_t inputGpioCount = 0;
// static void start(PowerSwitch *powerSwitch)
// {
//     hwConfigGpioOutput(powerSwitch->outputGpio);
//     // uint16_t oldCount = inputGpioCount;
//     switch (powerSwitch->workMode)
//     {
//     case ByNone:
//     {
//         break;
//     }
//     case ByTimer:
//     {
//         startTimerSwitch(powerSwitch);
//         break;
//     }
//     case ByInputSignal:
//     {
//         inputGpioCount += startInputTrigger(powerSwitch);
//         break;
//     }
//     case ByMultiInputSignal:
//     {
//         inputGpioCount += startMultiInputTrigger(powerSwitch);
//         break;
//     }
//     case ByCmd:
//     {
//         break;
//     }
//     default:
//     {
//         printf("Not support workmode=%d", powerSwitch->workMode);
//         break;
//     }
//     }

//     // if (oldCount == 0 && inputGpioCount > 0)
//     // {
//     //     startInputSignalGpioTaskLoop();
//     // }
// }

// static void releasePowerSwitch(PowerSwitch *p)
// {
//     hwConfigGpioOutput(p->outputPin, 0);

//     switch (p->workMode)
//     {
//     case ByNone:
//     {
//         break;
//     }
//     case ByTimer:
//     {
//         releaseTimerTrigger(p);
//         break;
//     }
//     case ByInputSignal:
//     {
//         releaseInputTrigger((InputTrigger *)p->trigger);
//         inputGpioCount--;
//         break;
//     }
//     case ByMultiInputSignal:
//     {
//         MultiInputTrigger *mt = (MultiInputTrigger *)p->trigger;
//         inputGpioCount -= mt->triggersCount;
//         releaseMultiInputTrigger(mt);

//         break;
//     }
//     case ByCmd:
//     {
//         break;
//     }
//     default:
//     {
//         printf("Not support workmode=%d", p->workMode);
//         break;
//     }
//     }
//     free(p->name);
//     free(p);
// }