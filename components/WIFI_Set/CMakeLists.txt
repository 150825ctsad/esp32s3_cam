idf_component_register(SRCS "WIFI_Set.c"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES MQTT_Set
                    REQUIRES nvs_flash esp_wifi esp_event
                    )
