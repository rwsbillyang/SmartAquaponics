#pragma once

#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

#include "common.h"

#ifdef __cplusplus
"C"
{
#endif
    extern const int32_t TimerMockGpioNum; // no gpio input, mock it

    typedef struct _TimerSubObject TimerSubObject;
    struct _TimerSubObject
    {
        uint32_t startDelayMs; // start after delay ms
        uint32_t onMs;
        uint32_t offMs;

        TimerHandle_t delayTimer;
        TimerHandle_t turnOffTimer;
        TimerHandle_t turnOnTimer;

        int (*changeStartDelay)(TimerSubObject *self, uint32_t newValue);
        int (*changeOnMs)(TimerSubObject *self, uint32_t newValue);
        int (*changeOffMs)(TimerSubObject *self, uint32_t newValue);

        ISerialize(TimerSubObject);
    } ;

    TimerSubObject *createTimerSubObject(uint32_t startDelayMs, uint32_t onMs, uint32_t offMs);
#ifdef __cplusplus
}
#endif