#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <inttypes.h>

#include "monitor.h"
#include "monitor_data_to_powerstate.h"

static void serialize(Monitor *m)
{
    // TODO
}
static Monitor *deserialize()
{
    // TODO
    return NULL;
}

/**
 * uninit , free all memory including name, subObject, and Monitor self
 */
static void release(Monitor *m)
{
    m->device->uninit(m);
    free(m->name);
    enum SubObjectType t = m->subType;
    if (t == SubType_DataToPowerStateMap)
    {
        DataToPowerStateMap *sub = (DataToPowerStateMap *)m->subObject;
        sub->release(sub);
    }
    else
    {
        free(m->subObject);
    }

    free(m);
}

/**
 * create a Monitor and init it by calling its init method
 */
Monitor *createMonitor(char *name, enum MonitorType monitorType, int32_t gpioNum, IDevice *device, void *subObject, enum SubObjectType subType)
{
    char *p = (char *)malloc(strlen(name) * sizeof(char));
    strcpy(p, name);

    Monitor *m = (Monitor *)malloc(sizeof(Monitor));
    m->name = p;
    m->monitorType = monitorType;
    m->gpioNum = gpioNum;
    m->subObject = subObject;
    m->subType = subType;
    m->serialize = serialize;
    m->deserialize = deserialize;
    m->release = release;

    device->init(m);

    return m;
}
