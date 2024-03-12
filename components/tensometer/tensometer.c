// Copyright 2024 PWrInSpace, Kuba

#include "HX711.h"
#include "esp_log.h"

#define READINGS_NB 1

#define TAG "HX"

void tensometer_init(gpio_num_t dout, gpio_num_t sck) {
    HX711_init(dout, sck, eGAIN_128);  // 128 gain for channel A
    // HX711_power_up();  // I thin it is not necessary
}

bool tensometer_read(float *out) {
    // if (HX711_is_ready() == false) {
    //     ESP_LOGE(TAG, "Hx is not ready :C");
    //     return false;
    // }

    *out = HX711_read();
    return true;
}

bool tensometer_calibrate(float known_weight) {
    float scale = 0;
    float reading = 0;

    if (tensometer_read(&reading) == false) {
        return false;
    }

    scale = known_weight / reading;

    HX711_set_scale(scale);

    return true;
}

bool tensometer_tare(void) {
    float offset = 0;
    if (tensometer_read(&offset) == false) {
        return false;
    }

    HX711_set_offset(offset);

    return true;
}
