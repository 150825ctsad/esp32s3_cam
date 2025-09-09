idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                                 "../components/MAX471"
                                 "../components/SHT20"
                                 "../components/Camear"
                                 "../components/JDQ"
                    REQUIRES esp_lcd 
                             driver 
                             peripherals 
                             LCD
                             MAX471
                             SHT20
                             Camear
                             JDQ
                             TSL2584
                             I2C
                             WIFI_Set
                    )