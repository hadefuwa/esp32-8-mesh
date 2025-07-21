#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "esp_log.h"

// Mode selection: define BLUETOOTH_MODE_MESH or BLUETOOTH_MODE_SPP
#define BLUETOOTH_MODE_MESH 0
#define BLUETOOTH_MODE_SPP 1
#ifndef BLUETOOTH_MODE
#define BLUETOOTH_MODE BLUETOOTH_MODE_MESH // Change to BLUETOOTH_MODE_SPP for SPP mode
#endif

void bluetooth_mesh_init(void);
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
    printf("Mode: %s\n", BLUETOOTH_MODE == BLUETOOTH_MODE_MESH ? "Mesh" : "SPP/BLE UART");

    // Initialize Bluetooth based on mode
    if (BLUETOOTH_MODE == BLUETOOTH_MODE_MESH) {
        bluetooth_mesh_init();
    } else {
        bluetooth_spp_init();
    }

    // Main loop (can be expanded for serial console, status, etc.)
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
} 