#ifndef _MH_I2C_H_
#define _MH_I2C_H_

#include <stdint.h>
#include "mh_types.h"

void i2c_init(unsigned master_addr, unsigned clock_rate);
void i2c_deinit(void);

void i2c_master_read_reg(uint32_t slave_addr, uint32_t byte, uint32_t *value);
void i2c_master_write_reg(uint32_t slave_addr, uint32_t reg, uint32_t value);

void i2c_master_read_reg_async(uint32_t slave_addr, uint32_t reg);
void i2c_master_write_reg_async(uint32_t slave_addr, uint32_t reg, uint32_t value);

//TODO: Replace it with possibility to pass a callback to i2c read/write_async funcs
__attribute__((weak)) void i2c_master_read_complete(uint32_t value);
__attribute__((weak)) void i2c_master_write_complete(void);

#endif /* _MH_I2C_H_ */
