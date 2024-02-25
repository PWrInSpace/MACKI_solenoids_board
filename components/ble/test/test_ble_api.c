// Copyright 2023 PWr in Space, Krzysztof Gliwiński

#include "ble_api.h"
#include "string.h"
#include "unity.h"

#define BLE_TEST_TAG "BLE_UNIT_TEST"
#define TEST_DEVICE_NAME "ROSALIA_UNIT_TEST"

/* The max length of characteristic value. When the GATT client performs a write
 * or prepare write operation, the data length must be less than
 * GATTS_DEMO_CHAR_VAL_LEN_MAX.
 */
#define GATTS_DEMO_CHAR_VAL_LEN_MAX 500
#define PREPARE_BUF_MAX_SIZE 1024
#define CHAR_DECLARATION_SIZE (sizeof(uint8_t))

#define ADV_CONFIG_FLAG (1 << 0)
#define SCAN_RSP_CONFIG_FLAG (1 << 1)
#define SVC_INST_ID 0
#define SVC_INST_ID2 1

/* Service */
const uint16_t GATTS_SERVICE_UUID_TEST = 0x00FE;
const uint16_t GATTS_CHAR_UUID_TEST_A = 0xFF01;
const uint16_t GATTS_CHAR_UUID_TEST_B = 0xFF02;
const uint16_t GATTS_CHAR_UUID_TEST_C = 0xFF03;

const uint16_t GATTS_SERVICE_UUID_TEST2 = 0x00FF;
const uint16_t GATTS_CHAR_UUID_TEST_A2 = 0xFF04;
const uint16_t GATTS_CHAR_UUID_TEST_B2 = 0xFF05;
const uint16_t GATTS_CHAR_UUID_TEST_C2 = 0xFF06;

const uint16_t primary_service_uuid = ESP_GATT_UUID_PRI_SERVICE;
const uint16_t secondary_service_uuid = ESP_GATT_UUID_PRI_SERVICE;
const uint16_t character_declaration_uuid = ESP_GATT_UUID_CHAR_DECLARE;
const uint16_t character_client_config_uuid = ESP_GATT_UUID_CHAR_CLIENT_CONFIG;
const uint8_t char_prop_read = ESP_GATT_CHAR_PROP_BIT_READ;
const uint8_t char_prop_write = ESP_GATT_CHAR_PROP_BIT_WRITE;
const uint8_t char_prop_read_write_notify = ESP_GATT_CHAR_PROP_BIT_WRITE |
                                            ESP_GATT_CHAR_PROP_BIT_READ |
                                            ESP_GATT_CHAR_PROP_BIT_NOTIFY;
const uint8_t heart_measurement_ccc[2] = {0x00, 0x00};
const uint8_t char_value[4] = {0x11, 0x22, 0x33, 0x44};

typedef enum {
  PROFILE_A,
  PROFILE_B,
  PROFILE_TOTAL_NUM
} ble_gatt_profile_names_t;

typedef enum {
  IDX_SVC,
  IDX_CHAR_A,
  IDX_CHAR_VAL_A,
  IDX_CHAR_CFG_A,
  IDX_CHAR_B,
  IDX_CHAR_VAL_B,
  IDX_CHAR_C,
  IDX_CHAR_VAL_C,
  HRS_IDX_NB,
} ble_gatt_database_profile_idx;

typedef struct {
  uint8_t *prepare_buf;
  int prepare_len;
} prepare_type_env_t;

void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if,
                         esp_ble_gatts_cb_param_t *param);

void gap_event_handler(esp_gap_ble_cb_event_t event,
                       esp_ble_gap_cb_param_t *param);

void gatt_profile_a_event_handler(esp_gatts_cb_event_t event,
                                  esp_gatt_if_t gatts_if,
                                  esp_ble_gatts_cb_param_t *param);

void gatt_profile_b_event_handler(esp_gatts_cb_event_t event,
                                  esp_gatt_if_t gatts_if,
                                  esp_ble_gatts_cb_param_t *param);

void example_prepare_write_event_env(esp_gatt_if_t gatts_if,
                                     prepare_type_env_t *prepare_write_env,
                                     esp_ble_gatts_cb_param_t *param);

void example_exec_write_event_env(prepare_type_env_t *prepare_write_env,
                                  esp_ble_gatts_cb_param_t *param);

