#pragma once

#include <stdint.h>
#include <stdlib.h>

#include "common.h"
#include "monitor.h"

#ifdef __cplusplus
extern "C"
{
#endif
    typedef struct _ComplexMonitor ComplexMonitor;
    
    struct _ComplexMonitor
    {
        char *name; // for debug convenience
        enum MonitorType monitorType;
        Monitor **monitors;
        uint16_t count;

        ISerialize(struct _ComplexMonitor);
        Release(struct _ComplexMonitor);
    };



    /**
     * @param name
     * @param count the count of Monitor
     * @param ...  the list of Monitor
     * */
    ComplexMonitor *createComplexMonitor(char *name, int count, ...);

#ifdef __cplusplus
}
#endif