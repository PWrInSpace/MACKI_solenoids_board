// Copyright 2023 PWr in Space, Krzysztof Gliwiński
#pragma once

#include <stdint.h>

#include "ble_gap_conf.h"
#include "ble_gatt_conf.h"
#include "esp_bt.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "nvs_flash.h"
#include "sdkconfig.h"

/*!
 * \file ble_api.h
 * \brief BLE API - contains all BLE functions and structures
 * ass well as attributes of GATT - unified in one structure
 */

/* LSB
 <-------------------------------------------------------------------------------->
 MSB */
// first uuid, 16bit, [12],[13] is the value
// second uuid, 32bit, [12], [13], [14], [15] is the value
#define BLE_UUID_CONFIG_DEFAULT()                                           \
  {                                                                         \
    0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, \
        0xEE, 0x00, 0x00, 0x00, 0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00,   \
        0x80, 0x00, 0x10, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00,               \
  }

// slave connection min interval, Time = min_interval * 1.25 msec
// slave connection max interval, Time = max_interval * 1.25 msec
// TEST_MANUFACTURER_DATA_LEN,
// &test_manufacturer[0],
#define BLE_ADV_DATA_CONFIG_DEFAULT()                                          \
  {                                                                            \
    .set_scan_rsp = false, .include_name = true, .include_txpower = false,     \
    .min_interval = 0x0006, .max_interval = 0x0010, .appearance = 0x00,        \
    .manufacturer_len = 0, .p_manufacturer_data = NULL, .service_data_len = 0, \
    .p_service_data = NULL, .service_uuid_len = sizeof(adv_service_uuid128),   \
    .p_service_uuid = adv_service_uuid128,                                     \
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),      \
  }

// TEST_MANUFACTURER_DATA_LEN,
// &test_manufacturer[0],
#define BLE_SCAN_RSP_DATA_CONFIG_DEFAULT()                                  \
  {                                                                         \
    .set_scan_rsp = true, .include_name = true, .include_txpower = true,    \
    .appearance = 0x00, .manufacturer_len = 0, .p_manufacturer_data = NULL, \
    .service_data_len = 0, .p_service_data = NULL,                          \
    .service_uuid_len = sizeof(adv_service_uuid128),                        \
    .p_service_uuid = adv_service_uuid128,                                  \
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),   \
  }

#define BLE_ADV_PARAMS_CONFIG_DEFAULT()                                 \
  {                                                                     \
    .adv_int_min = 0x20, .adv_int_max = 0x40, .adv_type = ADV_TYPE_IND, \
    .own_addr_type = BLE_ADDR_TYPE_PUBLIC, .channel_map = ADV_CHNL_ALL, \
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,             \
  }

/*!
 * \brief BLE status enum
 */
typedef enum { BLE_OK = 0, BLE_HARDWARE_INIT_ERR, BLE_ERR } ble_err_t;

/*!
 * \brief Main BLE configuration struct.
 * Contains gatts and gap configuration
 */
typedef struct {
  ble_gatts_t *gatt_config;
  ble_gap_t *gap_config;
  esp_bt_controller_config_t bt_cfg;
} ble_config_t;

/*!
 * \brief Initiates all necessary esp hardware related to bluetooth
 */
ble_err_t ble_esp_hardware_init(ble_config_t *ble);

/*!
 * \brief Initiates the whole bluetooth stack, as well as gap ble_gatts_t and
 * ble_gap_t configuration
 */
ble_err_t ble_init(ble_config_t *ble);

/*!
 * \brief Converts ble_err_t to string
 * \param[in] err - error to convert
 * \return string representation of error
 */
const char *ble_err_to_string(ble_err_t err);
