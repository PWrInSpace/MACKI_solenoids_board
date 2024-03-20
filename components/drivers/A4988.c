// Copyright 2024 PWrInSpace, Kuba

#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <rom/ets_sys.h>
#include "A4988.h"
#include "sdkconfig.h"

#define HIGH 1
#define LOW 0
// The datasheet says that the minimum time of the signal is 1us, but it doesn't work :C
// I found out by experiment that the minimum time for esp is 600 us.
#define KOWALSKI_CONST_DELAY_US 600
// The datasheet says it is 200ns, the minimum amount of time that we can stop is 1 us
#define DIRECTION_STEP_DELAY_US 1

bool A4988_init(A4988_t *dev) {
    esp_err_t err = ESP_OK;

    err |= gpio_set_direction(dev->direction_pin, GPIO_MODE_OUTPUT);
    err |= gpio_set_direction(dev->step_pin, GPIO_MODE_OUTPUT);
    err |= gpio_set_level(dev->direction_pin, LOW);
    err |= gpio_set_level(dev->step_pin, LOW);

    return err == ESP_OK;
}

bool A4988_move(A4988_t *dev, A4988_direction_t direction, size_t number_of_steps) {
    esp_err_t err = ESP_OK;
    err = gpio_set_level(dev->direction_pin, direction);
    if (err != ESP_OK) {
        return false;
    }

    ets_delay_us(DIRECTION_STEP_DELAY_US);
    for (size_t i = 0; i < number_of_steps; i++) {
        err |= gpio_set_level(dev->step_pin, HIGH);
        ets_delay_us(KOWALSKI_CONST_DELAY_US);
        err |= gpio_set_level(dev->step_pin, LOW);
        ets_delay_us(KOWALSKI_CONST_DELAY_US);

        if (err != ESP_OK) {
            return false;
        }
    }

    return true;
}
