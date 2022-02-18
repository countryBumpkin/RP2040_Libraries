/*
 * I2C driver for the HDC1080 temperature and humidity sensor
 * link: https://www.ti.com/product/HDC1080?keyMatch=HDC1080&tisearch=search-everything&usecase=GPN
 *
 * Author: Garrett Wells
 * Date: 02/12/2022
 */
#include "hdc1080.h"

/*
 * Do the default initialization for the HDC1080
 *  Initially starts in the low power state and cannot be read from until configured through
 *  one of the functions below.
 */
HDC1080::HDC1080(i2c_inst_t* i2c_port){
    I2C_PORT = i2c_port;
}

/*
 * Write a 16 bit config value to the config register. This must be done before taking a measurement.
 *  Also use this for reading battery voltage warnings and turning on the heater.
 */
void HDC1080::set_config(HDC_Config c_value){
    const uint8_t CONFIG_REG=0x02;
    uint8_t data[] = {CONFIG_REG, c_value, 0x00}; // write three bytes, last should always be 0x00
    // pointer write to 0x02 with the config values following
    int result = i2c_write_blocking(I2C_PORT, HDC1080_ADDR, &data[0], 3, false);
    if(result==PICO_ERROR_GENERIC){ // check if valid
        printf("[ERROR] WRITE POINTER I2C unable to write bytes to HDC1080 config, address not acknowledged or no device\n");
        return;
    }
}

uint8_t HDC1080::read_config(){
    const uint8_t CONFIG_REG=0x02;

    // set pointer for read
    int result = i2c_write_blocking(I2C_PORT, HDC1080_ADDR, &CONFIG_REG, 1, false);
    if(result==PICO_ERROR_GENERIC){ // check if valid
        printf("[ERROR] WRITE POINTER I2C unable to write bytes to HDC1080 config, address not acknowledged or no device\n");
        return 0x00;
    }

    uint8_t output[2];
    result = i2c_read_blocking(I2C_PORT, HDC1080_ADDR, output, 2, false);
    if(result==PICO_ERROR_GENERIC){ // check if valid
        printf("[ERROR] WRITE POINTER I2C unable to write bytes to HDC1080 config, address not acknowledged or no device\n");
        return 0x00;
    }

    return output[0];
}

/*
 * Read both temperature then humidity and place in the provided pointer
 *  Returns an array of measurements:
 *      dst[0] = temperature in Celsius
 *      dst[1] = humidity (%RH)
 *      dst[2] = temperature in Fahrenheit, only if size is 3, otherwise just the first two
 */
void HDC1080::read_both(Degrees degrees, HDC_Resolution res, float* dst, int size){
    if(dst == NULL){ // check for invalid inputs
        printf("[ERROR] NULL destination ptr(dst) passed to read_both()\n");
        return;
    }

    // config register
    if(res==HIGH_RES){
        set_config(COMBO_14);
    }else{
        set_config(COMBO_11);
    }

    // trigger measurement
    const uint8_t TEMP_REG=0x00;
    int result = i2c_write_blocking(I2C_PORT, HDC1080_ADDR, &TEMP_REG, 1, false);
    if(result==PICO_ERROR_GENERIC)
        printf("[ERROR] WRITE POINTER I2C unable to write, address not acknowledged or available\n");

    // wait for the measurement to complete
    if(res==HIGH_RES){
        sleep_ms(14);
    }else{
        sleep_ms(8);
    }

    // read the values from the sensor in one read operation
    uint8_t output[4];
    result = i2c_read_blocking(I2C_PORT, HDC1080_ADDR, &output[0], 4, false);
    if(result==PICO_ERROR_GENERIC)
        printf("[ERROR] READ I2C unable to write, address not acknowledged or available\n");

    //printf("TEMP: Output[0]=0x%X, Output[1]=0x%X\n", output[0], output[1]);
    //printf("HUM: Output[2]=0x%X, Output[3]=0x%X\n", output[2], output[3]);

    // convert temp value to float
    uint16_t raw_bits = output[0]<<8|output[1];
    double mid_rep = ((double)raw_bits)/((double)65536);
    if(size == 3){ // output C and F
        dst[0] = mid_rep*165 - 40;
        dst[2] = dst[0]*1.8 + 32;

    }else if(degrees==CELSIUS){ // convert to C
        dst[0] = mid_rep*165 - 40;

    }else{ // convert to F
        mid_rep = mid_rep*165 - 40;
        dst[0] = mid_rep*1.8 + 32;
    }

    // convert humidity value to float
    raw_bits = output[2]<<8|output[3];
    mid_rep = ((double)raw_bits)/((double)65536);
    dst[1] = mid_rep*100;
}

