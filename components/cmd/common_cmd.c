// Copyright 2024 PWrInSpace, Kuba
#include "cli_task.h"
#include "cmd.h"
#include "load_cell.h"
#include "stepper_motor_task.h"
#include "valves_task.h"

static int cmd_read_data(int argc, char **argv) {
    char buffer[255];
    size_t wsize = 0;

    valve_data_t valve_data;
    for (size_t i = 0; i < NUMBER_OF_VALVES; ++i) {
        valve_get_data(i, &valve_data);
        wsize +=
            snprintf(buffer + wsize, sizeof(buffer) - wsize, "VALVE%d,%d,%d,%d;", i,
                     valve_data.valve_state, valve_data.valve_status, valve_data.time_to_close);
    }

    float hx_reading = 0;
    (void)load_cell_read(1, &hx_reading);
    wsize += snprintf(buffer + wsize, sizeof(buffer) - wsize, "HX,%f;", hx_reading);

    stepper_motor_data_t motor_data;
    for (size_t i = 0; i < MAX_NUMBER_OF_MOTORS; ++i) {
        (void) stepper_motor_get_data(i, &motor_data);
        wsize += snprintf(buffer + wsize, sizeof(buffer) - wsize, "MOTOR%d,%d,%d;", i,
                          motor_data.state, motor_data.motion);
    }

    CLI_WRITE_G("%s", buffer);

    return 0;
}

bool cmd_register_common(void) {
    const esp_console_cmd_t open_cmd[] = {
        {.command = "read_data", .help = "read data", .hint = NULL, .func = cmd_read_data},
    };
    size_t number_of_commands = sizeof(open_cmd) / sizeof(open_cmd[0]);

    for (size_t i = 0; i < number_of_commands; ++i) {
        if (esp_console_cmd_register(&open_cmd[i]) != ESP_OK) {
            return false;
        }
    }

    return true;
}
