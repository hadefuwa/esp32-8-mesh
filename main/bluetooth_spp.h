#ifndef BLUETOOTH_SPP_H
#define BLUETOOTH_SPP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"
#include "esp_spp_api.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_defs.h"
#include "esp_gatt_common_api.h"

// Configuration
#define MAX_CONNECTIONS 8
#define MAX_PACKET_SIZE 512
#define DEVICE_NAME "ESP32_Multi_SPP"
#define BLE_DEVICE_NAME "ESP32_Multi_BLE"

// Connection states
typedef enum {
    CONN_STATE_DISCONNECTED = 0,
    CONN_STATE_CONNECTING,
    CONN_STATE_CONNECTED,
    CONN_STATE_DISCONNECTING
} connection_state_t;

// Connection info structure
typedef struct {
    uint32_t handle;
    connection_state_t state;
    esp_bd_addr_t remote_addr;
    char remote_name[ESP_BT_GAP_MAX_BDNAME_LEN + 1];
    uint32_t bytes_received;
    uint32_t bytes_sent;
    uint32_t last_activity;
} connection_info_t;

// Message structure for inter-task communication
typedef struct {
    uint32_t conn_handle;
    uint8_t data[MAX_PACKET_SIZE];
    uint16_t length;
    uint8_t type; // 0 = received, 1 = to send
} bt_message_t;

// Function declarations
void bluetooth_spp_init(void);
esp_err_t bluetooth_spp_send_data(uint32_t conn_handle, const uint8_t *data, uint16_t length);
esp_err_t bluetooth_spp_broadcast_data(const uint8_t *data, uint16_t length);
void bluetooth_spp_disconnect(uint32_t conn_handle);
void bluetooth_spp_get_connection_info(connection_info_t *conn_info, uint8_t *count);
void bluetooth_spp_set_device_name(const char *name);

// Callback function type for received data
typedef void (*data_received_callback_t)(uint32_t conn_handle, const uint8_t *data, uint16_t length);
void bluetooth_spp_set_data_callback(data_received_callback_t callback);

#endif // BLUETOOTH_SPP_H 