#pragma once

#include <stdint.h>
#include <stdlib.h>

#include "common.h"
#include "power_switch.h"
#include "monitor.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct
    {
        PowerSwitch *ps;
        Monitor *monitor;
    } SignalDataSource;

    // start the powerSwitch, and add it into the head in system
    void regiesterPowerSwitch(PowerSwitch *ps);

    // release the powerSwitch, and remove it from the system
    void unregisterPowerSwitch(PowerSwitch *ps);
    
    /**
     * find PowerSwitch source according to gpio
     * @param inputGpio the gpio of monitor
     * @return return NULL if not found, caller shoule free the returned result
     * */
    SignalDataSource *findPowerSwitchByGpio(int32_t inputGpio);
#ifdef __cplusplus
}
#endif