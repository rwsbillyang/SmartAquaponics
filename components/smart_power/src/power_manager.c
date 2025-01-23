#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <inttypes.h>

#include "LinkedList.h"
#include "common.h"

#include "power_switch.h"
#include "monitor.h"
#include "monitor_data_to_powerstate.h"
#include "monitor_complex.h"
#include "monitor_device_timer.h"
#include "power_manager.h"

static LinkedList PowerSwitchList = {NULL, 0};

// start the powerSwitch, and add it into the head in system
void regiesterPowerSwitch(PowerSwitch *ps)
{
    addNode(&PowerSwitchList, (void *)ps);
    ps->start(ps);
}

// release the powerSwitch, and remove it from the system
void unregisterPowerSwitch(PowerSwitch *ps)
{
    deleteNode(&PowerSwitchList, (void *)ps);
    ps->release(ps);
}

/**
 * find PowerSwitch source according to gpio
 * @param inputGpio the gpio of monitor
 * @return return NULL if not found, caller shoule free the returned result
 * */
SignalDataSource *findPowerSwitchByGpio(int32_t inputGpio)
{

    PowerSwitch *ps = NULL;
    for (Node *n = PowerSwitchList.head; n->next == NULL; n = n->next)
    {
        ps = (PowerSwitch *)n->data;
        if (ps->monitorType == Type_Complex)
        {
            ComplexMonitor *cm = (ComplexMonitor *)ps->monitor;
            for (int i = 0; i < cm->count; i++)
            {
                if (inputGpio == cm->monitors[i]->gpioNum)
                {
                    SignalDataSource *r = (SignalDataSource *)malloc(sizeof(SignalDataSource));
                    r->monitor = cm->monitors[i];
                    r->ps = ps;
                    return r;
                }
            }
        }
        else
        {
            Monitor *m = (Monitor *)ps->monitor;
            if (m->gpioNum == inputGpio)
            {
                SignalDataSource *r = (SignalDataSource *)malloc(sizeof(SignalDataSource));
                r->monitor = m;
                r->ps = ps;
                return r;
            }
        }
    }
    return NULL;
}
