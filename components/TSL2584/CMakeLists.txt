idf_component_register(SRCS "TSL2584.c"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES esp_driver_gpio 
                                  esp_driver_i2c
                                  I2C
                    )
