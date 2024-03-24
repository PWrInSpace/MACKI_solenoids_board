// Copyright 2023 PWr in Space, Krzysztof Gliwiński

#pragma once

#include <string.h>

#include "ble_api.h"
#include "ble_gap_conf.h"
#include "ble_gatt_conf.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_console.h"

#define BLE_DEVICE_NAME "MACKI"

typedef struct {
    uint8_t* prepare_buf;
    int prepare_len;
} prepare_type_env_t;

/*!
 * \brief GAP event handler. Mainly responsible for handling advertisement and connection events in
 * the GAP layer.
 */
void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t* param);

/*!
 * \brief GATT profile console handler. Handles events related to CONSOLE GATT profile.
 */
void gatt_profile_console_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if,
                                        esp_ble_gatts_cb_param_t* param);

/*!
 * \brief GATT event handler. Mainly responsible for handling GATT events.
 * Calls certain gatt profile event handlers.
 */
void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if,
                         esp_ble_gatts_cb_param_t* param);

void console_exec_write_event_env(prepare_type_env_t* prepare_write_env,
                                  esp_ble_gatts_cb_param_t* param);

void console_prepare_write_event_env(esp_gatt_if_t gatts_if, prepare_type_env_t* prepare_write_env,
                                     esp_ble_gatts_cb_param_t* param);

void parse_cli_command(char* command);

/*!
 * \brief Initialize BLE stack and start advertising. Can be called as RTOS task if necessary - not
 * required.
 * \param arg - pointer to the argument passed to the task - not used, can be NULL
 */
void ble_init_task(void* arg);
