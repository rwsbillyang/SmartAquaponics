## Introduction
An aquapponic smart control system running on [ESP32-C3](https://github.com/espressif/esp-idf)

## Features

- Supports main pump which is controlled by timer or water level monitor sensors. Default: turn off when water is high-level, and turn off when water is low-level;
- Supports wave pump which is controlled by timer;
- Supports water heater which is controlled by ds18b20 sensor. Default: turn on heater if water temparature < 20 degree, and turn off if temparature >25
- Supports auto feeder which is controlled by timer;
- supports monitor the temperature and humidity based on dht11 sensor;
- TODO: night lights
 
## How to 

```sh
source ~/esp/esp-idf/export.sh
idf.py set-target esp32c3
idf.py menuconfig
idf.py build
```

 More Info :  [Esp doc: Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/index.html) 


## License: GPLv2

