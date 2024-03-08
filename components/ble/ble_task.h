// Copyright 2023 PWr in Space, Krzysztof Gliwiński

#pragma once

#include <string.h>

#include "ble_api.h"
#include "ble_gap_conf.h"
#include "ble_gatt_conf.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define BLE_DEVICE_NAME "MACKI"

void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t* param);

void gatt_profile_a_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if,
                                  esp_ble_gatts_cb_param_t* param);

void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if,
                         esp_ble_gatts_cb_param_t* param);

void ble_init_task(void* arg);
