// Copyright 2024 PWrInSpace, Kuba
#include "esp_log.h"
#include "esp_console.h"
#include "cli_task.h"
#include "valves_cmd.h"
#include "valves_task.h"

#define TAG "Main"

void app_main(void) {
    ESP_LOGI(TAG, "Command history disabled");

    printf("\033[0;35m\n"
           "_|      _|    _|_|      _|_|_|  _|    _|  _|_|_|\n"
           "_|_|  _|_|  _|    _|  _|        _|  _|      _|  \n"
           "_|  _|  _|  _|_|_|_|  _|        _|_|        _|  \n"
           "_|      _|  _|    _|  _|        _|  _|      _|  \n"
           "_|      _|  _|    _|    _|_|_|  _|    _|  _|_|_|\n"
           "\n\033[0m");

    uint8_t valves_pins[NUMBER_OF_VALVES] = {2, 13};

    valves_init(valves_pins);

    esp_console_config_t console_config = {
        .max_cmdline_args = 8,
        .max_cmdline_length = 256,
    };
    esp_console_init(&console_config);

    cmd_register_valves();
    cli_init(console_config.max_cmdline_length);
    cli_run();
}
