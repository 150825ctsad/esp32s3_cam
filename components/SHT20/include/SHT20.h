#ifndef SHT20_H
#define SHT20_H

#include "esp_err.h"
#include "driver/i2c_master.h"
#include <stdio.h>
/* SHT20.h */
extern uint16_t STH20_WData;   // 原始温度
extern uint16_t STH20_SData;   // 原始湿度
void Z_STH20_GetData(i2c_master_dev_handle_t i2c_device_handle,uint8_t command);
void sth20_app(i2c_master_dev_handle_t i2c_device_handle);
void sth20_init(i2c_master_dev_handle_t i2c_device_handle);
extern i2c_master_dev_handle_t sth_dev;
#endif // STH20_H
