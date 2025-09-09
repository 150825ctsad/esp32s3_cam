#include <stdio.h>
#include "JDQ.h"
#include "driver/gpio.h"

#define  GPIO_NUM_RELAY 38

void relay_init(void)
{
    // 配置GPIO引脚
    gpio_config_t conf;
    // 设置为输出模式
    conf.mode = GPIO_MODE_OUTPUT;
    // 配置要使用的引脚
    conf.pin_bit_mask = (1ULL << GPIO_NUM_RELAY);

    // 配置GPIO
    gpio_config(&conf);
    
    // 初始状态设为关闭
    relay_off();
}

void relay_on(void)
{
    // 设置引脚为高电平（打开继电器）
    gpio_set_level(GPIO_NUM_RELAY, 1);
}

void relay_off(void)
{
    // 设置引脚为低电平（关闭继电器）
    gpio_set_level(GPIO_NUM_RELAY, 0);
}