/*
 * Read the humidity register to get a value +/- 2%.
 *  Returns the relative humidity as a percentage ex. 40%
 */
float HDC1080::humidity(HDC_Resolution res){
    const uint8_t HUM_REG=0x01; // temperature register pointer
    uint8_t output[2];

    // set config to read just temperature
    if(res==HIGH_RES){
        set_config(SINGLE_14);
    }else if(res==MEDIUM_RES){
        set_config(HUM_11);
    }else{
        set_config(HUM_8);
    }

    // trigger measurement
    int result = i2c_write_blocking(I2C_PORT, HDC1080_ADDR, &HUM_REG, 1, false);
    if(result==PICO_ERROR_GENERIC)
        printf("[ERROR] Writing Humidity: I2C address not acknowledged or no device present\n");

    // wait for measurement to complete, 7ms (14bit), 4ms (11bit)
    if(res==HIGH_RES)
        sleep_ms(7);
    else if(res==MEDIUM_RES){
        sleep_ms(4);
    }else{
        sleep_ms(3);
    }

    // read the humidity register, returns 16 bits, first two are always 0
    result = i2c_read_blocking(I2C_PORT, HDC1080_ADDR, output, 2, false);

    if(result==PICO_ERROR_GENERIC){
        printf("[ERROR] Reading Humidity: I2C address not acknowledged or no device present\n");
        return 0;
    }else{
        //printf("\t\tOutput[0]:0x%X\n\t\tOutput[1]:0x%X\n", output[0], output[1]);
        int16_t raw_bit_hum = output[0]<<8|output[1];
        double mid_rep = ((double)raw_bit_hum)/((double)65536);
        float hum = mid_rep*100;
        return hum;
    }
}

/*
 * Get the current temperature in fahrenheit
 */
float HDC1080::fahrenheit(HDC_Resolution res){
    return temperature(FAHRENHEIT, res);
}

/*
 * Get the current temperature in degrees celsius
 */
float HDC1080::celsius(HDC_Resolution res){
    return temperature(CELSIUS, res);
}

/*
 * Get the temperature in degrees fahrenheit at 14 bit resolution
 */
float HDC1080::fahrenheit(void){
    return temperature(FAHRENHEIT, HIGH_RES);
}

/*
 * Get the temperature in degrees celsius at 14 bit resolution
 */
float HDC1080::celsius(void){
    return temperature(CELSIUS, HIGH_RES);
}

/*
 * Config register and trigger the desired measurement. Don't wait for measurement to complete.
 *  Made for use in RTOS applications where we don't want to rely on sleep_ms() to wait for read to complete.
 */
void HDC1080::trigger_temp_measurement(HDC_Resolution res){
    const uint8_t TEMP_REG=0x00; // temperature register pointer

    // set config to read just temperature
    if(res==HIGH_RES){
        set_config(SINGLE_14);
    }else{
        set_config(TEMP_11);
    }

    // trigger measurement
    int result = i2c_write_blocking(I2C_PORT, HDC1080_ADDR, &TEMP_REG, 1, false);
    if(result==PICO_ERROR_GENERIC)
        printf("[ERROR] Writing Temp: I2C address not acknowledged or no device present\n");
}

