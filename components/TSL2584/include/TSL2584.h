#ifndef TSL2584_H
#define TSL2584_H


#include "esp_err.h"
#include <stdlib.h>
#include "driver/i2c_master.h"

// TSL2584设备结构体
// 函数声明
void tsl2584_init(i2c_master_dev_handle_t i2c_dev);
void tsl2584_app(i2c_master_dev_handle_t i2c_dev);
int tsl2584_read_lux_x100(void);

extern i2c_master_dev_handle_t tsl_dev;
#endif // TSL2584_H