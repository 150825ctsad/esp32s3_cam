idf_component_register(SRCS "I2C.c"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES esp_driver_i2c 
                                  I2C
                    )
