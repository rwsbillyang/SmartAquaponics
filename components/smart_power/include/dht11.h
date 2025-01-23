#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /** DHT11 init
     * @param dht11_pin GPIO pin number
     * @return 无
     */
    void dht11Init(uint8_t dht11_pin);

    /** read DHT11 data
     * @param temprature 温度值X10
     * @param humidity 湿度值
     * @return 无
     */
    int dht11Read(uint8_t dht11_pin, int *temprature, int *humidity);

#ifdef __cplusplus
}
#endif