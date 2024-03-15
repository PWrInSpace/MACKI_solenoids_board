// Copyright 2024 PWrInSpace, Kuba

#ifndef _load_cell_H_
#define _load_cell_H_

#include <stdbool.h>
#include "driver/gpio.h"

bool load_cell_init(gpio_num_t dout, gpio_num_t sck);

bool load_cell_read(size_t times, float *out);

bool load_cell_calibrate(float known_weight);

bool load_cell_tare(void);

void load_cell_set_raw_offset(int32_t raw_offset);

int32_t load_cell_get_raw_offset(void);

void load_cell_set_scale(float scale);

float load_cell_get_scale(void);


#endif
