// Copyright 2023 PWr in Space, Krzysztof Gliwiński
#include "ble_gatt_conf.h"

#define BLE_GATT_TAG "BLE_GATT_API"

esp_err_t ble_gatt_init(ble_gatts_t *gatts_conf) {
  esp_err_t ret = ESP_OK;
  if (gatts_conf == NULL) {
    ESP_LOGE(BLE_GATT_TAG, "GATT config is NULL");
    return ESP_ERR_INVALID_ARG;
  }

  if (gatts_conf->profiles_num == 0 ||
      gatts_conf->profiles_num > GATTS_PROFILES_NUM_MAX) {
    ESP_LOGE(BLE_GATT_TAG, "No GATT profiles or too many profiles: %d",
             gatts_conf->profiles_num);
    return ESP_ERR_INVALID_ARG;
  }

  ret = esp_ble_gatts_register_callback(gatts_conf->event_handler_cb);
  if (ret != ESP_OK) {
    ESP_LOGE(BLE_GATT_TAG, "Failed to register GATT callback: %s",
             esp_err_to_name(ret));
    return ret;
  }

  for (uint16_t i = 0; i < gatts_conf->profiles_num; ++i) {
    ESP_LOGI(BLE_GATT_TAG, "Registering app: %d", i);
    ret = esp_ble_gatts_app_register(i);
    if (ret != ESP_OK) {
      ESP_LOGE(BLE_GATT_TAG, "Failed to register app: %s, %s", __func__,
               esp_err_to_name(ret));
      return ret;
    }
  }

  ret = esp_ble_gatt_set_local_mtu(500);
  if (ret != ESP_OK) {
    ESP_LOGE(BLE_GATT_TAG, "Failed to set MTU: %s", esp_err_to_name(ret));
    return ret;
  }

  return ret;
}
