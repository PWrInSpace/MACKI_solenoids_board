// Copyright 2024 PWrInSpace, Kuba
#include "esp_log.h"
#include "esp_console.h"
#include "cli_task.h"
#include "driver/gpio.h"

#include "tensometer_cmd.h"
#include "tensometer.h"
#include "valves_cmd.h"
#include "valves_task.h"
#include "common_cmd.h"

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

    uint8_t valves_pins[NUMBER_OF_VALVES] = {GPIO_NUM_2, GPIO_NUM_16};

    valves_init(valves_pins);
    tensometer_init(GPIO_NUM_13, GPIO_NUM_14);

    esp_console_config_t console_config = {
        .max_cmdline_args = 8,
        .max_cmdline_length = 256,
    };
    esp_console_init(&console_config);

    cmd_register_common();
    cmd_register_valves();
    cmd_register_tensometer();
    cli_init(console_config.max_cmdline_length);
    cli_run();
}
