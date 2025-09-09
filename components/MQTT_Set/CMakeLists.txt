idf_component_register(SRCS "MQTT_Set.c"
                    INCLUDE_DIRS "include"
                    REQUIRES mqtt json SHT20 Camear JDQ
                    )
