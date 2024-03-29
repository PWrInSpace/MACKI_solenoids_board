// Copyright 2024 PWrInSpace, Kuba

#include "valves_task.h"
#include "argtable3/argtable3.h"
#include "cli_task.h"
#include "cmd.h"

static struct {
    struct arg_int *valve_number;
    struct arg_end *end;
} args_open_close;

static struct {
    struct arg_int *valve_number;
    struct arg_int *time_ms;
    struct arg_end *end;
} args_time_open;

static int parse_open_close(int argc, char**argv) {
    int nerrors = arg_parse(argc, argv, (void **) &args_open_close);
    if (nerrors != 0) {
        arg_print_errors(stderr, args_open_close.end, argv[0]);
        return 1;
    }

    if (args_open_close.valve_number->ival[0] >= NUMBER_OF_VALVES) {
        return 2;
    }

    return 0;
}

static int cmd_valve_open(int argc, char **argv) {
    int ret = parse_open_close(argc, argv);
    if (ret != 0) {
        return ret;
    }

    valve_open(args_open_close.valve_number->ival[0]);
    CLI_WRITE_G("Valve open");

    return 0;
}

static int cmd_valve_close(int argc, char **argv) {
    int ret = parse_open_close(argc, argv);
    if (ret != 0) {
        return ret;
    }

    valve_close(args_open_close.valve_number->ival[0]);
    CLI_WRITE_G("Valve close");

    return 0;
}

static int cmd_valve_time_open(int argc, char **argv) {
    int nerrors = arg_parse(argc, argv, (void **) &args_time_open);
    if (nerrors != 0) {
        arg_print_errors(stderr, args_time_open.end, argv[0]);
        return 1;
    }

    if (args_time_open.valve_number->ival[0] > (NUMBER_OF_VALVES - 1)) {
        return 1;
    }

    valve_time_open(args_time_open.valve_number->ival[0], args_time_open.time_ms->ival[0]);
    CLI_WRITE_G("Valve open for %d", args_time_open.time_ms->ival[0]);

    return 0;
}

bool cmd_register_valves(void) {
    args_open_close.valve_number = arg_int1(NULL, NULL, "valve_number", "Valve number");
    args_open_close.end = arg_end(1);

    args_time_open.valve_number = arg_int1(NULL, NULL, "valve_number", "Valve number");
    args_time_open.time_ms = arg_int1(NULL, NULL, "open_time", "Open time ms");
    args_time_open.end = arg_end(2);


    const esp_console_cmd_t open_cmd[] = {
        {.command = "valve_open", .help = "open valve", .hint = NULL,
        .func = cmd_valve_open, .argtable = &args_open_close},
        {.command = "valve_close", .help = "close valve", .hint = NULL,
        .func = cmd_valve_close, .argtable = &args_open_close},
        {.command = "valve_time_open", .help = "time open valve", .hint = NULL,
        .func = cmd_valve_time_open, .argtable = &args_time_open},
    };
    size_t number_of_commands = sizeof(open_cmd) / sizeof(open_cmd[0]);

    for (size_t i = 0; i < number_of_commands; ++i) {
        if (esp_console_cmd_register(&open_cmd[i]) != ESP_OK) {
            return false;
        }
    }

    return true;
}