/*
 * Config register and trigger the desired measurement. Don't wait for the sensor measurement to complete.
 *  Made for use with RTOS where using sleep_ms() would cause problems.
 */
void HDC1080::trigger_humidity_measurement(HDC_Resolution res){
    const uint8_t HUM_REG=0x01; // temperature register pointer

    // set config to read just temperature
    if(res==HIGH_RES){
        set_config(SINGLE_14);
    }else if(res==MEDIUM_RES){
        set_config(HUM_11);
    }else{
        set_config(HUM_8);
    }

    // trigger measurement
    int result = i2c_write_blocking(I2C_PORT, HDC1080_ADDR, &HUM_REG, 1, false);
    if(result==PICO_ERROR_GENERIC)
        printf("[ERROR] Writing Humidity: I2C address not acknowledged or no device present\n");
}

/*
 * Trigger a read on both temperature and humidity sensors
 */
void HDC1080::trigger_both(HDC_Resolution res){
    if(res == HIGH_RES){
        set_config(COMBO_14);
    }else{
        set_config(COMBO_11);
    }

    // trigger measurement
    const uint8_t TEMP_REG=0x00;
    int result = i2c_write_blocking(I2C_PORT, HDC1080_ADDR, &TEMP_REG, 1, false);
    if(result==PICO_ERROR_GENERIC)
        printf("[ERROR] WRITE POINTER I2C unable to write, address not acknowledged or available\n");
}

/*
 * Read a raw sensor output from the HDC1080 and return the uint16_t representation that can be converted elsewhere.
 */
uint16_t HDC1080::read_raw(){
    uint8_t output[2];
    //printf("--- READ RAW ---\n");
    // read the humidity register, returns 16 bits, first two are always 0
    int result = i2c_read_blocking(I2C_PORT, HDC1080_ADDR, output, 2, false);
    if(result==PICO_ERROR_GENERIC){
        printf("[ERROR] Reading Raw: I2C address not acknowledged or no device present\n");
        return 0;
    }

    //printf("\t\tOutput[0]:0x%X\n\t\tOutput[1]:0x%X\n", output[0], output[1]);
    return output[0]<<8|output[1];
}

/*
 * Read both the temperature and humidity after setting the sensor in combo read mode.
 *  Must wait 14ms after triggering measurement to read these.
 */
void HDC1080::read_both_raw(uint16_t* temp, uint16_t* humidity){
    uint8_t output[4];
    int result = i2c_read_blocking(I2C_PORT, HDC1080_ADDR, &output[0], 4, false);
    if(result==PICO_ERROR_GENERIC)
        printf("[ERROR] READ BOTH I2C address not acknowledged or available\n");

    //printf("TEMP: Output[0]=0x%X, Output[1]=0x%X\n", output[0], output[1]);
    //printf("HUM: Output[2]=0x%X, Output[3]=0x%X\n", output[2], output[3]);

    // convert temp value to float
    *temp = output[0]<<8|output[1];

    // convert humidity value to float
    *humidity = output[2]<<8|output[3];
}

/*
 * Convert a 16 bit raw register value to a float temperature or humidity value.
 */
float HDC1080::raw_to_float(uint16_t raw, HDC_Measure des_output){
    double prelim = ((double)raw)/((double)65536); // do base conversion common to temp and humidity
    if(des_output == TEMPERATURE_C){
        return (prelim*165)-40;
    }else if(des_output == TEMPERATURE_F){
        return ((prelim*165) - 40)*1.8 + 32;
    }else{
        return prelim*100;
    }
}

/*
 * Reads the 16 bit manufacturer ID from the HDC1080
 */
