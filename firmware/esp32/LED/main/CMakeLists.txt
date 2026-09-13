idf_component_register(SRCS "main.c" "src/mqtt_app.c" "src/wifi_app.c" "src/led_app.c"
                    INCLUDE_DIRS "./inc"
                    REQUIRES mqtt json driver nvs_flash esp_netif esp_wifi led_strip)