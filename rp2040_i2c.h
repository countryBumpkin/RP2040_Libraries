#include <hardware/i2c.h>
#include <pico/stdlib.h>

/*
 * All the functions needed to intialize and configure the I2C interface on the pico
 */
void init_i2c(i2c_inst_t* i2c_port){
    i2c_init(i2c_port, 100*1000); // set port and set baud rate to 100kHz
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);
    sleep_ms(500);
}
