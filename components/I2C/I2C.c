#include <stdio.h>
#include "I2C.h"
#include "driver/i2c_master.h"

esp_err_t i2c_master_init(i2c_master_dev_handle_t *i2c_device_handle,i2c_master_dev_handle_t *i2c_device_handle1){
i2c_master_bus_config_t i2c_config = {
    .i2c_port = I2C_NUM_0,  // 使用I2C0
    .sda_io_num = GPIO_NUM_2,  // SDA引脚
    .scl_io_num = GPIO_NUM_3,  // SCL引脚
    .clk_source = I2C_CLK_SRC_DEFAULT,
    .glitch_ignore_cnt = 7,
    .flags.enable_internal_pullup = 1,  // 启用内部上拉
};
    i2c_master_bus_handle_t i2c_bus;
    i2c_new_master_bus(&i2c_config,&i2c_bus);

i2c_device_config_t dev_cfg = {
    .device_address = 0x40,  // SHT20的I2C地址
    .scl_speed_hz = 100000,  // SCL频率设置为100kHz
};

    esp_err_t ret = i2c_master_bus_add_device(i2c_bus, &dev_cfg, i2c_device_handle);

i2c_device_config_t tsl_cfg = {
    .device_address = 0x39,  // TSL2561的I2C地址
    .scl_speed_hz = 100000,  // SCL频率设置为100kHz
};

    esp_err_t ret1 = i2c_master_bus_add_device(i2c_bus, &tsl_cfg, i2c_device_handle1);

    if (ret != ESP_OK || ret1 != ESP_OK) {
        printf("Failed to add I2C device: %s\n", esp_err_to_name(ret));
        return ret;
    }
    return ESP_OK;
}