uint8_t adv_config_done = 0;
#define adv_config_flag (1 << 0)
#define scan_rsp_config_flag (1 << 1)

uint8_t adv_service_uuid128[32] = BLE_UUID_CONFIG_DEFAULT();
uint16_t profile_a_handle_table[HRS_IDX_NB];
uint16_t profile_b_handle_table[HRS_IDX_NB];
prepare_type_env_t prepare_write_env;

const esp_gatts_attr_db_t gatt_profile_a_db[HRS_IDX_NB] = {
    // Service Declaration
    [IDX_SVC] = {{ESP_GATT_AUTO_RSP},
                 {ESP_UUID_LEN_16, (uint8_t *)&primary_service_uuid,
                  ESP_GATT_PERM_READ, sizeof(uint16_t),
                  sizeof(GATTS_SERVICE_UUID_TEST),
                  (uint8_t *)&GATTS_SERVICE_UUID_TEST}},

    /* Characteristic Declaration */
    [IDX_CHAR_A] = {{ESP_GATT_AUTO_RSP},
                    {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid,
                     ESP_GATT_PERM_READ, CHAR_DECLARATION_SIZE,
                     CHAR_DECLARATION_SIZE,
                     (uint8_t *)&char_prop_read_write_notify}},

    /* Characteristic Value */
    [IDX_CHAR_VAL_A] = {{ESP_GATT_AUTO_RSP},
                        {ESP_UUID_LEN_16, (uint8_t *)&GATTS_CHAR_UUID_TEST_A,
                         ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
                         GATTS_DEMO_CHAR_VAL_LEN_MAX, sizeof(char_value),
                         (uint8_t *)char_value}},

    /* Client Characteristic Configuration Descriptor */
    [IDX_CHAR_CFG_A] = {{ESP_GATT_AUTO_RSP},
                        {ESP_UUID_LEN_16,
                         (uint8_t *)&character_client_config_uuid,
                         ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
                         sizeof(uint16_t), sizeof(heart_measurement_ccc),
                         (uint8_t *)heart_measurement_ccc}},

    /* Characteristic Declaration */
    [IDX_CHAR_B] = {{ESP_GATT_AUTO_RSP},
                    {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid,
                     ESP_GATT_PERM_READ, CHAR_DECLARATION_SIZE,
                     CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read}},

    /* Characteristic Value */
    [IDX_CHAR_VAL_B] = {{ESP_GATT_AUTO_RSP},
                        {ESP_UUID_LEN_16, (uint8_t *)&GATTS_CHAR_UUID_TEST_B2,
                         ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
                         GATTS_DEMO_CHAR_VAL_LEN_MAX, sizeof(char_value),
                         (uint8_t *)char_value}},

    /* Characteristic Declaration */
    [IDX_CHAR_C] = {{ESP_GATT_AUTO_RSP},
                    {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid,
                     ESP_GATT_PERM_READ, CHAR_DECLARATION_SIZE,
                     CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_write}},

    /* Characteristic Value */
    [IDX_CHAR_VAL_C] = {{ESP_GATT_AUTO_RSP},
                        {ESP_UUID_LEN_16, (uint8_t *)&GATTS_CHAR_UUID_TEST_C2,
                         ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
                         GATTS_DEMO_CHAR_VAL_LEN_MAX, sizeof(char_value),
                         (uint8_t *)char_value}},
};

