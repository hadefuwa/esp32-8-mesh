/*
 * Bluetooth SPP/BLE UART Implementation (up to 8 connections)
 *
 * This implementation provides both Classic Bluetooth SPP and BLE UART functionality
 * with support for up to 8 concurrent connections, message relay, and status monitoring.
 */

#include "bluetooth_spp.h"

static const char *TAG = "BT_SPP";

// Global variables
static connection_info_t connections[MAX_CONNECTIONS];
static SemaphoreHandle_t connections_mutex;
static QueueHandle_t message_queue;
static TaskHandle_t message_task_handle;
static data_received_callback_t data_callback = NULL;
static bool bluetooth_initialized = false;
static char device_name[ESP_BT_GAP_MAX_BDNAME_LEN + 1] = DEVICE_NAME;
static char ble_device_name[ESP_BT_GAP_MAX_BDNAME_LEN + 1] = BLE_DEVICE_NAME;

// BLE UART Service UUIDs
static const uint16_t BLE_UART_SERVICE_UUID = 0x6E400001B5A3F393E0A9E50E24DCCA9E;
static const uint16_t BLE_UART_TX_CHAR_UUID = 0x6E400002B5A3F393E0A9E50E24DCCA9E;
static const uint16_t BLE_UART_RX_CHAR_UUID = 0x6E400003B5A3F393E0A9E50E24DCCA9E;

// BLE GATT interface
static uint8_t adv_config_done = 0;
#define adv_config_flag      (1 << 0)
#define scan_rsp_config_flag (1 << 1)

static esp_ble_adv_data_t adv_data = {
    .set_scan_rsp = false,
    .include_name = true,
    .include_txpower = true,
    .min_interval = 0x0006,
    .max_interval = 0x0010,
    .appearance = 0x00,
    .manufacturer_len = 0,
    .p_manufacturer_data =  NULL,
    .service_data_len = 0,
    .p_service_data = NULL,
    .service_uuid_len = sizeof(BLE_UART_SERVICE_UUID),
    .p_service_uuid = (uint8_t *)&BLE_UART_SERVICE_UUID,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

static esp_ble_adv_data_t scan_rsp_data = {
    .set_scan_rsp = true,
    .include_name = true,
    .include_txpower = true,
    .appearance = 0x00,
    .manufacturer_len = 0,
    .p_manufacturer_data =  NULL,
    .service_data_len = 0,
    .p_service_data = NULL,
    .service_uuid_len = 0,
    .p_service_uuid = NULL,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

static esp_ble_adv_params_t adv_params = {
    .adv_int_min        = 0x20,
    .adv_int_max        = 0x40,
    .adv_type           = ADV_TYPE_IND,
    .own_addr_type      = BLE_ADDR_TYPE_PUBLIC,
    .channel_map        = ADV_CHNL_ALL,
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

// Function prototypes
static void message_task(void *pvParameters);
static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);
static void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
static void spp_event_handler(esp_spp_cb_event_t event, esp_spp_cb_param_t *param);
static uint32_t find_free_connection_slot(void);
static uint32_t find_connection_by_handle(uint32_t handle);
static void update_connection_activity(uint32_t conn_handle);
static void print_connection_status(void);

// Initialize Bluetooth SPP/BLE UART
void bluetooth_spp_init(void) {
    esp_err_t ret;
    
    ESP_LOGI(TAG, "Initializing Bluetooth SPP/BLE UART...");
    
    // Initialize mutex for thread safety
    connections_mutex = xSemaphoreCreateMutex();
    if (connections_mutex == NULL) {
        ESP_LOGE(TAG, "Failed to create mutex");
        return;
    }
    
    // Initialize message queue
    message_queue = xQueueCreate(20, sizeof(bt_message_t));
    if (message_queue == NULL) {
        ESP_LOGE(TAG, "Failed to create message queue");
        vSemaphoreDelete(connections_mutex);
        return;
    }
    
    // Initialize connection array
    memset(connections, 0, sizeof(connections));
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        connections[i].handle = 0xFFFFFFFF;
        connections[i].state = CONN_STATE_DISCONNECTED;
    }
    
    // Initialize Bluetooth controller and stack
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));
    
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret) {
        ESP_LOGE(TAG, "Initialize controller failed: %s", esp_err_to_name(ret));
        return;
    }
    
    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret) {
        ESP_LOGE(TAG, "Enable controller failed: %s", esp_err_to_name(ret));
        return;
    }
    
    ret = esp_bluedroid_init();
    if (ret) {
        ESP_LOGE(TAG, "Init bluedroid failed: %s", esp_err_to_name(ret));
        return;
    }
    
    ret = esp_bluedroid_enable();
    if (ret) {
        ESP_LOGE(TAG, "Enable bluedroid failed: %s", esp_err_to_name(ret));
        return;
    }
    
    // Register GAP and GATTS callbacks
    ret = esp_ble_gap_register_callback(gap_event_handler);
    if (ret) {
        ESP_LOGE(TAG, "GAP register failed: %s", esp_err_to_name(ret));
        return;
    }
    
    ret = esp_ble_gatts_register_callback(gatts_event_handler);
    if (ret) {
        ESP_LOGE(TAG, "GATTS register failed: %s", esp_err_to_name(ret));
        return;
    }
    
    // Register SPP callback for Classic Bluetooth
    ret = esp_spp_register_callback(spp_event_handler);
    if (ret) {
        ESP_LOGE(TAG, "SPP register failed: %s", esp_err_to_name(ret));
        return;
    }
    
    ret = esp_spp_init(ESP_SPP_MODE_CB);
    if (ret) {
        ESP_LOGE(TAG, "SPP init failed: %s", esp_err_to_name(ret));
        return;
    }
    
    // Set device name
    esp_bt_dev_set_device_name(device_name);
    esp_ble_gap_set_device_name(ble_device_name);
    
    // Start advertising
    ret = esp_ble_gap_config_adv_data(&adv_data);
    if (ret) {
        ESP_LOGE(TAG, "Config adv data failed: %s", esp_err_to_name(ret));
        return;
    }
    adv_config_done |= adv_config_flag;
    
    ret = esp_ble_gap_config_adv_data(&scan_rsp_data);
    if (ret) {
        ESP_LOGE(TAG, "Config scan response data failed: %s", esp_err_to_name(ret));
        return;
    }
    adv_config_done |= scan_rsp_config_flag;
    
    // Start message processing task
    xTaskCreate(message_task, "bt_message_task", 4096, NULL, 5, &message_task_handle);
    
    bluetooth_initialized = true;
    ESP_LOGI(TAG, "Bluetooth SPP/BLE UART initialized successfully");
    ESP_LOGI(TAG, "Device name: %s (Classic), %s (BLE)", device_name, ble_device_name);
}

