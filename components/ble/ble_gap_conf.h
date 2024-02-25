// Copyright 2023 PWr in Space, Krzysztof Gliwiński
#pragma once

#include <stdint.h>

#include "esp_gap_ble_api.h"
#include "esp_log.h"

/*!
 * \file ble_gap_conf.h
 * \brief API for configurating the BLE GAP module
 */

/*!
 * \brief GAP configuration structure
 * Used as a central device, only broadcaster, peripheral or both can be set
 */
typedef enum {
  BLE_GAP_BROADCASTER = 0x01b,
  BLE_GAP_CENTRAL = 0x10b,
  BLE_GAP_BROADCASTER_CENTRAL = 0x11b
} ble_gap_conf_type_t;

/*!
 * \brief Handle GAP events
 * \param[in] event GAP event
 * \param[in] param GAP event parameters
 */
typedef void (*ble_gap_event_handler)(esp_gap_ble_cb_event_t event,
                                      esp_ble_gap_cb_param_t *param);

typedef struct {
  ble_gap_conf_type_t conf_type;
  esp_ble_adv_params_t adv_params;
  esp_ble_adv_data_t adv_data;
  esp_ble_adv_data_t scan_rsp_data;
  ble_gap_event_handler event_handler_cb;
} ble_gap_t;

/*!
 * \brief Initialize the BLE GAP module
 * \param[in] gap_conf GAP configuration structure
 */
bool ble_gap_init(ble_gap_t *gap_conf);
