// Copyright 2023 PWr in Space, Krzysztof Gliwiński
#include "ble_task.h"

#define BLE_APP_TAG "BLE_APP"

#define adv_config_flag (1 << 0)
#define scan_rsp_config_flag (1 << 1)

/*! \brief Declared service and characteristics UUIDS */
const uint16_t GATTS_CONSOLE_SERVICE_UUID = 0x00FE;
const uint16_t GATTS_CONSOLE_READ_CHAR_UUID = 0xFF01;
const uint16_t GATTS_CONSOLE_WRITE_CHAR_UUID = 0xFF02;

/*! \brief Declared constants from */
const uint16_t primary_service_uuid = ESP_GATT_UUID_PRI_SERVICE;
const uint16_t secondary_service_uuid = ESP_GATT_UUID_PRI_SERVICE;
const uint16_t character_declaration_uuid = ESP_GATT_UUID_CHAR_DECLARE;
const uint16_t character_client_config_uuid = ESP_GATT_UUID_CHAR_CLIENT_CONFIG;
const uint8_t char_prop_read = ESP_GATT_CHAR_PROP_BIT_READ;
const uint8_t char_prop_write = ESP_GATT_CHAR_PROP_BIT_WRITE;
const uint8_t char_prop_read_write = ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_READ;

/*! \brief Project specific constants */
#define GATTS_MACKI_CHAR_VAL_LEN_MAX 500
#define PREPARE_BUF_MAX_SIZE 1024
#define CHAR_DECLARATION_SIZE (sizeof(uint8_t))
#define SVC_INST_ID 0
prepare_type_env_t prepare_write_env;

static uint8_t raw_adv_data[] = {
    /* flags */
    0x02, 0x01, 0x06,
    /* tx power*/
    0x02, 0x0a, 0xeb,
    /* service uuid */
    0x03, 0x03, 0xFF, 0x00,
    /* device name */
    0x0f, 0x09, 'E', 'S', 'P', '_', 'G', 'A', 'T', 'T', 'S', '_', 'D', 'E', 'M', 'O'};
static uint8_t raw_scan_rsp_data[] = {
    /* flags */
    0x02, 0x01, 0x06,
    /* tx power */
    0x02, 0x0a, 0xeb,
    /* service uuid */
    0x03, 0x03, 0xFF, 0x00};

const uint8_t char_value[4] = {0x69, 0x42, 0x04, 0x20};

/*! \brief Project specific GATT profile defines,
 *  \note READ means server (ESP32) read, analogically write means server (ESP32) write
 */
typedef enum { CONSOLE_SERVICE, PROFILE_TOTAL_NUM } rosalia_ble_gatt_profile_names_t;

typedef enum {
    CONSOLE_SERVICE_IDX = 0,
    CONSOLE_READ_IDX,
    CONSOLE_READ_VAL_IDX,
    CONSOLE_WRITE_IDX,
    CONSOLE_WRITE_VAL_IDX,
    CONSOLE_MAX_IDX
} rosalia_main_valve_service_idxs;

static uint8_t adv_config_done = 0;

static uint8_t adv_service_uuid128[32] = BLE_UUID_CONFIG_DEFAULT();

static ble_gap_t ble_gap_conf = {.conf_type = BLE_GAP_BROADCASTER_CENTRAL,
                                 .event_handler_cb = gap_event_handler,
                                 .adv_data = BLE_ADV_DATA_CONFIG_DEFAULT(),
                                 .scan_rsp_data = BLE_SCAN_RSP_DATA_CONFIG_DEFAULT(),
                                 .adv_params = BLE_ADV_PARAMS_CONFIG_DEFAULT()};

static ble_gatts_t ble_gatt_conf = {
    .profiles_num = PROFILE_TOTAL_NUM,
    .event_handler_cb = gatts_event_handler,
    .profiles = {[CONSOLE_SERVICE] = {.gatts_cb = gatt_profile_console_event_handler,
                                      .gatts_if = ESP_GATT_IF_NONE}}};

