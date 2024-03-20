// Copyright 2024 PWrInSpace, Kuba
#include "esp_log.h"
#include "esp_console.h"
#include "cli_task.h"
#include "driver/gpio.h"
#include "cmd.h"
#include "load_cell.h"
#include "valves_task.h"
#include "A4988.h"
#include "stepper_motor_task.h"


#define TAG "Main"

#define MOTOR0_DIRECTION_PIN GPIO_NUM_22
#define MOTOR0_STEP_PIN GPIO_NUM_21
#define VALVE_1_PIN GPIO_NUM_2
#define VALVE_2_PIN GPIO_NUM_16
#define LOAD_CELL_DOUT_PIN GPIO_NUM_13
#define LOAD_CELL_SCK_PIN GPIO_NUM_14

#define CMD_MAX_ARGS 8
#define CMD_MAX_LENGTH 256

#define LOAD_CELL_INIT_OFFSET 0  // TBD
#define LOAD_CELL_INIT_SCALE 1  // TBD

void app_main(void) {
    printf("\033[0;35m\n"
           "_|      _|    _|_|      _|_|_|  _|    _|  _|_|_|\n"
           "_|_|  _|_|  _|    _|  _|        _|  _|      _|  \n"
           "_|  _|  _|  _|_|_|_|  _|        _|_|        _|  \n"
           "_|      _|  _|    _|  _|        _|  _|      _|  \n"
           "_|      _|  _|    _|    _|_|_|  _|    _|  _|_|_|\n"
           "\n\033[0m");

    uint8_t valves_pins[NUMBER_OF_VALVES] = {VALVE_1_PIN, VALVE_2_PIN};

    valves_init(valves_pins);

    A4988_t stepper_motors[] = {
        {.direction_pin = MOTOR0_DIRECTION_PIN, .step_pin = MOTOR0_STEP_PIN}
    };
    stepper_motor_init(stepper_motors, sizeof(stepper_motors)/sizeof(stepper_motors[0]));

    load_cell_init(LOAD_CELL_DOUT_PIN, LOAD_CELL_SCK_PIN);
    load_cell_set_raw_offset(LOAD_CELL_INIT_OFFSET);
    load_cell_set_scale(LOAD_CELL_INIT_SCALE);

    esp_console_config_t console_config = {
        .max_cmdline_args = CMD_MAX_ARGS,
        .max_cmdline_length = CMD_MAX_LENGTH,
    };
    esp_console_init(&console_config);

    cmd_register_common();
    cmd_register_valves();
    cmd_register_load_cell();
    cmd_register_stepper_motor();

    cli_init(console_config.max_cmdline_length);
    cli_run();
}
