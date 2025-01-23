#pragma once

#include <inttypes.h>
#include "common.h"
#include "monitor.h"
#include "power_switch.h"
#include "power_manager.h"

#ifdef __cplusplus
extern "C"
{
#endif
    // turn on/off msg
    typedef struct
    {
        PowerSwitch *powerSwitch;
        enum PowerState state;
    } PowerSwitchMsg;

    void sendDataReady(int32_t monitorGpio);
    void sendDataReadyISR(int32_t monitorGpio);
    void sendPowerStateMsg(PowerSwitch *powerSwitch, enum PowerState state);

    void startReadDataTask(uint8_t isCreateNewTask);
    void startSwitchTask();

    void releaseReadDataTask();
    void releaseSwitchTask();

#ifdef __cplusplus
}
#endif