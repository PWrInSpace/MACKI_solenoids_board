// Copyright 2024 PWrInSpace, Kuba

#ifndef _A49888_H_
#define _A49888_H_

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

typedef enum {
    COUNTER_CLOCKWISE = 0,
    CLOCKWISE,
} A4988_direction_t;

typedef struct {
    uint32_t step_pin;
    uint32_t direction_pin;
} A4988_t;

bool A4988_init(A4988_t *dev);

bool A4988_move(A4988_t *dev, A4988_direction_t direction, size_t number_of_steps);

#endif
