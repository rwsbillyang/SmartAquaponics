#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <inttypes.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <inttypes.h>

#include "monitor_data_to_powerstate.h"


void releaseDataToPowerStateMap(DataToPowerStateMap *self)
{
    DataToPowerState *e;
    for (int i = 0; i < self->count; i++)
    {
        e = self->array[i];
        e->release(e);
    }
    free(self);
}
void releaseDataToPowerState(DataToPowerState *self)
{
    free(self);
}

static void serializeDataToPowerState(DataToPowerState *self)
{
}
static DataToPowerState *deserializeDataToPowerState()
{
    return NULL;
}

static void serializeDataToPowerStateMap(DataToPowerStateMap *self)
{
}
static DataToPowerStateMap *deserializeDataToPowerStateMap()
{
    return NULL;
}

DataToPowerState *createDataToPowerState(enum Op op, int threshold, enum PowerState powerState)
{
    uint16_t size = sizeof(DataToPowerState);
    DataToPowerState *me = (DataToPowerState *)malloc(size);
    me->op = op;
    me->threshold = threshold;
    me->powerState = powerState;
    me->serialize = serializeDataToPowerState;
    me->deserialize = deserializeDataToPowerState;
    me->release = releaseDataToPowerState;
    
    return me;
}
DataToPowerStateMap *createDataToPowerStateMap(int num, ...)
{
    DataToPowerState **optionArray = (DataToPowerState **)calloc(num, sizeof(DataToPowerState *));
    va_list valist;
    va_start(valist, num);
    for (int i = 0; i < num; i++)
    {
        optionArray[i] = va_arg(valist, DataToPowerState *);
    }
    va_end(valist);

    DataToPowerStateMap *me = (DataToPowerStateMap *)malloc(sizeof(DataToPowerStateMap));
    me->array = optionArray;
    me->count = num;
    me->serialize = serializeDataToPowerStateMap;
    me->deserialize = deserializeDataToPowerStateMap;
    me->release = releaseDataToPowerStateMap;

    return me;
}

enum PowerState data2PowerState(int signal, DataToPowerState **array, uint16_t count)
{
    for (int i = 0; i < count; i++)
    {
        DataToPowerState *option = array[i];
        switch (option->op)
        {
        case eq:
        {
            if (signal == option->threshold)
            {
                return option->powerState;
            }
            break;
        }
        case lt:
        {
            if (signal < option->threshold)
            {
                return option->powerState;
            }
            break;
        }
        case lte:
        {
            if (signal <= option->threshold)
            {
                return option->powerState;
            }
            break;
        }
        case gt:
        {
            if (signal > option->threshold)
            {
                return option->powerState;
            }
            break;
        }
        case gte:
        {
            if (signal >= option->threshold)
            {
                return option->powerState;
            }
            break;
        }
        default:
        {
            printf("Not support Op=%d", option->op);
            break;
        }
        }
    }

    return PowerState_None;
}