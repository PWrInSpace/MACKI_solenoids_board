// Copyright 2024 PWrInSpace, Kuba
#include "ble_task.h"
#include "cli_task.h"
#include "cmd.h"
#include "driver/gpio.h"
#include "esp_console.h"
#include "esp_log.h"
#include "load_cell.h"
#include "valves_task.h"

#define TAG "Main"

void app_main(void) {
    ESP_LOGI(TAG, "Command history disabled");

    printf(
        "\033[0;35m\n"
        "_|      _|    _|_|      _|_|_|  _|    _|  _|_|_|\n"
        "_|_|  _|_|  _|    _|  _|        _|  _|      _|  \n"
        "_|  _|  _|  _|_|_|_|  _|        _|_|        _|  \n"
        "_|      _|  _|    _|  _|        _|  _|      _|  \n"
        "_|      _|  _|    _|    _|_|_|  _|    _|  _|_|_|\n"
        "\n\033[0m");

    uint8_t valves_pins[NUMBER_OF_VALVES] = {GPIO_NUM_2, GPIO_NUM_16};

    valves_init(valves_pins);
    load_cell_init(GPIO_NUM_13, GPIO_NUM_14);
    load_cell_set_raw_offset(0);  // TBD
    load_cell_set_scale(1);       // TBD

    esp_console_config_t console_config = {
        .max_cmdline_args = 8,
        .max_cmdline_length = 256,
    };
    esp_console_init(&console_config);

    // Wait for the queue to initiate
    ble_init_task(NULL);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    cmd_register_common();
    cmd_register_valves();
    cmd_register_load_cell();
    cli_init(console_config.max_cmdline_length);
    cli_run();

}