const esp_gatts_attr_db_t gatt_profile_b_db[HRS_IDX_NB] = {
    // Service Declaration
    [IDX_SVC] = {{ESP_GATT_AUTO_RSP},
                 {ESP_UUID_LEN_16, (uint8_t *)&secondary_service_uuid,
                  ESP_GATT_PERM_READ, sizeof(uint16_t),
                  sizeof(GATTS_SERVICE_UUID_TEST2),
                  (uint8_t *)&GATTS_SERVICE_UUID_TEST2}},

    /* Characteristic Declaration */
    [IDX_CHAR_A] = {{ESP_GATT_AUTO_RSP},
                    {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid,
                     ESP_GATT_PERM_READ, CHAR_DECLARATION_SIZE,
                     CHAR_DECLARATION_SIZE,
                     (uint8_t *)&char_prop_read_write_notify}},

    /* Characteristic Value */
    [IDX_CHAR_VAL_A] = {{ESP_GATT_AUTO_RSP},
                        {ESP_UUID_LEN_16, (uint8_t *)&GATTS_CHAR_UUID_TEST_A2,
                         ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
                         GATTS_DEMO_CHAR_VAL_LEN_MAX, sizeof(char_value),
                         (uint8_t *)char_value}},

    /* Client Characteristic Configuration Descriptor */
    [IDX_CHAR_CFG_A] = {{ESP_GATT_AUTO_RSP},
                        {ESP_UUID_LEN_16,
                         (uint8_t *)&character_client_config_uuid,
                         ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
                         sizeof(uint16_t), sizeof(heart_measurement_ccc),
                         (uint8_t *)heart_measurement_ccc}},

    /* Characteristic Declaration */
    [IDX_CHAR_B] = {{ESP_GATT_AUTO_RSP},
                    {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid,
                     ESP_GATT_PERM_READ, CHAR_DECLARATION_SIZE,
                     CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read}},

    /* Characteristic Value */
    [IDX_CHAR_VAL_B] = {{ESP_GATT_AUTO_RSP},
                        {ESP_UUID_LEN_16, (uint8_t *)&GATTS_CHAR_UUID_TEST_B,
                         ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
                         GATTS_DEMO_CHAR_VAL_LEN_MAX, sizeof(char_value),
                         (uint8_t *)char_value}},

    /* Characteristic Declaration */
    [IDX_CHAR_C] = {{ESP_GATT_AUTO_RSP},
                    {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid,
                     ESP_GATT_PERM_READ, CHAR_DECLARATION_SIZE,
                     CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_write}},

    /* Characteristic Value */
    [IDX_CHAR_VAL_C] = {{ESP_GATT_AUTO_RSP},
                        {ESP_UUID_LEN_16, (uint8_t *)&GATTS_CHAR_UUID_TEST_C,
                         ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
                         GATTS_DEMO_CHAR_VAL_LEN_MAX, sizeof(char_value),
                         (uint8_t *)char_value}},
};

ble_gatts_t ble_gatt_conf = {
    .profiles_num = PROFILE_TOTAL_NUM,
    .event_handler_cb = gatts_event_handler,
    .profiles = {[PROFILE_A] = {.gatts_cb = gatt_profile_a_event_handler,
                                .gatts_if = ESP_GATT_IF_NONE},
                 [PROFILE_B] = {.gatts_cb = gatt_profile_b_event_handler,
                                .gatts_if = ESP_GATT_IF_NONE}}};

ble_gap_t ble_gap_conf = {.conf_type = BLE_GAP_BROADCASTER_CENTRAL,
                          .event_handler_cb = gap_event_handler,
                          .adv_params = BLE_ADV_PARAMS_CONFIG_DEFAULT(),
                          .adv_data = BLE_ADV_DATA_CONFIG_DEFAULT(),
                          .scan_rsp_data = BLE_SCAN_RSP_DATA_CONFIG_DEFAULT()};

ble_config_t ble_conf = {.gap_config = &ble_gap_conf,
                         .gatt_config = &ble_gatt_conf,
                         .bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT()};

void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if,
                         esp_ble_gatts_cb_param_t *param) {
  /* If event is register event, store the gatts_if for each profile */
  if (event == ESP_GATTS_REG_EVT) {
    if (param->reg.status == ESP_GATT_OK) {
      ble_gatt_conf.profiles[param->reg.app_id].gatts_if = gatts_if;
    } else {
      ESP_LOGI(BLE_TEST_TAG, "Reg app failed, app_id %04x, status %d\n",
               param->reg.app_id, param->reg.status);
      return;
    }
  }

  /* If the gatts_if equal to profile A, call profile A cb handler,
   * so here call each profile's callback */
  do {
    int idx;
    for (idx = 0; idx < PROFILE_TOTAL_NUM; idx++) {
      if (gatts_if == ESP_GATT_IF_NONE ||
          gatts_if == ble_gatt_conf.profiles[idx].gatts_if) {
        if (ble_gatt_conf.profiles[idx].gatts_cb) {
          ble_gatt_conf.profiles[idx].gatts_cb(event, gatts_if, param);
        }
      }
    }
  } while (0);
}

