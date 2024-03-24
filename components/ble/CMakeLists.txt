idf_component_register(SRCS "ble_api.c" "ble_gap_conf.c" "ble_gatt_conf.c" "ble_task.c"
                    INCLUDE_DIRS "."
                    REQUIRES cmock driver soc esp_rom ble bt nvs_flash console cmd)

target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-format -Wall -W -Werror)