// Copyright 2024 PWrInSpace, Kuba

#ifndef _CMD_H_
#define _CMD_H_

#include <stdint.h>

bool cmd_register_common(void);

bool cmd_register_valves(void);

bool cmd_register_load_cell(void);

bool cmd_register_stepper_motor(void);

#endif
