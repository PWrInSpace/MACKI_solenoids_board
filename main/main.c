// Copyright 2024 PWrInSpace, Kuba
#include <stdint.h>

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hello_world.h"

void app_main(void) {
    gpio_set_direction(GPIO_NUM_2, GPIO_MODE_OUTPUT);

    uint32_t gpio_level = 1;
    while (1) {
        hello_world();

        gpio_set_level(GPIO_NUM_2, gpio_level);
        gpio_level = !gpio_level;

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
