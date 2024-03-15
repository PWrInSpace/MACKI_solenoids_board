// Copyright 2024 PWrInSpace, Kuba

#include "HX711.h"
#include "load_cell.h"
#include "esp_log.h"

#define TARE_NB_OF_SAMPLES 10
#define SCALE_NB_OF_SAMPLES 10
#define WAIT_MS 50

#define TAG "HX"

static struct {
    hx711_t hx;
    int32_t raw_offset;
    float scale;
} dev;

bool load_cell_init(gpio_num_t dout, gpio_num_t sck) {
    dev.hx.dout = dout;
    dev.hx.pd_sck = sck;
    dev.hx.gain = HX711_GAIN_A_128;
    dev.raw_offset = 0;
    dev.scale = 1;

    if (hx711_init(&dev.hx) != ESP_OK) {
        return false;
    }

    return true;
}

void load_cell_set_raw_offset(int32_t raw_offset) {
    dev.raw_offset = raw_offset;
}

int32_t load_cell_get_raw_offset(void) {
    return dev.raw_offset;
}

void load_cell_set_scale(float scale) {
    dev.scale = scale;
}

float load_cell_get_scale(void) {
    return dev.scale;
}

static bool load_cell_read_raw(size_t times, int32_t *out) {
    if (hx711_wait(&dev.hx, WAIT_MS) != ESP_OK) {
        return false;
    }

    int32_t data = 0;
    if (hx711_read_average(&dev.hx, times, &data) != ESP_OK) {
        return false;
    }
    *out = data;

    return true;
}

bool load_cell_read(size_t times, float *out) {
    int32_t raw = 0;
    if (load_cell_read_raw(times, &raw) == false) {
        return false;
    }
    *out = dev.scale * (raw - dev.raw_offset);

    return true;
}

bool load_cell_calibrate(float known_weight) {
    float measurement = 0;
    dev.scale = 1;  // Set scale to 1 to get only (raw value - offset)
    if (load_cell_read(SCALE_NB_OF_SAMPLES, &measurement) == false) {
        return false;
    }

    dev.scale = known_weight / measurement;

    return true;
}

bool load_cell_tare(void) {
    int32_t offset = 0;
    if (load_cell_read_raw(TARE_NB_OF_SAMPLES, &offset) == false) {
        return false;
    }

    dev.raw_offset = offset;

    return true;
}
