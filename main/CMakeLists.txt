idf_component_register(
    SRCS
        "main.c"
        "wifi_manager.c"
        "modbus_master.c"
        "devices/dtsu666.c"
    INCLUDE_DIRS
        "."
        "devices"
    REQUIRES
        nvs_flash
        esp_wifi
        esp_event
        esp_netif
        esp_timer
        esp-modbus
        
)