uint16_t HDC1080::read_manufacturer_id(){
    // set pointer value to manufacturer id register
    int result = i2c_write_blocking(I2C_PORT, HDC1080_ADDR, &MAN_ID, 1, false);
    if(result==PICO_ERROR_GENERIC)
        printf("[ERROR] I2C unable to write bytes to HDC1080, address not acknowledged or no device\n");

    uint8_t output[2];
    result = i2c_read_blocking(I2C_PORT, HDC1080_ADDR, output, 2, false);
    if(result==PICO_ERROR_GENERIC)
        printf("[ERROR] I2C address not acknowledged or no device present\n");

    int man_id = output[0]<<8|output[1];

    return man_id;
}

/*
 * Read the unique device ID (serial number) from the sensor
 *  Requires reading from 3 registers(40 bits) and accumulating values into one 64bit value to return
 */
uint64_t HDC1080::read_UID(){
    uint8_t target_reg = HDC_UID_1; // start with the first UID register, then increment
    uint16_t accum[3]; // three 16 bit register values read from device

    for(int i = 0; i < 3; i++){
        int result = i2c_write_blocking(I2C_PORT, HDC1080_ADDR, &target_reg, 1, false);
        if(result==PICO_ERROR_GENERIC){ // check if valid
            printf("[ERROR] I2C unable to write bytes to HDC1080, address not acknowledged or no device\n");
            return PICO_ERROR_GENERIC;
        }

        uint8_t output[2];
        result = i2c_read_blocking(I2C_PORT, HDC1080_ADDR, output, 2, false);
        if(result==PICO_ERROR_GENERIC){ // check if valid
            printf("[ERROR] I2C address not acknowledged or no device present\n");
            return PICO_ERROR_GENERIC;
        }

        //printf("Output[%d]: [0]=0x%X, [1]=0x%X\n", i, output[0], output[1]);
        accum[i] = output[0]<<8|output[1];
        //printf("UID Register[%d]: 0x%X\n", i, tmp);
        target_reg += 1; // move to the next register
    }
    uint64_t out = ((uint64_t)accum[0]<<32)|(accum[1]<<16|accum[2]);
    //printf("test = 0x%X, out=0x%llX\n", test, out);
    return out;
}

/*
 * Activate with bit 13 in config register. This can be used to burn moisture off of the sensor
 *  to obtain more accurate readings.
 */
void HDC1080::set_heater(bool heater_on){
    if(heater_on){
        set_config(HEATER_ON);
    }else{
        set_config(HEATER_OFF);
    }
}

/*
 * Read the temperature from the HDC, choose the conversion type (C/F), and the resolution.
 *  Resolution can be HIGH, MEDIUM, or LOW
 */
float HDC1080::temperature(Degrees deg, HDC_Resolution res){
    const uint8_t TEMP_REG=0x00; // temperature register pointer
    uint8_t output[2];

    // set config to read just temperature
    if(res==HIGH_RES){
        set_config(SINGLE_14);
    }else{
        set_config(TEMP_11);
    }

    // trigger measurement
    int result = i2c_write_blocking(I2C_PORT, HDC1080_ADDR, &TEMP_REG, 1, false);
    if(result==PICO_ERROR_GENERIC)
        printf("[ERROR] Writing Temp: I2C address not acknowledged or no device present\n");

    // wait for measurement to complete, 7ms (14bit), 4ms (11bit)
    if(res==HIGH_RES)
        sleep_ms(7);
    else if(res==MEDIUM_RES){
        sleep_ms(4);
    }

    // read the temperature register
    result = i2c_read_blocking(I2C_PORT, HDC1080_ADDR, output, 2, false);

    if(result==PICO_ERROR_GENERIC){
        printf("[ERROR] Reading Temp: I2C address not acknowledged or no device present\n");
        return 0;
    }else{
        //printf("\t\tOutput[0]:0x%X\n\t\tOutput[1]:0x%X\n", output[0], output[1]);
        // convert raw bits to float
        int16_t raw_bit_temp = output[0]<<8|output[1];
        double mid_rep = ((double)raw_bit_temp)/((double)65536);
        float temp = mid_rep*165 - 40;

        if(deg==CELSIUS)
            return temp; // return C

        return (temp*1.8)+32; // return F
    }
}