void gap_event_handler(esp_gap_ble_cb_event_t event,
                       esp_ble_gap_cb_param_t *param) {
  switch (event) {
    case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
      adv_config_done &= (~ADV_CONFIG_FLAG);
      if (adv_config_done == 0) {
        esp_ble_gap_start_advertising(&ble_conf.gap_config->adv_params);
      }
      break;
    case ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT:
      adv_config_done &= (~SCAN_RSP_CONFIG_FLAG);
      if (adv_config_done == 0) {
        esp_ble_gap_start_advertising(&ble_conf.gap_config->adv_params);
      }
      break;
    case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
      /* advertising start complete event to indicate advertising start
       * successfully or failed */
      if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
        ESP_LOGE(BLE_TEST_TAG, "advertising start failed");
      } else {
        ESP_LOGI(BLE_TEST_TAG, "advertising start successfully");
      }
      break;
    case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
      if (param->adv_stop_cmpl.status != ESP_BT_STATUS_SUCCESS) {
        ESP_LOGE(BLE_TEST_TAG, "Advertising stop failed");
      } else {
        ESP_LOGI(BLE_TEST_TAG, "Stop adv successfully\n");
      }
      break;
    case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
      ESP_LOGI(
          BLE_TEST_TAG,
          "update connection params status = %d, min_int = %d, max_int = "
          "%d,conn_int = %d,latency = %d, timeout = %d",
          param->update_conn_params.status, param->update_conn_params.min_int,
          param->update_conn_params.max_int, param->update_conn_params.conn_int,
          param->update_conn_params.latency, param->update_conn_params.timeout);
      break;
    default:
      break;
  }
}

