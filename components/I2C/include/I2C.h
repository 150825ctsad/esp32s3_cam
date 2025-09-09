#ifndef I2C_H
#define I2C_H

#include "driver/i2c_master.h"

esp_err_t i2c_master_init(i2c_master_dev_handle_t *i2c_device_handle,i2c_master_dev_handle_t *i2c_device_handle1);

#endif // I2C_H
