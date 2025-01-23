#pragma once

#include <stdint.h>
#include <stdlib.h>

#include "common.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct _Monitor Monitor;

    // different implementiaons have different values
    typedef struct
    {
        void (*init)(Monitor *monitor);
        void (*start)(Monitor *monitor);
        enum PowerState (*readAndTransform)(Monitor *monitor);
        void (*uninit)(Monitor *monitor);
    } IDevice;

    extern IDevice monitor_device_volt_level;
    extern IDevice monitor_device_timer;
    extern IDevice monitor_device_ds18b20;
    extern IDevice monitor_device_dht11;

    struct _Monitor
    {
        char *name; // for debug convenience
        enum MonitorType monitorType;
        int32_t gpioNum; // input control signal from which gpio pin No.

        enum SubObjectType subType;
        void *subObject; // point to DataToPowerStateMap or TimerSubObject, or null

        IDevice *device; // different implementiaons have different values
        ISerialize(Monitor);
        Release(Monitor);
    };

    /**
     * create a Monitor and init it by calling its init method
     */
    Monitor *createMonitor(char *name, enum MonitorType monitorType, int32_t gpioNum, IDevice *device, void *subObject, enum SubObjectType subType);
#ifdef __cplusplus
}
#endif