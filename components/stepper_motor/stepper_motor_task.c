// Copyright 2024 PWrInSpace, Kuba

#include "stepper_motor_task.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <memory.h>
#include "esp_log.h"


typedef struct {
    TaskHandle_t task_handle;
    SemaphoreHandle_t data_mutex;
} stepper_motor_task_t;

typedef struct {
    A4988_t driver;
    stepper_motor_data_t data;
    stepper_motor_task_t task;
} stepper_motor_t;

static stepper_motor_t _motors[MAX_NUMBER_OF_MOTORS];
// Data cannot be overridden while task is running because motor is in busy state
static stepper_motor_move_t _move_cfg;
static portMUX_TYPE _mux = portMUX_INITIALIZER_UNLOCKED;

static bool _get_motor_pointer(uint8_t motor_number, stepper_motor_t **out) {
    if (motor_number >= MAX_NUMBER_OF_MOTORS) {
        return false;
    }

    *out = &_motors[motor_number];

    return true;
}

static bool _init_motor(stepper_motor_t *motor, A4988_t *driver) {
    if (motor == NULL) {
        return false;
    }

    memcpy(&motor->driver, driver, sizeof(motor->driver));
    if (A4988_init(driver) == false) {
        return false;
    }

    motor->data.state = SMOTOR_READY;
    motor->data.motion = SMOTOR_NO_MOTION;

    motor->task.data_mutex = xSemaphoreCreateMutex();
    if (motor->task.data_mutex == NULL) {
        return false;
    }

    return true;
}

bool stepper_motor_init(A4988_t *motor_drivers, size_t number_of_motors) {
    if (motor_drivers == NULL) {
        return false;
    }

    if (number_of_motors > MAX_NUMBER_OF_MOTORS) {
        return false;
    }

    stepper_motor_t *motor = NULL;
    for (size_t i = 0; i < number_of_motors; ++i) {
        if (_get_motor_pointer(i, &motor) == false) {
            return false;
        }
        if (_init_motor(motor, motor_drivers + i) == false) {
            return false;
        }
    }

    return true;
}

static bool _motor_is_ready(stepper_motor_t *motor) {
    bool is_ready = false;

    if (xSemaphoreTake(motor->task.data_mutex, 10) != pdTRUE) {
        return false;
    }

    is_ready = motor->data.state == SMOTOR_READY;

    xSemaphoreGive(motor->task.data_mutex);

    return is_ready;
}

static bool _motor_set_data(stepper_motor_t *motor, stepper_motor_state_t state,
                            stepper_motor_motion_t motion) {
    if (xSemaphoreTake(motor->task.data_mutex, 10) != pdTRUE) {
        return false;
    }

    motor->data.state = state;
    motor->data.motion = motion;

    xSemaphoreGive(motor->task.data_mutex);

    return true;
}

static stepper_motor_motion_t _A4988_driection_to_motion(A4988_direction_t direction) {
    if (direction == CLOCKWISE) {
        return SMOTOR_CLOCKWISE;
    }

    return SMOTOR_COUNTER_CLOCKWISE;
}

static void _stepper_motor_task(void *arg) {
    stepper_motor_t *motor = NULL;

    if (_get_motor_pointer(_move_cfg.motor_number, &motor) == false) {
        vTaskDelete(NULL);
    }

    stepper_motor_motion_t motion = _A4988_driection_to_motion(_move_cfg.direction);
    if (_motor_set_data(motor, SMOTOR_BUSY, motion) == false) {
        vTaskDelete(NULL);
    }

    taskENTER_CRITICAL(&_mux);
    (void) A4988_move(&motor->driver, _move_cfg.direction, _move_cfg.steps);
    taskEXIT_CRITICAL(&_mux);

    (void)_motor_set_data(motor, SMOTOR_READY, SMOTOR_NO_MOTION);

    vTaskDelete(NULL);
}

bool stepper_motor_run_on_task(stepper_motor_move_t *move_cfg) {
    if (move_cfg == NULL) {
        return false;
    }

    stepper_motor_t *motor;
    if (_get_motor_pointer(move_cfg->motor_number, &motor) == false) {
        return false;
    }

    if (_motor_is_ready(motor) == false) {
        return false;
    }

    _move_cfg = *move_cfg;

    xTaskCreatePinnedToCore(
        _stepper_motor_task,
        "motor task",
        SMOTOR_TASK_STACK_DEPTH,
        NULL,
        SMOTOR_TASK_PRIORITY,
        &(motor->task.task_handle),
        SMOTOR_TASK_CPU_NUM);
    if (motor->task.task_handle == NULL) {
        return false;
    }

    return true;
}

bool stepper_motor_get_data(uint8_t motor_number, stepper_motor_data_t *out) {
    stepper_motor_t *motor = NULL;
    if (_get_motor_pointer(motor_number, &motor) == false) {
        return false;
    }

    if (xSemaphoreTake(motor->task.data_mutex, 10) != pdTRUE) {
        return false;
    }

    *out = motor->data;

    xSemaphoreGive(motor->task.data_mutex);
    return true;
}