static ble_config_t ble_conf = {.gap_config = &ble_gap_conf,
                                .gatt_config = &ble_gatt_conf,
                                .bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT()};

// TODO(Glibus): in the future, look how gatts database could be defined with 128bit UUIDs
static uint16_t profile_macki_handle_table[CONSOLE_MAX_IDX];
const esp_gatts_attr_db_t gatt_profile_macki_db[CONSOLE_MAX_IDX] = {
    [CONSOLE_SERVICE_IDX] = {{ESP_GATT_AUTO_RSP},
                             {ESP_UUID_LEN_16, (uint8_t*)&primary_service_uuid, ESP_GATT_PERM_READ,
                              sizeof(uint16_t), sizeof(GATTS_CONSOLE_SERVICE_UUID),
                              (uint8_t*)&GATTS_CONSOLE_SERVICE_UUID}},
    [CONSOLE_READ_IDX] = {{ESP_GATT_AUTO_RSP},
                          {ESP_UUID_LEN_16, (uint8_t*)&character_declaration_uuid,
                           ESP_GATT_PERM_READ, CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE,
                           (uint8_t*)&char_prop_read}},
    [CONSOLE_READ_VAL_IDX] = {{ESP_GATT_AUTO_RSP},
                              {ESP_UUID_LEN_16, (uint8_t*)&GATTS_CONSOLE_READ_CHAR_UUID,
                               ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
                               GATTS_MACKI_CHAR_VAL_LEN_MAX, sizeof(char_value),
                               (uint8_t*)&char_value}},
    [CONSOLE_WRITE_IDX] = {{ESP_GATT_AUTO_RSP},
                           {ESP_UUID_LEN_16, (uint8_t*)&character_declaration_uuid,
                            ESP_GATT_PERM_READ, CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE,
                            (uint8_t*)&char_prop_read_write}},
    [CONSOLE_WRITE_VAL_IDX] = {
        {ESP_GATT_AUTO_RSP},
        {ESP_UUID_LEN_16, (uint8_t*)&GATTS_CONSOLE_WRITE_CHAR_UUID,
         ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE, GATTS_MACKI_CHAR_VAL_LEN_MAX, sizeof(char_value),
         (uint8_t*)&char_value}}};