void gatt_profile_a_event_handler(esp_gatts_cb_event_t event,
                                  esp_gatt_if_t gatts_if,
                                  esp_ble_gatts_cb_param_t *param) {
  switch (event) {
    case ESP_GATTS_REG_EVT: {
      esp_err_t set_dev_name_ret =
          esp_ble_gap_set_device_name(TEST_DEVICE_NAME);
      if (set_dev_name_ret) {
        ESP_LOGE(BLE_TEST_TAG, "set device name failed, error code = %x",
                 set_dev_name_ret);
      }
#ifdef CONFIG_SET_RAW_ADV_DATA
      esp_err_t raw_adv_ret =
          esp_ble_gap_config_adv_data_raw(raw_adv_data, sizeof(raw_adv_data));
      if (raw_adv_ret) {
        ESP_LOGE(BLE_TEST_TAG, "config raw adv data failed, error code = %x ",
                 raw_adv_ret);
      }
      adv_config_done |= ADV_CONFIG_FLAG;
      esp_err_t raw_scan_ret = esp_ble_gap_config_scan_rsp_data_raw(
          raw_scan_rsp_data, sizeof(raw_scan_rsp_data));
      if (raw_scan_ret) {
        ESP_LOGE(BLE_TEST_TAG,
                 "config raw scan rsp data failed, error code = %x",
                 raw_scan_ret);
      }
      adv_config_done |= SCAN_RSP_CONFIG_FLAG;
#else
      // config adv data
      esp_err_t ret =
          esp_ble_gap_config_adv_data(&ble_conf.gap_config->adv_data);
      if (ret) {
        ESP_LOGE(BLE_TEST_TAG, "config adv data failed, error code = %x", ret);
      }
      adv_config_done |= ADV_CONFIG_FLAG;
      // config scan response data
      ret = esp_ble_gap_config_adv_data(&ble_conf.gap_config->scan_rsp_data);
      if (ret) {
        ESP_LOGE(BLE_TEST_TAG,
                 "config scan response data failed, error code = %x", ret);
      }
      adv_config_done |= SCAN_RSP_CONFIG_FLAG;
#endif
      esp_err_t create_attr_ret = esp_ble_gatts_create_attr_tab(
          gatt_profile_a_db, gatts_if, HRS_IDX_NB, SVC_INST_ID);
      if (create_attr_ret) {
        ESP_LOGE(BLE_TEST_TAG, "create attr table failed, error code = %x",
                 create_attr_ret);
      }
    } break;
    case ESP_GATTS_READ_EVT:
      ESP_LOGI(BLE_TEST_TAG, "ESP_GATTS_READ_EVT");
      break;
    case ESP_GATTS_WRITE_EVT:
      if (!param->write.is_prep) {
        // the data length of gattc write  must be less than
        // GATTS_DEMO_CHAR_VAL_LEN_MAX.
        ESP_LOGI(BLE_TEST_TAG,
                 "GATT_WRITE_EVT, handle = %d, value len = %d, value :",
                 param->write.handle, param->write.len);
        esp_log_buffer_hex(BLE_TEST_TAG, param->write.value, param->write.len);
        if (profile_a_handle_table[IDX_CHAR_CFG_A] == param->write.handle &&
            param->write.len == 2) {
          uint16_t descr_value =
              param->write.value[1] << 8 | param->write.value[0];
          if (descr_value == 0x0001) {
            ESP_LOGI(BLE_TEST_TAG, "notify enable");
            uint8_t notify_data[15];
            for (int i = 0; i < sizeof(notify_data); ++i) {
              notify_data[i] = i % 0xff;
            }
            // the size of notify_data[] need less than MTU size
            esp_ble_gatts_send_indicate(gatts_if, param->write.conn_id,
                                        profile_a_handle_table[IDX_CHAR_VAL_A],
                                        sizeof(notify_data), notify_data,
                                        false);
          } else if (descr_value == 0x0002) {
            ESP_LOGI(BLE_TEST_TAG, "indicate enable");
            uint8_t indicate_data[15];
            for (int i = 0; i < sizeof(indicate_data); ++i) {
              indicate_data[i] = i % 0xff;
            }
            // the size of indicate_data[] need less than MTU size
            esp_ble_gatts_send_indicate(gatts_if, param->write.conn_id,
                                        profile_a_handle_table[IDX_CHAR_VAL_A],
                                        sizeof(indicate_data), indicate_data,
                                        true);
          } else if (descr_value == 0x0000) {
            ESP_LOGI(BLE_TEST_TAG, "notify/indicate disable ");
          } else {
            ESP_LOGE(BLE_TEST_TAG, "unknown descr value");
            esp_log_buffer_hex(BLE_TEST_TAG, param->write.value,
                               param->write.len);
          }
        }
        /* send response when param->write.need_rsp is true*/
        if (param->write.need_rsp) {
          esp_ble_gatts_send_response(gatts_if, param->write.conn_id,
                                      param->write.trans_id, ESP_GATT_OK, NULL);
        }
      } else {
        /* handle prepare write */
        example_prepare_write_event_env(gatts_if, &prepare_write_env, param);
      }
      break;
    case ESP_GATTS_EXEC_WRITE_EVT:
      // the length of gattc prepare write data must be less than
      // GATTS_DEMO_CHAR_VAL_LEN_MAX.
      ESP_LOGI(BLE_TEST_TAG, "ESP_GATTS_EXEC_WRITE_EVT");
      example_exec_write_event_env(&prepare_write_env, param);
      break;
    case ESP_GATTS_MTU_EVT:
      ESP_LOGI(BLE_TEST_TAG, "ESP_GATTS_MTU_EVT, MTU %d", param->mtu.mtu);
      break;
    case ESP_GATTS_CONF_EVT:
      ESP_LOGI(BLE_TEST_TAG, "ESP_GATTS_CONF_EVT, status = %d, attr_handle %d",
               param->conf.status, param->conf.handle);
      break;
    case ESP_GATTS_START_EVT:
      ESP_LOGI(BLE_TEST_TAG, "SERVICE_START_EVT, status %d, service_handle %d",
               param->start.status, param->start.service_handle);
      break;
    case ESP_GATTS_CONNECT_EVT:
      ESP_LOGI(BLE_TEST_TAG, "ESP_GATTS_CONNECT_EVT, conn_id = %d",
               param->connect.conn_id);
      esp_log_buffer_hex(BLE_TEST_TAG, param->connect.remote_bda, 6);
      esp_ble_conn_update_params_t conn_params = {0};
      memcpy(conn_params.bda, param->connect.remote_bda, sizeof(esp_bd_addr_t));
      /* For the iOS system, please refer to Apple official documents about the
       * BLE connection parameters restrictions. */
      conn_params.latency = 0;
      conn_params.max_int = 0x20;  // max_int = 0x20*1.25ms = 40ms
      conn_params.min_int = 0x10;  // min_int = 0x10*1.25ms = 20ms
      conn_params.timeout = 400;   // timeout = 400*10ms = 4000ms
      // start sent the update connection parameters to the peer device.
      esp_ble_gap_update_conn_params(&conn_params);
      break;
    case ESP_GATTS_DISCONNECT_EVT:
      ESP_LOGI(BLE_TEST_TAG, "ESP_GATTS_DISCONNECT_EVT, reason = 0x%x",
               param->disconnect.reason);
      esp_ble_gap_start_advertising(&ble_conf.gap_config->adv_params);
      break;
    case ESP_GATTS_CREAT_ATTR_TAB_EVT: {
      if (param->add_attr_tab.status != ESP_GATT_OK) {
        ESP_LOGE(BLE_TEST_TAG, "create attribute table failed, error code=0x%x",
                 param->add_attr_tab.status);
      } else if (param->add_attr_tab.num_handle != HRS_IDX_NB) {
        ESP_LOGE(BLE_TEST_TAG,
                 "create attribute table abnormally, num_handle(%d) \
                        doesn't equal to HRS_IDX_NB(%d)",
                 param->add_attr_tab.num_handle, HRS_IDX_NB);
      } else {
        ESP_LOGI(
            BLE_TEST_TAG,
            "create attribute table successfully, the number handle = %d\n",
            param->add_attr_tab.num_handle);
        memcpy(profile_a_handle_table, param->add_attr_tab.handles,
               sizeof(profile_a_handle_table));
        esp_ble_gatts_start_service(profile_a_handle_table[IDX_SVC]);
      }
      break;
    }
    case ESP_GATTS_STOP_EVT:
    case ESP_GATTS_OPEN_EVT:
    case ESP_GATTS_CANCEL_OPEN_EVT:
    case ESP_GATTS_CLOSE_EVT:
    case ESP_GATTS_LISTEN_EVT:
    case ESP_GATTS_CONGEST_EVT:
    case ESP_GATTS_UNREG_EVT:
    case ESP_GATTS_DELETE_EVT:
    default:
      break;
  }
}

