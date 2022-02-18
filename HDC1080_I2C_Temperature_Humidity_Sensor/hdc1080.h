/*
 * I2C driver for the HDC1080 temperature and humidity sensor
 * link: https://www.ti.com/product/HDC1080?keyMatch=HDC1080&tisearch=search-everything&usecase=GPN
 *
 * Author: Garrett Wells
 * Date: 02/12/2022
 */
#ifndef HDC1080_H
#define HDC1080_H

#include <string>
#include <hardware/i2c.h>

using namespace std;

enum Degrees {CELSIUS=0, FAHRENHEIT=1};
enum HDC_Measure {TEMPERATURE_C, TEMPERATURE_F, HUMIDITY};
enum HDC_Resolution {HIGH_RES=14, MEDIUM_RES=11, LOW_RES=8};
enum HDC_Config {SINGLE_14=0x00,    /*read temp/humidity at 14 bits*/
            TEMP_11=0x04,           /*config to read 11 bit temperature*/
            HUM_11=0x01,            /*read 11 bit humidity*/
            HUM_8=0x02,             /*read 8 bit humidity*/
            COMBO_14=0x10,          /*read temp and humidity at 14 bit res*/
            COMBO_11=0x15,          /*read both at 11 bit resolution*/
            RESET=0x10,             /*reset config register, will not read*/
            HEATER_ON=0x20,         /*turn on the heater*/
            HEATER_OFF=0x10};       /*same as reset, actually same as all other values here*/

/*
 * Define the API for reading from the HDC1080 sensor via I2C
 */
class HDC1080 {
    private:
        // register pointers
        const uint8_t HDC1080_ADDR=0x40, /*default address for the HDC1080*/
            HDC_TEMP=0x00,      /*temperature register*/
            HDC_HUM=0x01,       /*humidity register*/
            HDC_CONFIG=0x02,    /*configuration register*/
            HDC_UID_1=0xFB,     /*unique ID register 1*/
            HDC_UID_2=0xFC,     /*unique ID register 2*/
            HDC_UID_3=0xFC,     /*unique ID register 2*/
            MAN_ID=0xFE,        /*manufacturer ID for TI*/
            DEV_ID=0xFF;        /*device ID*/

        i2c_inst_t* I2C_PORT=i2c0;
        float temperature(Degrees, HDC_Resolution);

    public:
        HDC1080(i2c_inst_t* i2c_port);

        void set_config(HDC_Config);    // set the device for measurement, heater, checking battery voltage
        void set_heater(bool);          // set heater on/off to remove condensation from the humidity sensor

        uint8_t read_config();  // read the bits of the configuration register

        void read_both(Degrees, HDC_Resolution, float*, int);    // read temperature and humidity at the same time and save to pointer
        float fahrenheit(HDC_Resolution);               // read temperature with custom resolution
        float celsius(HDC_Resolution);                  // ....
        float fahrenheit(void);                         // read temp in farenheit with default 14 bit resolution
        float celsius(void);                            // ....
        float humidity(HDC_Resolution);                 // read humidity with custom resolution

        void trigger_temp_measurement(HDC_Resolution);  // trigger a read of the temperature sensor without waiting for the result
        void trigger_humidity_measurement(HDC_Resolution); // trigger a sensor read of the humidity sensor without waiting for result
        void trigger_both(HDC_Resolution);

        uint16_t read_raw();                            // get the raw 16 bit output of the temperature or humidity sensor
        void read_both_raw(uint16_t*, uint16_t*);

        uint16_t read_manufacturer_id(void);            // read the TI manufacturer ID, should be 0x5449
        uint64_t read_UID(void);                        // read the 40 bit unique ID, aka serial number

        float raw_to_float(uint16_t, HDC_Measure);  // convert a 16 bit raw sensor output to humidity or temperature in float form

};

#endif