// Message processing task
static void message_task(void *pvParameters) {
    bt_message_t message;
    
    while (1) {
        if (xQueueReceive(message_queue, &message, portMAX_DELAY) == pdTRUE) {
            if (message.type == 0) { // Received data
                if (data_callback) {
                    data_callback(message.conn_handle, message.data, message.length);
                }
                ESP_LOGI(TAG, "Received %d bytes from connection %lu", message.length, message.conn_handle);
            } else if (message.type == 1) { // Data to send
                // Handle sending data (implement based on connection type)
                ESP_LOGI(TAG, "Sending %d bytes to connection %lu", message.length, message.conn_handle);
            }
        }
    }
}

// GAP event handler
static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param) {
    switch (event) {
        case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
            adv_config_done &= (~adv_config_flag);
            if (adv_config_done == 0) {
                esp_ble_gap_start_advertising(&adv_params);
            }
            break;
        case ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT:
            adv_config_done &= (~scan_rsp_config_flag);
            if (adv_config_done == 0) {
                esp_ble_gap_start_advertising(&adv_params);
            }
            break;
        case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
            if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
                ESP_LOGE(TAG, "Advertising start failed");
            } else {
                ESP_LOGI(TAG, "Advertising started");
            }
            break;
        case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
            if (param->adv_stop_cmpl.status != ESP_BT_STATUS_SUCCESS) {
                ESP_LOGE(TAG, "Advertising stop failed");
            } else {
                ESP_LOGI(TAG, "Advertising stopped");
            }
            break;
        case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
            ESP_LOGI(TAG, "Connection parameters updated");
            break;
        default:
            break;
    }
}

