// Copyright 2024 PWrInSpace, Kuba

#ifndef _CMD_H_
#define _CMD_H_

#include <stdint.h>

typedef enum {
    LOAD_CELL_ARG_PARSE = 0,
    LOAD_CELL_UNABLE_TO_TARE,
    LOAD_CELL_UNABLE_TO_CALIBRAT,
    SMOTOR_ARG_PARSE,
    SMOTOR_ARG_MOTOR_NUMBER,
    SMOTOR_ARG_DIRECTION,
    SMOTOR_ARG_STEPS,
    SMOTOR_RUN,
    VALVES_ARG_PARSE,
    VALVES_ARG_VALVE_NUMBER,
} cmd_error_t;

bool cmd_register_common(void);

bool cmd_register_valves(void);

bool cmd_register_load_cell(void);

bool cmd_register_stepper_motor(void);

#endif