void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t* param) {
    switch (event) {
        case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
            adv_config_done &= (~adv_config_flag);
            if (adv_config_done == 0) {
                esp_ble_gap_start_advertising(&ble_conf.gap_config->adv_params);
            }
            break;
        case ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT:
            adv_config_done &= (~scan_rsp_config_flag);
            if (adv_config_done == 0) {
                esp_ble_gap_start_advertising(&ble_conf.gap_config->adv_params);
            }
            break;
        case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
            /* advertising start complete event to indicate advertising start
             * successfully or failed */
            if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
                ESP_LOGE(BLE_APP_TAG, "advertising start failed");
            } else {
                ESP_LOGI(BLE_APP_TAG, "advertising start successfully");
            }
            break;
        case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
            if (param->adv_stop_cmpl.status != ESP_BT_STATUS_SUCCESS) {
                ESP_LOGE(BLE_APP_TAG, "Advertising stop failed");
            } else {
                ESP_LOGI(BLE_APP_TAG, "Stop adv successfully\n");
            }
            break;
        case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
            ESP_LOGI(BLE_APP_TAG,
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

// TODO(Gliwus): In the future, refactor it to encapsulate each case in a separate function
void gatt_profile_console_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if,
                                        esp_ble_gatts_cb_param_t* param) {
    switch (event) {
        case ESP_GATTS_REG_EVT:
            esp_err_t set_dev_name_ret = esp_ble_gap_set_device_name(BLE_DEVICE_NAME);
            if (set_dev_name_ret) {
                ESP_LOGE(BLE_APP_TAG, "set device name failed, error code = %x", set_dev_name_ret);
            }
            // config adv data
            esp_err_t ret = esp_ble_gap_config_adv_data(&ble_conf.gap_config->adv_data);
            if (ret) {
                ESP_LOGE(BLE_APP_TAG, "config adv data failed, error code = %x", ret);
            }
            adv_config_done |= adv_config_flag;
            // config scan response data
            ret = esp_ble_gap_config_adv_data(&ble_conf.gap_config->scan_rsp_data);
            if (ret) {
                ESP_LOGE(BLE_APP_TAG, "config scan response data failed, error code = %x", ret);
            }
            adv_config_done |= scan_rsp_config_flag;
            esp_err_t create_attr_ret = esp_ble_gatts_create_attr_tab(
                gatt_profile_macki_db, gatts_if, CONSOLE_MAX_IDX, SVC_INST_ID);
            if (create_attr_ret) {
                ESP_LOGE(BLE_APP_TAG, "create attr table failed, error code = %x", create_attr_ret);
            }
            break;
        case ESP_GATTS_CREAT_ATTR_TAB_EVT:
            if (param->add_attr_tab.status != ESP_GATT_OK) {
                ESP_LOGE(BLE_APP_TAG, "create attribute table failed, error code=0x%x",
                         param->add_attr_tab.status);
            } else if (param->add_attr_tab.num_handle != CONSOLE_MAX_IDX) {
                ESP_LOGE(BLE_APP_TAG,
                         "create attribute table abnormally, num_handle(%d) \
                        doesn't equal to HRS_IDX_NB(%d)",
                         param->add_attr_tab.num_handle, CONSOLE_MAX_IDX);
            } else {
                ESP_LOGI(BLE_APP_TAG,
                         "create attribute table successfully, the number handle = %d\n",
                         param->add_attr_tab.num_handle);
                memcpy(profile_macki_handle_table, param->add_attr_tab.handles,
                       sizeof(profile_macki_handle_table));
                esp_ble_gatts_start_service(profile_macki_handle_table[CONSOLE_SERVICE_IDX]);
            }
            break;
        case ESP_GATTS_WRITE_EVT:
            if (!param->write.is_prep) {
                // the data length of gattc write  must be less than
                // GATTS_DEMO_CHAR_VAL_LEN_MAX.
                ESP_LOGI(BLE_APP_TAG, "GATT_WRITE_EVT, handle = %d, value len = %d, value :",
                         param->write.handle, param->write.len);
                esp_log_buffer_hex(BLE_APP_TAG, param->write.value, param->write.len);
                if (profile_macki_handle_table[CONSOLE_MAX_IDX] == param->write.handle &&
                    param->write.len == 2) {
                    uint16_t descr_value = param->write.value[1] << 8 | param->write.value[0];
                    if (descr_value == 0x0001) {
                        ESP_LOGI(BLE_APP_TAG, "notify enable");
                        uint8_t notify_data[15];
                        for (int i = 0; i < sizeof(notify_data); ++i) {
                            notify_data[i] = i % 0xff;
                        }
                        // the size of notify_data[] need less than MTU size
                        esp_ble_gatts_send_indicate(
                            gatts_if, param->write.conn_id,
                            profile_macki_handle_table[CONSOLE_WRITE_VAL_IDX], sizeof(notify_data),
                            notify_data, false);
                    } else if (descr_value == 0x0002) {
                        ESP_LOGI(BLE_APP_TAG, "indicate enable");
                        uint8_t indicate_data[15];
                        for (int i = 0; i < sizeof(indicate_data); ++i) {
                            indicate_data[i] = i % 0xff;
                        }
                        // the size of indicate_data[] need less than MTU size
                        esp_ble_gatts_send_indicate(
                            gatts_if, param->write.conn_id,
                            profile_macki_handle_table[CONSOLE_WRITE_VAL_IDX],
                            sizeof(indicate_data), indicate_data, true);
                    } else if (descr_value == 0x0000) {
                        ESP_LOGI(BLE_APP_TAG, "notify/indicate disable ");
                    } else {
                        ESP_LOGE(BLE_APP_TAG, "unknown descr value");
                        esp_log_buffer_hex(BLE_APP_TAG, param->write.value, param->write.len);
                    }
                }
                /* send response when param->write.need_rsp is true*/
                // if (param->write.need_rsp) {
                //     esp_ble_gatts_send_response(gatts_if, param->write.conn_id,
                //                                 param->write.trans_id, ESP_GATT_OK, NULL);
                // }
            } else {
                /* handle prepare write */
                console_prepare_write_event_env(gatts_if, &prepare_write_env, param);
            }
            break;
        case ESP_GATTS_EXEC_WRITE_EVT:
            // the length of gattc prepare write data must be less than
            // GATTS_DEMO_CHAR_VAL_LEN_MAX.
            ESP_LOGI(BLE_APP_TAG, "ESP_GATTS_EXEC_WRITE_EVT");
            console_exec_write_event_env(&prepare_write_env, param);
            break;
        case ESP_GATTS_READ_EVT:
            ESP_LOGI(BLE_APP_TAG, "ESP_GATTS_READ_EVT");
            break;
        case ESP_GATTS_MTU_EVT:
            ESP_LOGI(BLE_APP_TAG, "ESP_GATTS_MTU_EVT, MTU %d", param->mtu.mtu);
            break;
        case ESP_GATTS_CONF_EVT:
            ESP_LOGI(BLE_APP_TAG, "ESP_GATTS_CONF_EVT, status = %d, attr_handle %d",
                     param->conf.status, param->conf.handle);
            break;
        case ESP_GATTS_START_EVT:
            ESP_LOGI(BLE_APP_TAG, "SERVICE_START_EVT, status %d, service_handle %d",
                     param->start.status, param->start.service_handle);
            break;
        case ESP_GATTS_CONNECT_EVT:
            ESP_LOGI(BLE_APP_TAG, "ESP_GATTS_CONNECT_EVT, conn_id = %d", param->connect.conn_id);
            esp_log_buffer_hex(BLE_APP_TAG, param->connect.remote_bda, 6);
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
            ESP_LOGI(BLE_APP_TAG, "ESP_GATTS_DISCONNECT_EVT, reason = 0x%x",
                     param->disconnect.reason);
            esp_ble_gap_start_advertising(&ble_conf.gap_config->adv_params);
            break;
        case ESP_GATTS_STOP_EVT:
        case ESP_GATTS_OPEN_EVT:
        case ESP_GATTS_CANCEL_OPEN_EVT:
        case ESP_GATTS_CLOSE_EVT:
        case ESP_GATTS_LISTEN_EVT:
        case ESP_GATTS_CONGEST_EVT:
        case ESP_GATTS_UNREG_EVT:
        case ESP_GATTS_DELETE_EVT:
        default:
            ESP_LOGE(BLE_APP_TAG, "Unhandled GATT event: %d", event);
            break;
    }
}

void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if,
                         esp_ble_gatts_cb_param_t* param) {
    /* If event is register event, store the gatts_if for each profile */
    if (event == ESP_GATTS_REG_EVT) {
        if (param->reg.status == ESP_GATT_OK) {
            ble_gatt_conf.profiles[param->reg.app_id].gatts_if = gatts_if;
        } else {
            ESP_LOGI(BLE_APP_TAG, "Reg app failed, app_id %04x, status %d\n", param->reg.app_id,
                     param->reg.status);
            return;
        }
    }

    /* If the gatts_if equal to profile A, call profile A cb handler,
     * so here call each profile's callback */
    do {
        int idx;
        for (idx = 0; idx < PROFILE_TOTAL_NUM; idx++) {
            if (gatts_if == ESP_GATT_IF_NONE || gatts_if == ble_gatt_conf.profiles[idx].gatts_if) {
                if (ble_gatt_conf.profiles[idx].gatts_cb) {
                    ble_gatt_conf.profiles[idx].gatts_cb(event, gatts_if, param);
                }
            }
        }
    } while (0);
}

