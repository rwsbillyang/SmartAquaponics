#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <inttypes.h>

#include "LinkedList.h"
#include "common.h"

#include "monitor.h"
#include "monitor_data_to_powerstate.h"
#include "monitor_complex.h"
#include "monitor_device_timer.h"
#include "power_switch.h"
#include "power_manager.h"

// #define  topWaterMonitorGpio  11
// #define  bottomWaterMonitorGpio  12
// #define  mainPumpRelayGpio  13
// #define  wavePumpRelayGpio  14
// #define temperatureGpio  15
// #define heaterRelayGpio  16
// #define feederGpio  17
// #define DHT11_PIN  18

uint16_t topWaterMonitorGpio = 11;
uint16_t bottomWaterMonitorGpio = 12;
uint16_t mainPumpRelayGpio = 13;

uint16_t wavePumpRelayGpio = 14;

uint16_t temperatureGpio = 15; // DS18B20
uint16_t heaterRelayGpio = 16;

uint16_t feederRelayGpio = 17;

uint16_t dht11DataPin = 18; // temperature and humidity

// water level monitor
// M03, M04 water level monitor ouput:
// no water: high level
// has water: low level

static PowerSwitch *installMainPump(uint8_t useWaterMonitor)
{
    PowerSwitch *pump = NULL;
    if (useWaterMonitor == 2)
    {
        DataToPowerState *hasWaterTurnOff = createDataToPowerState(eq, LowLevel, PowerState_Off); // if signal eq Low (has water), turn off pump
        DataToPowerStateMap *map = createDataToPowerStateMap(1, hasWaterTurnOff);
        Monitor *topWaterLevelMonitor = createMonitor("topWaterMonitor", Type_VoltLevel, topWaterMonitorGpio, &monitor_device_volt_level, map, SubType_DataToPowerStateMap);

        DataToPowerState *noWaterTurnOn = createDataToPowerState(eq, HighLevel, PowerState_On); // if signal eq High (no water), turn on pump
        DataToPowerStateMap *map2 = createDataToPowerStateMap(1, noWaterTurnOn);
        Monitor *bottomWaterLevelMonitor = createMonitor("bottomWaterMonitor", Type_VoltLevel, bottomWaterMonitorGpio, &monitor_device_volt_level, map2, SubType_DataToPowerStateMap);

        ComplexMonitor *monitor = createComplexMonitor("waterMonitors", 2, topWaterLevelMonitor, bottomWaterLevelMonitor);

        pump = createPowerSwitch("mainPump", mainPumpRelayGpio, HighLevel, (void *)monitor, monitor->monitorType);
    }
    else
    {
        TimerSubObject *sub = createTimerSubObject(5 * 1000, 30 * 60 * 1000, 30 * 60 * 1000);
        Monitor *monitor = createMonitor("timerMonitor", Type_Timer, TimerMockGpioNum, &monitor_device_timer, sub, SubType_Timer);
        pump = createPowerSwitch("mainPump", mainPumpRelayGpio, HighLevel, (void *)monitor, monitor->monitorType);
    }

    regiesterPowerSwitch(pump);
    return pump;
}

static PowerSwitch *installWavePump()
{

    TimerSubObject *sub = createTimerSubObject(5 * 1000, 1 * 60 * 1000, 5 * 60 * 1000);
    Monitor *monitor = createMonitor("waveTimer", Type_Timer, TimerMockGpioNum, &monitor_device_timer, sub, SubType_Timer);
    PowerSwitch *pump = createPowerSwitch("wavePump", wavePumpRelayGpio, HighLevel, (void *)monitor, monitor->monitorType);

    regiesterPowerSwitch(pump);
    return pump;
}

static PowerSwitch *intallHeater()
{
    DataToPowerState *turnOff = createDataToPowerState(gt, 25, PowerState_Off); // if signal gte 25 degree, turn off heater
    DataToPowerState *turnOn = createDataToPowerState(lt, 20, PowerState_On);    // if signal lt 20 degree, turn on heater
    DataToPowerStateMap *map = createDataToPowerStateMap(2, turnOff, turnOn);
    Monitor *monitor = createMonitor("heaterMonitor", Type_Other, temperatureGpio, &monitor_device_ds18b20, map, SubType_DataToPowerStateMap);

    PowerSwitch *heater = createPowerSwitch("heater", heaterRelayGpio, HighLevel, (void *)monitor, monitor->monitorType);
    regiesterPowerSwitch(heater);
    return heater;
}

static PowerSwitch *installAutoFeeder()
{
    TimerSubObject *sub = createTimerSubObject(10 * 1000, 1 * 60 * 1000, 5 * 60 * 1000);
    Monitor *monitor = createMonitor("feederTimer", Type_Timer, TimerMockGpioNum, &monitor_device_timer, sub, SubType_Timer);
    PowerSwitch *feeder = createPowerSwitch("feeder", feederRelayGpio, HighLevel, (void *)monitor, monitor->monitorType);

    regiesterPowerSwitch(feeder);
    return feeder;
}

static PowerSwitch *installNightLights()
{
    return NULL;
}

void initAquaponics()
{
    installMainPump(2);
    installWavePump();
    intallHeater();
    installAutoFeeder();
    installNightLights();
}