void gatt_profile_b_event_handler(esp_gatts_cb_event_t event,
                                  esp_gatt_if_t gatts_if,
                                  esp_ble_gatts_cb_param_t *param) {
  ESP_LOGI(BLE_TEST_TAG, "gatt_profile_b_event_handler");
  switch (event) {
    case ESP_GATTS_REG_EVT: {
      esp_err_t create_attr_ret = esp_ble_gatts_create_attr_tab(
          gatt_profile_b_db, gatts_if, HRS_IDX_NB, SVC_INST_ID2);
      if (create_attr_ret) {
        ESP_LOGE(BLE_TEST_TAG, "create attr table failed, error code = %x",
                 create_attr_ret);
      }
    } break;
    case ESP_GATTS_READ_EVT:
      ESP_LOGI(BLE_TEST_TAG, "ESP_GATTS_READ_EVT");
      break;
    case ESP_GATTS_WRITE_EVT:
      if (!param->write.is_prep) {
        // the data length of gattc write  must be less than
        // GATTS_DEMO_CHAR_VAL_LEN_MAX.
        ESP_LOGI(BLE_TEST_TAG,
                 "GATT_WRITE_EVT, handle = %d, value len = %d, value :",
                 param->write.handle, param->write.len);
        esp_log_buffer_hex(BLE_TEST_TAG, param->write.value, param->write.len);
        if (profile_b_handle_table[IDX_CHAR_CFG_A] == param->write.handle &&
            param->write.len == 2) {
          uint16_t descr_value =
              param->write.value[1] << 8 | param->write.value[0];
          if (descr_value == 0x0001) {
            ESP_LOGI(BLE_TEST_TAG, "notify enable");
            uint8_t notify_data[15];
            for (int i = 0; i < sizeof(notify_data); ++i) {
              notify_data[i] = i % 0xff;
            }
            // the size of notify_data[] need less than MTU size
            esp_ble_gatts_send_indicate(gatts_if, param->write.conn_id,
                                        profile_b_handle_table[IDX_CHAR_VAL_A],
                                        sizeof(notify_data), notify_data,
                                        false);
          } else if (descr_value == 0x0002) {
            ESP_LOGI(BLE_TEST_TAG, "indicate enable");
            uint8_t indicate_data[15];
            for (int i = 0; i < sizeof(indicate_data); ++i) {
              indicate_data[i] = i % 0xff;
            }
            // the size of indicate_data[] need less than MTU size
            esp_ble_gatts_send_indicate(gatts_if, param->write.conn_id,
                                        profile_b_handle_table[IDX_CHAR_VAL_A],
                                        sizeof(indicate_data), indicate_data,
                                        true);
          } else if (descr_value == 0x0000) {
            ESP_LOGI(BLE_TEST_TAG, "notify/indicate disable ");
          } else {
            ESP_LOGE(BLE_TEST_TAG, "unknown descr value");
            esp_log_buffer_hex(BLE_TEST_TAG, param->write.value,
                               param->write.len);
          }
        }
        /* send response when param->write.need_rsp is true*/
        if (param->write.need_rsp) {
          esp_ble_gatts_send_response(gatts_if, param->write.conn_id,
                                      param->write.trans_id, ESP_GATT_OK, NULL);
        }
      } else {
        /* handle prepare write */
        example_prepare_write_event_env(gatts_if, &prepare_write_env, param);
      }
      break;
    case ESP_GATTS_EXEC_WRITE_EVT:
      // the length of gattc prepare write data must be less than
      // GATTS_DEMO_CHAR_VAL_LEN_MAX.
      ESP_LOGI(BLE_TEST_TAG, "ESP_GATTS_EXEC_WRITE_EVT");
      example_exec_write_event_env(&prepare_write_env, param);
      break;
    case ESP_GATTS_MTU_EVT:
      ESP_LOGI(BLE_TEST_TAG, "ESP_GATTS_MTU_EVT, MTU %d", param->mtu.mtu);
      break;
    case ESP_GATTS_CONF_EVT:
      ESP_LOGI(BLE_TEST_TAG, "ESP_GATTS_CONF_EVT, status = %d, attr_handle %d",
               param->conf.status, param->conf.handle);
      break;
    case ESP_GATTS_START_EVT:
      ESP_LOGI(BLE_TEST_TAG, "SERVICE_START_EVT, status %d, service_handle %d",
               param->start.status, param->start.service_handle);
      break;
    case ESP_GATTS_CONNECT_EVT:
      ESP_LOGI(BLE_TEST_TAG, "ESP_GATTS_CONNECT_EVT, conn_id = %d",
               param->connect.conn_id);
      esp_log_buffer_hex(BLE_TEST_TAG, param->connect.remote_bda, 6);
      esp_ble_conn_update_params_t conn_params = {0};
      memcpy(conn_params.bda, param->connect.remote_bda, sizeof(esp_bd_addr_t));
      /* For the iOS system, please refer to Apple official documents about the
       * BLE connection parameters restrictions. */
      conn_params.latency = 0;
      conn_params.max_int = 0x20;  // max_int = 0x20*1.25ms = 40ms
      conn_params.min_int = 0x10;  // min_int = 0x10*1.25ms = 20ms
      conn_params.timeout = 400;   // timeout = 400*10ms = 4000ms
      // start sent the update connection parameters to the peer device.
      esp_ble_gap_update_conn_params(&conn_params);
      break;
    case ESP_GATTS_DISCONNECT_EVT:
      ESP_LOGI(BLE_TEST_TAG, "ESP_GATTS_DISCONNECT_EVT, reason = 0x%x",
               param->disconnect.reason);
      esp_ble_gap_start_advertising(&ble_conf.gap_config->adv_params);
      break;
    case ESP_GATTS_CREAT_ATTR_TAB_EVT: {
      if (param->add_attr_tab.status != ESP_GATT_OK) {
        ESP_LOGE(BLE_TEST_TAG, "create attribute table failed, error code=0x%x",
                 param->add_attr_tab.status);
      } else if (param->add_attr_tab.num_handle != HRS_IDX_NB) {
        ESP_LOGE(BLE_TEST_TAG,
                 "create attribute table abnormally, num_handle(%d) \
                        doesn't equal to HRS_IDX_NB(%d)",
                 param->add_attr_tab.num_handle, HRS_IDX_NB);
      } else {
        ESP_LOGI(
            BLE_TEST_TAG,
            "create attribute table successfully, the number handle = %d\n",
            param->add_attr_tab.num_handle);
        memcpy(profile_b_handle_table, param->add_attr_tab.handles,
               sizeof(profile_b_handle_table));
        esp_ble_gatts_start_service(profile_b_handle_table[IDX_SVC]);
      }
      break;
    }
    case ESP_GATTS_STOP_EVT:
    case ESP_GATTS_OPEN_EVT:
    case ESP_GATTS_CANCEL_OPEN_EVT:
    case ESP_GATTS_CLOSE_EVT:
    case ESP_GATTS_LISTEN_EVT:
    case ESP_GATTS_CONGEST_EVT:
    case ESP_GATTS_UNREG_EVT:
    case ESP_GATTS_DELETE_EVT:
    default:
      break;
  }
}