void console_prepare_write_event_env(esp_gatt_if_t gatts_if, prepare_type_env_t* prepare_write_env,
                                     esp_ble_gatts_cb_param_t* param) {
    ESP_LOGI(BLE_APP_TAG, "prepare write, handle = %d, value len = %d", param->write.handle,
             param->write.len);
    esp_gatt_status_t status = ESP_GATT_OK;
    if (prepare_write_env->prepare_buf == NULL) {
        prepare_write_env->prepare_buf = (uint8_t*)malloc(PREPARE_BUF_MAX_SIZE * sizeof(uint8_t));
        prepare_write_env->prepare_len = 0;
        if (prepare_write_env->prepare_buf == NULL) {
            ESP_LOGE(BLE_APP_TAG, "%s, Gatt_server prep no mem", __func__);
            status = ESP_GATT_NO_RESOURCES;
        }
    } else {
        if (param->write.offset > PREPARE_BUF_MAX_SIZE) {
            status = ESP_GATT_INVALID_OFFSET;
        } else if ((param->write.offset + param->write.len) > PREPARE_BUF_MAX_SIZE) {
            status = ESP_GATT_INVALID_ATTR_LEN;
        }
    }
    /*send response when param->write.need_rsp is true */
    if (param->write.need_rsp) {
        esp_gatt_rsp_t* gatt_rsp = (esp_gatt_rsp_t*)malloc(sizeof(esp_gatt_rsp_t));
        if (gatt_rsp != NULL) {
            gatt_rsp->attr_value.len = param->write.len;
            gatt_rsp->attr_value.handle = param->write.handle;
            gatt_rsp->attr_value.offset = param->write.offset;
            gatt_rsp->attr_value.auth_req = ESP_GATT_AUTH_REQ_NONE;
            memcpy(gatt_rsp->attr_value.value, param->write.value, param->write.len);
            esp_err_t response_err = esp_ble_gatts_send_response(
                gatts_if, param->write.conn_id, param->write.trans_id, status, gatt_rsp);
            if (response_err != ESP_OK) {
                ESP_LOGE(BLE_APP_TAG, "Send response error");
            }
            free(gatt_rsp);
        } else {
            ESP_LOGE(BLE_APP_TAG, "%s, malloc failed", __func__);
        }
    }
    if (status != ESP_GATT_OK) {
        return;
    }
    memcpy(prepare_write_env->prepare_buf + param->write.offset, param->write.value,
           param->write.len);
    prepare_write_env->prepare_len += param->write.len;
}

void console_exec_write_event_env(prepare_type_env_t* prepare_write_env,
                                  esp_ble_gatts_cb_param_t* param) {
    if (param->exec_write.exec_write_flag == ESP_GATT_PREP_WRITE_EXEC &&
        prepare_write_env->prepare_buf) {
        esp_log_buffer_hex(BLE_APP_TAG, prepare_write_env->prepare_buf,
                           prepare_write_env->prepare_len);
    } else {
        ESP_LOGI(BLE_APP_TAG, "ESP_GATT_PREP_WRITE_CANCEL");
    }
    if (prepare_write_env->prepare_buf) {
        free(prepare_write_env->prepare_buf);
        prepare_write_env->prepare_buf = NULL;
    }
    prepare_write_env->prepare_len = 0;
}

void ble_init_task(void* arg) {
    if (ble_init(&ble_conf) != BLE_OK) {
        ESP_LOGE(BLE_APP_TAG, "BLE init failed: %s", __func__);
        return;
    }

    ESP_LOGI(BLE_APP_TAG, "BLE init done");

    // ESP_LOGI(BLE_APP_TAG, "%d", ble_conf.bt_cfg);
    //   vTaskDelete(NULL);
}
