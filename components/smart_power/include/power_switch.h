#pragma once

#include <stdint.h>
#include <stdlib.h>

#include "common.h"
#include "monitor.h"

#ifdef __cplusplus
"C"
{
#endif

    typedef struct _PowerSwitch PowerSwitch;

    /// smart switch: turn of/off power
    struct _PowerSwitch
    {
        char *name;
        int32_t outputGpio;
        enum VoltLevel vLevelOfOn;
        enum MonitorType monitorType;
        void *monitor; // point to Monitor or ComplexMonitor, depends on monitorType

        ISerialize(PowerSwitch);
        Release(PowerSwitch);
        void (*start)(PowerSwitch *swtich);
        void (*changeMonitor)(PowerSwitch *ps, enum MonitorType monitorType, void *newMonitor);
        void (*hwSwitch)(PowerSwitch *swtich, enum PowerState state);
    };

    /**
     * @brief
     * @param name
     * @param outputPin
     * @param vLevelOfOn
     * @param monitor
     * @param monitorType
     * @return
     */
    PowerSwitch *createPowerSwitch(
        char *name,
        int32_t outputGpio,
        enum VoltLevel vLevelOfOn,
        void *monitor,
        enum MonitorType monitorType);

#ifdef __cplusplus
}
#endif