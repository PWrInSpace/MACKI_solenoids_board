// Copyright 2024 PWrInSpace, Kuba

#ifndef _STEPPER_MOTOR_TASK_H_
#define _STEPPER_MOTOR_TASK_H_

#include <stdbool.h>
#include "A4988.h"

#define SMOTOR_TASK_STACK_DEPTH CONFIG_STEPPER_MOTOR_TASK_STACK_DEPTH
#define SMOTOR_TASK_PRIORITY CONFIG_STEPPER_MOTOR_TASK_PRIORITY
#define SMOTOR_TASK_CPU_NUM CONFIG_STEPPER_MOTOR_TASK_CPU

#define MAX_NUMBER_OF_MOTORS 1

typedef enum {
    SMOTOR_READY = 1,
    SMOTOR_BUSY,
} stepper_motor_state_t;

typedef enum {
    SMOTOR_NO_MOTION = 0,
    SMOTOR_COUNTER_CLOCKWISE,
    SMOTOR_CLOCKWISE,
} stepper_motor_motion_t;

typedef struct {
    stepper_motor_state_t state;
    stepper_motor_motion_t motion;
} stepper_motor_data_t;

typedef struct {
    uint8_t motor_number;
    size_t steps;
    A4988_direction_t direction;
} stepper_motor_move_t;


bool stepper_motor_init(A4988_t *drivers, size_t number_of_motors);

bool stepper_motor_run_on_task(stepper_motor_move_t *move_cfg);

bool stepper_motor_get_data(uint8_t motor_number, stepper_motor_data_t *out);


#endif
