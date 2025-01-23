#pragma once

#include <stdint.h>
#include <stdlib.h>

#include "common.h"

#ifdef __cplusplus
extern "C"
{
#endif

    // the controll signal means which power state
    typedef struct _DataToPowerState DataToPowerState;
    struct _DataToPowerState
    {
        enum Op op;    // eg: if signal eq == threshhold, then set state powerState
        int threshold; // signal Op(eq, lt... etc.) threshold
        enum PowerState powerState;
        ISerialize(DataToPowerState);
        Release(DataToPowerState);
    };

    typedef struct _DataToPowerStateMap DataToPowerStateMap;
    struct _DataToPowerStateMap
    {
        DataToPowerState **array; // element of array is pointer which point to SignalToPowerState
        uint16_t count;
        ISerialize(DataToPowerStateMap);
        Release(DataToPowerStateMap);
    };

    /**
     * create DataToPowerState
     */
    DataToPowerState *createDataToPowerState(enum Op op, int threshold, enum PowerState powerState);

    /**
     * create DataToPowerStateMap using DataToPowerState list
     */
    DataToPowerStateMap *createDataToPowerStateMap(int num, ...);

    /**
     * convert the signal from device to power state according to DataToPowerState map
     *
     */
    enum PowerState data2PowerState(int signal, DataToPowerState **array, uint16_t count);

#ifdef __cplusplus
}
#endif