// Copyright 2024 PWrInSpace, Kuba

#ifndef _TENSOMETER_H_
#define _TENSOMETER_H_

#include <stdbool.h>
#include "driver/gpio.h"

void tensometer_init(gpio_num_t dout, gpio_num_t sck);

bool tensometer_read(float *out);

bool tensometer_calibrate(float known_weight);

bool tensometer_tare(void);


#endif