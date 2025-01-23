
#pragma once


#include <stdlib.h>
#include <inttypes.h>

#define StartDelayMs "StartDelayMs"
#define OnMs "OnMs"
#define OffMs "OffMs"
#define WorkModeCfg "WorkMode"



#ifdef __cplusplus
"C"
{
#endif

int save(char *key, uint32_t value);
int getValue(char *key);

#ifdef __cplusplus
}
#endif