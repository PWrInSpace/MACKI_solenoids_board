// Copyright 2024 PWrInSpace, Kuba
#ifndef _VALVES_TASK_C
#define _VALVES_TASK_C

#include <stdbool.h>
#include <stdint.h>

#define NUMBER_OF_VALVES 2
#define TIME_TO_CLOSE_NULL -9999

#define VALVES_TASK_STACK_DEPTH CONFIG_VALVES_TASK_STACK_DEPTH
#define VALVES_TASK_PRIORITY CONFIG_VALVES_TASK_PRIORITY
#define VALVES_TASK_CPU_NUM CONFIG_VALVES_TASK_CPU

typedef enum {
    VALVE_CLOSE = 0,
    VALVE_OPEN,
} valve_state_t;

typedef enum {
    VALVE_READY = 1,
    VALVE_BUSY,
} valve_status_t;

typedef struct {
    valve_state_t valve_state;
    valve_status_t valve_status;
    int32_t time_to_close;
    uint8_t gpio_pin;
} valve_data_t;

bool valves_init(uint8_t valves_pins[NUMBER_OF_VALVES]);

bool valve_get_data(uint8_t valve_index, valve_data_t *out);

bool valve_open(uint8_t valve_index);

bool valve_time_open(uint8_t valve_index, uint16_t time_ms);

bool valve_close(uint8_t valve_index);

#endif