void example_prepare_write_event_env(esp_gatt_if_t gatts_if,
                                     prepare_type_env_t *prepare_write_env,
                                     esp_ble_gatts_cb_param_t *param) {
  ESP_LOGI(BLE_TEST_TAG, "prepare write, handle = %d, value len = %d",
           param->write.handle, param->write.len);
  esp_gatt_status_t status = ESP_GATT_OK;
  if (prepare_write_env->prepare_buf == NULL) {
    prepare_write_env->prepare_buf =
        (uint8_t *)malloc(PREPARE_BUF_MAX_SIZE * sizeof(uint8_t));
    prepare_write_env->prepare_len = 0;
    if (prepare_write_env->prepare_buf == NULL) {
      ESP_LOGE(BLE_TEST_TAG, "%s, Gatt_server prep no mem", __func__);
      status = ESP_GATT_NO_RESOURCES;
    }
  } else {
    if (param->write.offset > PREPARE_BUF_MAX_SIZE) {
      status = ESP_GATT_INVALID_OFFSET;
    } else if ((param->write.offset + param->write.len) >
               PREPARE_BUF_MAX_SIZE) {
      status = ESP_GATT_INVALID_ATTR_LEN;
    }
  }
  /*send response when param->write.need_rsp is true */
  if (param->write.need_rsp) {
    esp_gatt_rsp_t *gatt_rsp = (esp_gatt_rsp_t *)malloc(sizeof(esp_gatt_rsp_t));
    if (gatt_rsp != NULL) {
      gatt_rsp->attr_value.len = param->write.len;
      gatt_rsp->attr_value.handle = param->write.handle;
      gatt_rsp->attr_value.offset = param->write.offset;
      gatt_rsp->attr_value.auth_req = ESP_GATT_AUTH_REQ_NONE;
      memcpy(gatt_rsp->attr_value.value, param->write.value, param->write.len);
      esp_err_t response_err =
          esp_ble_gatts_send_response(gatts_if, param->write.conn_id,
                                      param->write.trans_id, status, gatt_rsp);
      if (response_err != ESP_OK) {
        ESP_LOGE(BLE_TEST_TAG, "Send response error");
      }
      free(gatt_rsp);
    } else {
      ESP_LOGE(BLE_TEST_TAG, "%s, malloc failed", __func__);
    }
  }
  if (status != ESP_GATT_OK) {
    return;
  }
  memcpy(prepare_write_env->prepare_buf + param->write.offset,
         param->write.value, param->write.len);
  prepare_write_env->prepare_len += param->write.len;
}

void example_exec_write_event_env(prepare_type_env_t *prepare_write_env,
                                  esp_ble_gatts_cb_param_t *param) {
  if (param->exec_write.exec_write_flag == ESP_GATT_PREP_WRITE_EXEC &&
      prepare_write_env->prepare_buf) {
    esp_log_buffer_hex(BLE_TEST_TAG, prepare_write_env->prepare_buf,
                       prepare_write_env->prepare_len);
  } else {
    ESP_LOGI(BLE_TEST_TAG, "ESP_GATT_PREP_WRITE_CANCEL");
  }
  if (prepare_write_env->prepare_buf) {
    free(prepare_write_env->prepare_buf);
    prepare_write_env->prepare_buf = NULL;
  }
  prepare_write_env->prepare_len = 0;
}

TEST_CASE("Ble init test", "[BLE_API]") {
  TEST_ASSERT_EQUAL(BLE_OK, ble_init(&ble_conf));
}
