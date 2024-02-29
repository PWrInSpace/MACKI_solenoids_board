#include <memory.h>
#include "valves_task.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define COMMAND_QUEUE_SIZE NUMBER_OF_VALVES
#define DO_NOT_CLOSE 0

static struct {
    TaskHandle_t task_handle;
    SemaphoreHandle_t data_mutex;
} rtos;

static uint32_t valves_close_time[NUMBER_OF_VALVES];
static valve_data_t valves_data[NUMBER_OF_VALVES];

static bool _set_pin(uint8_t valve_index, valve_state_t state, uint32_t close_time) {
    if (valve_index >= NUMBER_OF_VALVES) {
        return false;
    }

    if (xSemaphoreTake(rtos.data_mutex, 10) != pdTRUE) {
        return false;
    }

    valve_data_t *valve = &valves_data[valve_index];
    if (valve->valve_status == VALVE_BUSSY) {
        xSemaphoreGive(rtos.data_mutex);
        return false;
    }

    valve->valve_state = state;
    gpio_set_level(valve->gpio_pin, valve->valve_state);

    if (close_time != DO_NOT_CLOSE) {
        valve->valve_status = VALVE_BUSSY;
        valves_close_time[valve_index] = pdTICKS_TO_MS(xTaskGetTickCount()) + close_time;
    }

    xSemaphoreGive(rtos.data_mutex);

    return true;
}

bool valve_open(uint8_t valve_index) {
    return _set_pin(valve_index, VALVE_OPEN, false);
}

bool valve_close(uint8_t valve_index) {
    return _set_pin(valve_index, VALVE_CLOSE, false);
}

bool valve_time_open(uint8_t valve_index, uint16_t time_ms) {
    if (time_ms == 0) {
        return false;
    }

    return _set_pin(valve_index, VALVE_OPEN, time_ms);
}

bool valve_get_data(uint8_t valve_index, valve_data_t *out) {
    if (xSemaphoreTake(rtos.data_mutex, 10) != pdTRUE) {
        return false;
    }

    memcpy(out, &valves_data[valve_index], sizeof(valves_data[valve_index]));

    xSemaphoreGive(rtos.data_mutex);
    return true;
}

static void _check_time_to_close(void) {
    if (xSemaphoreTake(rtos.data_mutex, 10) != pdTRUE) {
        return;
    }

    valve_data_t *valve = NULL;
    for (size_t i = 0; i < NUMBER_OF_VALVES; ++i) {
        valve = &valves_data[i];

        if (valve->valve_status == VALVE_BUSSY) {
            valve->time_to_close = valves_close_time[i] - pdTICKS_TO_MS(xTaskGetTickCount());

            if (valve->time_to_close <= 0) {
                valve->time_to_close = TIME_TO_CLOSE_NULL;
                valve->valve_status = VALVE_READY;
                valve->valve_state = VALVE_CLOSE;
                gpio_set_level(valve->gpio_pin, VALVE_CLOSE);
            }
        }
    }

    xSemaphoreGive(rtos.data_mutex);
}

static void _valves_task(void *arg) {
    while (true) {
        _check_time_to_close();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}


bool valves_init(uint8_t valves_pins[NUMBER_OF_VALVES]) {
    for (uint8_t i = 0; i < NUMBER_OF_VALVES; ++i) {
        valves_data[i].valve_state = VALVE_CLOSE;
        valves_data[i].valve_status = VALVE_READY;
        valves_data[i].time_to_close = TIME_TO_CLOSE_NULL;
        valves_data[i].gpio_pin = valves_pins[i];
        gpio_set_direction(valves_data[i].gpio_pin, GPIO_MODE_OUTPUT);
    }

    rtos.data_mutex = xSemaphoreCreateMutex();
    if (rtos.data_mutex == NULL) {
        goto deinit;
    }

    xTaskCreatePinnedToCore(
        _valves_task, "ValvesTask", 8192, NULL, 0, &rtos.task_handle, 1
    );
    if (rtos.task_handle == NULL) {
        goto deinit;
    }

    return true;

deinit:
    // TBD
    return false;
}