// Copyright 2024 PWrInSpace, Kuba

#include "argtable3/argtable3.h"
#include "cli_task.h"
#include "load_cell.h"
#include "cmd.h"

static struct {
    struct arg_dbl *known_weight;
    struct arg_end *end;
} args_calibrate;


static int cmd_load_cell_tare(int argc, char **argv) {
    if (load_cell_tare() == false) {
        return LOAD_CELL_UNABLE_TO_TARE;
    }

    int32_t offset = load_cell_get_raw_offset();
    CLI_WRITE_G("load_cell tared, offset %d", offset);

    return 0;
}

static int cmd_load_cell_calibrate(int argc, char **argv) {
    int nerrors = arg_parse(argc, argv, (void **) &args_calibrate);
    if (nerrors != 0) {
        arg_print_errors(stderr, args_calibrate.end, argv[0]);
        return LOAD_CELL_ARG_PARSE;
    }

    if (load_cell_calibrate(args_calibrate.known_weight->dval[0]) == false) {
        return LOAD_CELL_UNABLE_TO_CALIBRAT;
    }

    float scale = load_cell_get_scale();
    int32_t offset = load_cell_get_raw_offset();
    CLI_WRITE_G("Load cell calibrated, scale %f, offset %d", scale, offset);

    return 0;
}

bool cmd_register_load_cell(void) {
    args_calibrate.known_weight = arg_dbl1(NULL, NULL, "Known weight", "known_weight");
    args_calibrate.end = arg_end(1);

    const esp_console_cmd_t open_cmd[] = {
        {.command = "hx_tare", .help = "tare load cell", .hint = NULL,
        .func = cmd_load_cell_tare},
        {.command = "hx_calib", .help = "calibrate load cell", .hint = NULL,
        .func = cmd_load_cell_calibrate, .argtable = &args_calibrate},
    };
    size_t number_of_commands = sizeof(open_cmd) / sizeof(open_cmd[0]);

    for (size_t i = 0; i < number_of_commands; ++i) {
        if (esp_console_cmd_register(&open_cmd[i]) != ESP_OK) {
            return false;
        }
    }

    return true;
}
