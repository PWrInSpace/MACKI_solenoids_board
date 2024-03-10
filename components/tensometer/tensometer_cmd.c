// Copyright 2024 PWrInSpace, Kuba

#include "argtable3/argtable3.h"
#include "cli_task.h"
#include "tensometer.h"

static struct {
    struct arg_dbl *known_weight;
    struct arg_end *end;
} args_calibrate;


static int cmd_tensometer_tare(int argc, char **argv) {
    if (tensometer_tare() == false) {
        CLI_WRITE_E("Unable to tare tensometer");
        return 1;
    }

    CLI_WRITE_G("Tensometer tared");

    return 0;
}

static int cmd_tensometer_calibrate(int argc, char **argv) {
    int nerrors = arg_parse(argc, argv, (void **) &args_calibrate);
    if (nerrors != 0) {
        arg_print_errors(stderr, args_calibrate.end, argv[0]);
        return 1;
    }

    if (tensometer_calibrate(args_calibrate.known_weight->dval[0]) == false) {
        CLI_WRITE_E("Unable to calibrate tensometer");
        return 1;
    }

    CLI_WRITE_G("Tensometer calibrated");

    return 0;
}

bool cmd_register_tensometer(void) {
    args_calibrate.known_weight = arg_dbl1(NULL, NULL, "Known weight", "known_weight");
    args_calibrate.end = arg_end(1);

    const esp_console_cmd_t open_cmd[] = {
        {.command = "hx_tare", .help = "tare tensomter", .hint = NULL,
        .func = cmd_tensometer_tare},
        {.command = "hx_calib", .help = "calibrate tensometer", .hint = NULL,
        .func = cmd_tensometer_calibrate, .argtable = &args_calibrate},
    };
    size_t number_of_commands = sizeof(open_cmd) / sizeof(open_cmd[0]);

    for (size_t i = 0; i < number_of_commands; ++i) {
        if (esp_console_cmd_register(&open_cmd[i]) != ESP_OK) {
            return false;
        }
    }

    return true;
}