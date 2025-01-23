#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <inttypes.h>

#include "monitor.h"
#include "monitor_complex.h"

static void serialize(ComplexMonitor *m)
{
    //TODO
}
static ComplexMonitor *deserialize()
{
    //TODO
    return NULL;
}

static void release(ComplexMonitor *me)
{
    Monitor *m = NULL;
    for (int i = 0; i < me->count; i++) // 逐个访问可变参数
    {
        m = me->monitors[i];
        m->release(m);
    }
    free(me->name);
    free(me->monitors);
    free(me);
}

ComplexMonitor *createComplexMonitor(char *name, int count, ...)
{
    char *p = (char *)malloc(strlen(name) * sizeof(char));
    strcpy(p, name);

    ComplexMonitor *me = (ComplexMonitor *)malloc(sizeof(ComplexMonitor));
    me->name = p;
    me->monitorType = Type_Complex;
    me->monitors = (Monitor **)malloc(sizeof(Monitor *) * count);
    me->count = count;
    me->serialize = serialize;
    me->deserialize = deserialize;
    me->release = release;

    va_list args;
    va_start(args, count); // 初始化 args 以访问可变参数
    Monitor *m = NULL;
    for (int i = 0; i < count; i++) // 逐个访问可变参数
    {
        m = va_arg(args, Monitor *);
        m->device->init(m);
        me->monitors[i] = m;
    }
    va_end(args); // 清理 args

    return me;
}