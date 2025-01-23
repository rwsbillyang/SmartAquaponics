#pragma once

#define ISerialize(T)          \
    void (*serialize)(T * me); \
    T *(*deserialize)()

#define Release(T) void (*release)(T * me)
#define xBlockTime 1000

#define TAG  "SmartPower"

#ifdef __cplusplus
extern "C"
{
#endif


    enum VoltLevel
    {
        LowLevel,
        HighLevel
    };

    // state enum of power
    enum PowerState
    {
        PowerState_Off, // power off
        PowerState_On,  // power on
        PowerState_None // keep old state
    };

    /// @brief compare the sample signal to the threashold
    enum Op
    {
        eq,
        lt,
        lte,
        gt,
        gte
    };

    enum MonitorType
    {
        Type_None,  // not turn on/off, not controlled by anything
        Type_Timer, // controlled by timer
        Type_Complex,
        Type_VoltLevel, // controlled by one outsource, eg. sensor
        Type_Other,
        Type_Cmd // controlled by commands that maybe from network such as http request
    };
    enum SubObjectType
    {
        SubType_Timer,
        SubType_DataToPowerStateMap
    };

#ifdef __cplusplus
}
#endif