// Copyright 2023 PWr in Space, Krzysztof Gliwiński
#include "ble_gap_conf.h"

#define GAP_TAG "BLE_GAP"

bool ble_gap_init(ble_gap_t *gap_conf) {
  esp_err_t ret = ESP_OK;
  ret = esp_ble_gap_register_callback(gap_conf->event_handler_cb);
  if (ret != ESP_OK) {
    ESP_LOGE(GAP_TAG, "Failed to register GAP callback: %s",
             esp_err_to_name(ret));
    return ret;
  }

  return ret;
}
