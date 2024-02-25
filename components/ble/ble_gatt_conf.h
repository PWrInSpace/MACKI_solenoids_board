// Copyright 2023 PWr in Space, Krzysztof Gliwiński
#pragma once

#include <stdint.h>

#include "esp_err.h"
#include "esp_gatt_common_api.h"
#include "esp_gatt_defs.h"
#include "esp_gatts_api.h"
#include "esp_log.h"
#include "sdkconfig.h"

/*!
 * \file ble_gatt_conf.h
 * \brief BLE GATT configuration structure
 */

/*!
 * \brief Maximum number of GATT profiles
 */
#define GATTS_PROFILES_NUM_MAX 16

/*!
 * \brief Maximum number of GATT attributes in db
 * \note Limited for ESP-IDF for ESP32 to 16, for ESP32-S3 is 256 (due to DRAM limitations)
 */
#if CONFIG_IDF_TARGET_ESP32
#define GATTS_ATTRIBUTES_DB_MAX_NUM 16
#elif CONFIG_IDF_TARGET_ESP32S3
#define GATTS_ATTRIBUTES_DB_MAX_NUM 256
#else  // default value
#define GATTS_ATTRIBUTES_DB_MAX_NUM 16
#endif

/*!
 * \brief Handle GATT events
 * \param[in] event GATT event
 * \param[in] param GATT event parameters
 */
typedef void (*ble_gatts_event_handler)(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if,
                                        esp_ble_gatts_cb_param_t *param);

/*!
 * \brief BLE GATT configuration structure
 */
typedef struct {
    //   esp_gatts_attr_db_t *gatt_db;
    esp_gatts_cb_t gatts_cb;
    uint16_t gatts_if;
    uint16_t conn_id;
    uint16_t service_handle;
    esp_gatt_srvc_id_t service_id;
    uint16_t char_handle;
    esp_bt_uuid_t char_uuid;
    esp_gatt_perm_t perm;
    esp_gatt_char_prop_t property;
    uint16_t descr_handle;
    esp_bt_uuid_t descr_uuid;
    esp_gatts_attr_db_t gatt_db[GATTS_ATTRIBUTES_DB_MAX_NUM];
} ble_gatts_profile_t;

/*!
 * \brief Main gatts configuration structure
 * \param[in] profiles_num Number of profiles
 * \param[in] profiles Array of profiles
 * \param[in] event_handler_cb Event handler callback
 */
typedef struct {
    uint16_t profiles_num;
    ble_gatts_event_handler event_handler_cb;
    ble_gatts_profile_t profiles[GATTS_PROFILES_NUM_MAX];
} ble_gatts_t;

/*!
 * \brief Initialize the BLE GATT module
 * \param[in] gatts_conf GATT configuration structure
 */
esp_err_t ble_gatt_init(ble_gatts_t *gatts_conf);

/*!
 * \brief Register the BLE GATT module
 * \param[in] gatts_conf GATT configuration structure
 */
esp_err_t ble_gatt_register_event(ble_gatts_t *gatts_conf);
