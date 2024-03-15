// Copyright 2024 PWrInSpace, Kuba
#include "cli_task.h"
#include "valves_task.h"
#include "load_cell.h"
#include "cmd.h"


static int cmd_read_data(int argc, char **argv) {
    char buffer[255];
    size_t wsize = 0;
    valve_data_t data;

    for (size_t i = 0; i < NUMBER_OF_VALVES; ++i) {
        valve_get_data(i, &data);
        wsize += snprintf(buffer + wsize, sizeof(buffer) - wsize, "VALVE%d,%d,%d,%d;",
                 i, data.valve_state, data.valve_status, data.time_to_close);
    }

    float hx_reading = 0;
    (void) load_cell_read(1, &hx_reading);
    wsize += snprintf(buffer + wsize, sizeof(buffer) - wsize, "HX,%f;", hx_reading);

    CLI_WRITE_G("%s", buffer);

    return 0;
}

bool cmd_register_common(void) {
    const esp_console_cmd_t open_cmd[] = {
        {.command = "read_data", .help = "read data", .hint = NULL,
        .func = cmd_read_data},
    };
    size_t number_of_commands = sizeof(open_cmd) / sizeof(open_cmd[0]);

    for (size_t i = 0; i < number_of_commands; ++i) {
        if (esp_console_cmd_register(&open_cmd[i]) != ESP_OK) {
            return false;
        }
    }

    return true;
}
