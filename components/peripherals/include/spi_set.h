#ifndef _SPI_SET_H_
#define _SPI_SET_H_

#include <stdio.h>
#include "string.h"

#include "hal/spi_types.h"
#include "driver/spi_common.h"
#include "driver/spi_master.h"
#include "freertos/FreeRTOS.h" 
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include <stdint.h>



#define PIN_NUM_CLK     39//5
#define PIN_NUM_MOSI    40//4
#define PIN_NUM_MISO    41//16
#define PIN_NUM_CS      42//7

#define PIN_BK_LIGHT    45//17
#define PIN_NUM_RST     47//15
#define PIN_NUM_DC      48//6

#define LCD_WIDTH      240
#define LCD_HEIGHT     320

#define CHUNK_BYTES   4092          // ≤4092 字节


void spi_init();

#endif