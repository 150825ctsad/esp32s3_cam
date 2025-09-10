idf_component_register(SRCS "Http.c"
                    INCLUDE_DIRS "." "include"
                    REQUIRES esp_http_server esp32-camera freertos esp_timer
                    )