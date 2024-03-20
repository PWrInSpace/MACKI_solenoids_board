// Copyright 2024 PWrInSpace, Kuba

#include "argtable3/argtable3.h"
#include "cli_task.h"
#include "stepper_motor_task.h"
#include "cmd.h"

static struct {
    struct arg_int *motor_number;
    struct arg_int *steps;
    struct arg_int *direction;
    struct arg_end *end;
} args_move;


static int _parse_move(int argc, char**argv) {
    int nerrors = arg_parse(argc, argv, (void **) &args_move);
    if (nerrors != 0) {
        arg_print_errors(stderr, args_move.end, argv[0]);
        return SMOTOR_ARG_PARSE;
    }

    if (args_move.motor_number->ival[0] >= MAX_NUMBER_OF_MOTORS) {
        return SMOTOR_ARG_MOTOR_NUMBER;
    }

    if (args_move.direction->ival[0] != CLOCKWISE &&
        args_move.direction->ival[0] != COUNTER_CLOCKWISE) {
        return SMOTOR_ARG_DIRECTION;
    }

    if (args_move.steps->ival[0] < 0) {
        return SMOTOR_ARG_STEPS;
    }

    return 0;
}

static int cmd_stepper_motor_move(int argc, char **argv) {
    int ret = _parse_move(argc, argv);
    if (ret != 0) {
        return ret;
    }

    stepper_motor_move_t move_cfg = {
        .motor_number = args_move.motor_number->ival[0],
        .direction = args_move.direction->ival[0],
        .steps = args_move.steps->ival[0],
    };

    if (stepper_motor_run_on_task(&move_cfg) == false) {
        return SMOTOR_RUN;
    }

    CLI_WRITE_G("Motor %d is running, steps %d",
                move_cfg.motor_number, move_cfg.steps);

    return 0;
}

bool cmd_register_stepper_motor(void) {
    args_move.motor_number = arg_int1(NULL, NULL, "motor_number", "Motor number");
    args_move.direction = arg_int1(NULL, NULL, "direciton",
                                   "Motor direction [0 - Counter clockwise, 1 - Clockwise]");
    args_move.steps = arg_int1(NULL, NULL, "steps", "Number of steps");
    args_move.end = arg_end(3);

    const esp_console_cmd_t open_cmd[] = {
        {.command = "smotor_move", .help = "move stepper motor", .hint = NULL,
        .func = cmd_stepper_motor_move, .argtable = &args_move},
    };

    size_t number_of_commands = sizeof(open_cmd) / sizeof(open_cmd[0]);
    for (size_t i = 0; i < number_of_commands; ++i) {
        if (esp_console_cmd_register(&open_cmd[i]) != ESP_OK) {
            return false;
        }
    }

    return true;
}