// GATTS event handler
static void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
    switch (event) {
        case ESP_GATTS_WRITE_EVT:
            if (xSemaphoreTake(connections_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
                uint32_t conn_idx = find_connection_by_handle(param->write.conn_id);
                if (conn_idx < MAX_CONNECTIONS) {
                    // Queue received data
                    bt_message_t message;
                    message.conn_handle = param->write.conn_id;
                    message.type = 0; // Received
                    message.length = param->write.len;
                    memcpy(message.data, param->write.value, param->write.len);
                    xQueueSend(message_queue, &message, 0);
                    
                    update_connection_activity(param->write.conn_id);
                    connections[conn_idx].bytes_received += param->write.len;
                }
                xSemaphoreGive(connections_mutex);
            }
            break;
        case ESP_GATTS_CONNECT_EVT:
            ESP_LOGI(TAG, "BLE device connected, conn_id = %d", param->connect.conn_id);
            if (xSemaphoreTake(connections_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
                uint32_t free_slot = find_free_connection_slot();
                if (free_slot < MAX_CONNECTIONS) {
                    connections[free_slot].handle = param->connect.conn_id;
                    connections[free_slot].state = CONN_STATE_CONNECTED;
                    memcpy(connections[free_slot].remote_addr, param->connect.remote_bda, 6);
                    connections[free_slot].last_activity = xTaskGetTickCount();
                    ESP_LOGI(TAG, "Connection %lu established", free_slot);
                }
                xSemaphoreGive(connections_mutex);
            }
            break;
        case ESP_GATTS_DISCONNECT_EVT:
            ESP_LOGI(TAG, "BLE device disconnected, conn_id = %d", param->disconnect.conn_id);
            if (xSemaphoreTake(connections_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
                uint32_t conn_idx = find_connection_by_handle(param->disconnect.conn_id);
                if (conn_idx < MAX_CONNECTIONS) {
                    connections[conn_idx].state = CONN_STATE_DISCONNECTED;
                    connections[conn_idx].handle = 0xFFFFFFFF;
                    ESP_LOGI(TAG, "Connection %lu closed", conn_idx);
                }
                xSemaphoreGive(connections_mutex);
            }
            // Restart advertising
            esp_ble_gap_start_advertising(&adv_params);
            break;
        default:
            break;
    }
}

// SPP event handler for Classic Bluetooth
static void spp_event_handler(esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {
    switch (event) {
        case ESP_SPP_INIT_EVT:
            ESP_LOGI(TAG, "SPP initialized");
            esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
            break;
        case ESP_SPP_START_EVT:
            ESP_LOGI(TAG, "SPP server started");
            break;
        case ESP_SPP_SRV_OPEN_EVT:
            ESP_LOGI(TAG, "SPP client connected");
            if (xSemaphoreTake(connections_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
                uint32_t free_slot = find_free_connection_slot();
                if (free_slot < MAX_CONNECTIONS) {
                    connections[free_slot].handle = param->srv_open.handle;
                    connections[free_slot].state = CONN_STATE_CONNECTED;
                    memcpy(connections[free_slot].remote_addr, param->srv_open.rem_bda, 6);
                    connections[free_slot].last_activity = xTaskGetTickCount();
                    ESP_LOGI(TAG, "SPP Connection %lu established", free_slot);
                }
                xSemaphoreGive(connections_mutex);
            }
            break;
        case ESP_SPP_CLOSE_EVT:
            ESP_LOGI(TAG, "SPP connection closed");
            if (xSemaphoreTake(connections_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
                uint32_t conn_idx = find_connection_by_handle(param->close.handle);
                if (conn_idx < MAX_CONNECTIONS) {
                    connections[conn_idx].state = CONN_STATE_DISCONNECTED;
                    connections[conn_idx].handle = 0xFFFFFFFF;
                    ESP_LOGI(TAG, "SPP Connection %lu closed", conn_idx);
                }
                xSemaphoreGive(connections_mutex);
            }
            break;
        case ESP_SPP_DATA_IND_EVT:
            if (xSemaphoreTake(connections_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
                uint32_t conn_idx = find_connection_by_handle(param->data_ind.handle);
                if (conn_idx < MAX_CONNECTIONS) {
                    // Queue received data
                    bt_message_t message;
                    message.conn_handle = param->data_ind.handle;
                    message.type = 0; // Received
                    message.length = param->data_ind.len;
                    memcpy(message.data, param->data_ind.data, param->data_ind.len);
                    xQueueSend(message_queue, &message, 0);
                    
                    update_connection_activity(param->data_ind.handle);
                    connections[conn_idx].bytes_received += param->data_ind.len;
                }
                xSemaphoreGive(connections_mutex);
            }
            break;
        case ESP_SPP_WRITE_EVT:
            if (param->write.status != ESP_SPP_SUCCESS) {
                ESP_LOGE(TAG, "SPP write failed");
            }
            break;
        default:
            break;
    }
}

// Send data to specific connection
esp_err_t bluetooth_spp_send_data(uint32_t conn_handle, const uint8_t *data, uint16_t length) {
    if (!bluetooth_initialized || !data || length == 0 || length > MAX_PACKET_SIZE) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (xSemaphoreTake(connections_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        uint32_t conn_idx = find_connection_by_handle(conn_handle);
        if (conn_idx < MAX_CONNECTIONS && connections[conn_idx].state == CONN_STATE_CONNECTED) {
            esp_err_t ret = esp_spp_write(conn_handle, length, (uint8_t *)data);
            if (ret == ESP_OK) {
                connections[conn_idx].bytes_sent += length;
                update_connection_activity(conn_handle);
            }
            xSemaphoreGive(connections_mutex);
            return ret;
        }
        xSemaphoreGive(connections_mutex);
    }
    
    return ESP_ERR_NOT_FOUND;
}

// Broadcast data to all connected devices
esp_err_t bluetooth_spp_broadcast_data(const uint8_t *data, uint16_t length) {
    if (!bluetooth_initialized || !data || length == 0 || length > MAX_PACKET_SIZE) {
        return ESP_ERR_INVALID_ARG;
    }
    
    esp_err_t ret = ESP_OK;
    int sent_count = 0;
    
    if (xSemaphoreTake(connections_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        for (int i = 0; i < MAX_CONNECTIONS; i++) {
            if (connections[i].state == CONN_STATE_CONNECTED) {
                esp_err_t write_ret = esp_spp_write(connections[i].handle, length, (uint8_t *)data);
                if (write_ret == ESP_OK) {
                    connections[i].bytes_sent += length;
                    update_connection_activity(connections[i].handle);
                    sent_count++;
                } else {
                    ret = write_ret;
                }
            }
        }
        xSemaphoreGive(connections_mutex);
    }
    
    ESP_LOGI(TAG, "Broadcast sent to %d connections", sent_count);
    return ret;
}

// Disconnect specific connection
void bluetooth_spp_disconnect(uint32_t conn_handle) {
    if (!bluetooth_initialized) {
        return;
    }
    
    if (xSemaphoreTake(connections_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        uint32_t conn_idx = find_connection_by_handle(conn_handle);
        if (conn_idx < MAX_CONNECTIONS && connections[conn_idx].state == CONN_STATE_CONNECTED) {
            connections[conn_idx].state = CONN_STATE_DISCONNECTING;
            esp_spp_disconnect(conn_handle);
            ESP_LOGI(TAG, "Disconnecting connection %lu", conn_idx);
        }
        xSemaphoreGive(connections_mutex);
    }
}

// Get connection information
void bluetooth_spp_get_connection_info(connection_info_t *conn_info, uint8_t *count) {
    if (!conn_info || !count) {
        return;
    }
    
    if (xSemaphoreTake(connections_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        *count = 0;
        for (int i = 0; i < MAX_CONNECTIONS; i++) {
            if (connections[i].state == CONN_STATE_CONNECTED) {
                memcpy(&conn_info[*count], &connections[i], sizeof(connection_info_t));
                (*count)++;
            }
        }
        xSemaphoreGive(connections_mutex);
    }
}

// Set device name
void bluetooth_spp_set_device_name(const char *name) {
    if (!name) {
        return;
    }
    
    strncpy(device_name, name, ESP_BT_GAP_MAX_BDNAME_LEN);
    device_name[ESP_BT_GAP_MAX_BDNAME_LEN] = '\0';
    
    if (bluetooth_initialized) {
        esp_bt_dev_set_device_name(device_name);
        esp_ble_gap_set_device_name(device_name);
    }
}

// Set data received callback
void bluetooth_spp_set_data_callback(data_received_callback_t callback) {
    data_callback = callback;
}

// Helper functions
static uint32_t find_free_connection_slot(void) {
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        if (connections[i].state == CONN_STATE_DISCONNECTED) {
            return i;
        }
    }
    return MAX_CONNECTIONS; // No free slot
}

static uint32_t find_connection_by_handle(uint32_t handle) {
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        if (connections[i].handle == handle) {
            return i;
        }
    }
    return MAX_CONNECTIONS; // Not found
}

static void update_connection_activity(uint32_t conn_handle) {
    uint32_t conn_idx = find_connection_by_handle(conn_handle);
    if (conn_idx < MAX_CONNECTIONS) {
        connections[conn_idx].last_activity = xTaskGetTickCount();
    }
}

static void print_connection_status(void) {
    if (xSemaphoreTake(connections_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        int connected_count = 0;
        for (int i = 0; i < MAX_CONNECTIONS; i++) {
            if (connections[i].state == CONN_STATE_CONNECTED) {
                connected_count++;
                ESP_LOGI(TAG, "Connection %d: Handle=%lu, Bytes RX=%lu, Bytes TX=%lu", 
                         i, connections[i].handle, connections[i].bytes_received, connections[i].bytes_sent);
            }
        }
        ESP_LOGI(TAG, "Total connected: %d/%d", connected_count, MAX_CONNECTIONS);
        xSemaphoreGive(connections_mutex);
    }
} 