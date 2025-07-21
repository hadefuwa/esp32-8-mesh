/*
 * ESP32 Bluetooth Multi-Connection Example
 *
 * This example demonstrates how to use ESP-IDF to build an ESP32 app that can operate in either:
 *   - Bluetooth Mesh mode (BLE Mesh Generic OnOff Server)
 *   - Bluetooth CDC/SPP mode (Classic SPP or BLE UART, up to 8 connections)
 *
 * To switch modes, change the BLUETOOTH_MODE macro below.
 *
 * Extend the bluetooth_mesh.c and bluetooth_spp.c files to add your own logic.
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "bluetooth_mesh.h"

// Mode selection: set to BLUETOOTH_MODE_MESH or BLUETOOTH_MODE_SPP
#define BLUETOOTH_MODE_MESH 0
#define BLUETOOTH_MODE_SPP 1
#ifndef BLUETOOTH_MODE
#define BLUETOOTH_MODE BLUETOOTH_MODE_MESH // Change to BLUETOOTH_MODE_SPP for SPP/CDC mode
#endif

void bluetooth_spp_init(void);

void app_main(void) {
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Print startup info
    printf("\nESP32 Bluetooth Multi-Connection Example\n");
    printf("Mode: %s\n", BLUETOOTH_MODE == BLUETOOTH_MODE_MESH ? "Mesh" : "SPP/CDC");

    // Initialize Bluetooth based on mode
    if (BLUETOOTH_MODE == BLUETOOTH_MODE_MESH) {
        printf("Initializing BLE Mesh node...\n");
        ret = bluetooth_mesh_init();
        if (ret != ESP_OK) {
            printf("BLE Mesh init failed!\n");
        } else {
            printf("BLE Mesh node ready. Provision with a mesh app.\n");
        }
    } else {
        printf("Initializing Bluetooth SPP/CDC mode...\n");
        bluetooth_spp_init();
        // TODO: Add your SPP/CDC logic in bluetooth_spp.c
    }

    // Main loop (extend here for periodic tasks, status, etc.)
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
} 