idf_component_register(SRCS "Camear.c"
                    INCLUDE_DIRS "include"
                    REQUIRES esp_driver_gpio esp32-camera esp_psram LCD peripherals
                    )
