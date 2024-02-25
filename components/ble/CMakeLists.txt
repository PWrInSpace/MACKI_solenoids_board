idf_component_register(SRCS "ble_api.c" "ble_gap_conf.c" "ble_gatt_conf.c"
                    INCLUDE_DIRS "."
                    REQUIRES cmock driver soc esp_rom ble bt nvs_flash)

target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-format -Wall -W -Werror)