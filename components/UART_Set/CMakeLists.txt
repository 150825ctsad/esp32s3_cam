idf_component_register(
    SRCS "UART_Set.c"
    INCLUDE_DIRS "include"
    REQUIRES esp_driver_uart esp_driver_gpio
)