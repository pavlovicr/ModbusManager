idf_component_register(
    SRCS "main.c" "wifi_manager.c"
    INCLUDE_DIRS "."
    REQUIRES nvs_flash esp_wifi esp_event esp_netif esp_timer esp-modbus modbus_core
    PRIV_REQUIRES DTSU666H SOILHUMIDITY_1
)
