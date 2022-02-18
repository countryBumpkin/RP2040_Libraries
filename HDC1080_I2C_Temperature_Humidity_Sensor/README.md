# HDC1080 I2C Driver (C++)
The HDC1080 is a simple and cheap temperature and relative humidity sensor that communicates via I2C. As of 02/18/2022 there do not appear to be any public I2C drivers for this sensor when working with the RP2040 microcontroller. This is a driver I wrote while studying Real Time Operating Systems. It is friendly to new users and provides enough configurability for advanced users to get the level of control needed for advanced projects.

## Pre-requisites
1. Pico SDK installed
2. CMakeLists.txt configured to compile C++ source code. For example, include this line: `project(6_HDC1080_I2C_Driver C CXX ASM)`
3. Initialize I2C on the Pico:
```C++  
i2c_init(I2C_PORT_DEFAULT, 100*1000); // set port and set baud rate to 100kHz
gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);
```

## Resources
1. [HDC1080 Datasheet](https://www.ti.com/lit/ds/symlink/hdc1080.pdf?ts=1644618263104&ref_url=https%253A%252F%252Fwww.ti.com%252Fproduct%252FHDC1080)
2. [Pico I2C API](https://raspberrypi.github.io/pico-sdk-doxygen/group__hardware__i2c.html)
3. [Pico Documentation](https://www.raspberrypi.com/documentation/microcontrollers/rp2040.html) see the 'Getting Started with Raspberry Pi Pico' for a guide on setting up C++ development on your machine... and other helpful stuff.



# Usage
This library provides two levels of control for reading from the sensor. 
 1. High Level (simplest)
    * Library hides as much of the nitty gritty I2C interface as possible. User can read sensor with a single API call, library handles all timing required for the sensor and returns readings as float values.
    * Since the sensor requires 7ms to read high resolution values this method is unacceptable for any user operating in an RTOS environment. You should use the low level control.

2. Low Level (more involved)
    * User tells the sensor to take a measurement. Then the user is responsible for waiting at least 7ms per measurement before attempting to read from the sensor.
    * In an RTOS such as FreeRTOS this prevents the I2C driver from **disrupting the OS scheduler**.

### Driver Initialization
Run the following code to create a new HDC1080 object in C++.
```C++
HDC1080* hdc_sensor = new HDC1080(I2C_PORT_DEFAULT); // this should be either i2c0 (pico default) or i2c1, HDC1080() will use the pico default
uint8_t hdc_man_id = hdc_sensor->read_manufacturer_id(); // manufacturer ID is a good test, should always be 0x5449 (Texas Instruments)
```
    
## High Level Usage
To read temperature and humidity individually... (call takes 7ms+ for sensor read)
```C++
float temp = hdc_sensor->fahrenheit(); // use fahrenheit(MEDIUM_RES); to read a 11 bit output instead of 14 bit
float temp_celsius = hdc_sensor->celsius();
float rel_hum = hdc_sensor->humidity(HIGH_RES); // pass MEDIUM_RES for 11 bit, LOW_RES for 8 bit
```
  
or, to read both at the same time... (call takes 14ms+ for sensor read)
```C++
float measurements[2];
hdc_sensor->read_both(CELSIUS, HIGH_RES, &measurements, 2); // reads temperature in C, stores in measurements[0], humidity as percentage in index 1

// alternatively, to read temperature in Celsius and Fahrenheit at the same time...

hdc_sensor->read_both(0, HIGH_RES, &measurements, 3); // measurements[0] = C, [1] = RH, [2] = F
```
    
## Low Level Usage
Helpful for working with RTOS systems because it leaves the programmer in control of timing as much as possible.

Reading individual measurements...
```C++
hdc_sensor->trigger_temp_measurement(HIGH_RES);
// hdc_sensor->trigger_humidity_measurement(MEDIUM_RES); to measure RH
uint16_t raw_output = hdc_sensor->read_raw(); // reads the raw register value from I2C bus
float temp_f = hdc_sensor->raw_to_float(raw_output, TEMPERATURE_F); // TEMPERATURE_C for output in Celsius, HUMIDITY to get relative humidity
```
    
Reading both at the same time...
```C++
hdc_sensor->trigger_both();
//
// wait for 14ms for readings to complete on HDC1080
//
float measurement[3];
uint16_t temp_raw, hum_raw;
// read output
hdc_sensor->read_both_raw(&temp_raw, &hum_raw);
// convert output
measurement[0] = hdc_sensor->raw_to_float(temp_raw, TEMPERATURE_C);
measurement[2] = hdc_sensor->raw_to_float(temp_raw, TEMPERATURE_F);
measurement[1] = hdc_sensor->raw_to_float(hum_raw, HUMIDITY);
```

