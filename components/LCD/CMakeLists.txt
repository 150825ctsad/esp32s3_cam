idf_component_register(SRCS "lcd_set.c"
                    INCLUDE_DIRS "include"
                    REQUIRES esp_driver_spi esp_driver_gpio
                    PRIV_REQUIRES peripherals 
                    )