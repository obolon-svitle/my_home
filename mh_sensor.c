#include "mh_i2c.h"
#include "mh_sensor.h"
#include "mh_event.h"
#include "mh_utils.h"

#define I2C_CLOCK (100000UL)
#define DEVICE_ADDR (0x38)

#define SENSOR_ADDR 0x76
#define TEMP_REGISTER 0xfa

//TODO:
#define SENSOR_CTRL_REGISTER 0x00
#define SENSOR_CTRL_MEASURE_TEMP 0x00

void mh_sensor_init(void) {
    i2c_init(DEVICE_ADDR, I2C_CLOCK);
}

void mh_sensor_deinit(void) {
    i2c_deinit();
}

void mh_sensor_read_start(void) {
    i2c_master_write_reg_async(SENSOR_ADDR, SENSOR_CTRL_REGISTER, SENSOR_CTRL_MEASURE_TEMP);
}

void mh_sensor_read_stop(void) {
}

void i2c_master_read_complete(uint32_t value) {
    MH_EVENT_PUT(MH_EVENT_READ_SENSOR_COMPLETE, temp_value, value);
}

void i2c_master_write_complete(void) {
    i2c_master_read_reg_async(SENSOR_ADDR, TEMP_REGISTER);
}
