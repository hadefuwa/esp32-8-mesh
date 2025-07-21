# ESP32 Bluetooth Multi-Connection Example

## Overview
This project demonstrates an ESP32 application (using ESP-IDF) that supports:
- Bluetooth Mesh networking (BLE Mesh)
- Serial-over-Bluetooth (CDC/SPP-like) communication
- Up to 8 concurrent connections

The project is designed for learning, prototyping, and extending Bluetooth multi-connection capabilities on ESP32 devices.

## Features
- **Mode selection:** Mesh or CDC/SPP (selectable at compile time)
- **Multi-connection:** Accept/connect to up to 8 peers
- **Bidirectional messaging:** Send and receive text/data messages
- **Status indication:** Serial output and/or LEDs
- **Serial console:** For debugging, status, and command input

## Requirements
### Hardware
- ESP32 DevKit (or compatible)

### Software
- ESP-IDF (latest stable)
- Python (for ESP-IDF tools)
- VS Code + PlatformIO or ESP-IDF extension

### Bluetooth
- Mesh: ESP-BLE-MESH component
- CDC/SPP: Classic Bluetooth SPP (for Android/PC) or BLE UART-like service (for iOS)

## How to Use This Code

### 1. Clone the Repository
```sh
git clone https://github.com/hadefuwa/esp32-8-mesh.git
cd esp32-8-mesh
```

### 2. Select Bluetooth Mode (Mesh or CDC/SPP)
- Open `main/app_main.c`.
- Find the following lines near the top:
  ```c
  #define BLUETOOTH_MODE_MESH 0
  #define BLUETOOTH_MODE_SPP 1
  #ifndef BLUETOOTH_MODE
  #define BLUETOOTH_MODE BLUETOOTH_MODE_MESH // Change to BLUETOOTH_MODE_SPP for SPP/CDC mode
  #endif
  ```
- To use BLE Mesh, leave as is. To use CDC/SPP, change the last line to:
  ```c
  #define BLUETOOTH_MODE BLUETOOTH_MODE_SPP
  ```
- Save the file.

### 3. Configure ESP-IDF and Enable Bluetooth Mesh (if using Mesh)
- Run:
  ```sh
  idf.py menuconfig
  ```
- Go to `Component config > Bluetooth > Bluetooth Mesh Support` and enable it.
- Set other options as needed for your board.

### 4. Build and Flash the Firmware
- Connect your ESP32 board via USB.
- Run:
  ```sh
  idf.py build
  idf.py -p <PORT> flash
  ```
  Replace `<PORT>` with your serial port (e.g., `COM3` on Windows or `/dev/ttyUSB0` on Linux).

### 5. Monitor Serial Output
- Run:
  ```sh
  idf.py -p <PORT> monitor
  ```
- You should see startup messages and status output.

### 6. Provisioning (for BLE Mesh Mode)
- Use a BLE Mesh app (e.g., nRF Mesh or Espressif's mesh tools) to provision your ESP32 into a mesh network.
- The device will print provisioning events to the serial console.
- Extend `main/bluetooth_mesh.c` to add your own mesh logic (e.g., control GPIOs, relay messages).

### 7. CDC/SPP Mode (Template)
- The SPP/CDC mode is a template for you to implement Bluetooth Classic SPP or BLE UART.
- Extend `main/bluetooth_spp.c` to handle up to 8 connections, relay messages, and add your own logic.
- Example extension points are marked with TODO comments.

### 8. Extending the Code
- **For Mesh:**
  - Add your logic in the event handlers in `main/bluetooth_mesh.c`.
  - Example: Toggle an LED when a mesh message is received.
- **For SPP/CDC:**
  - Add your connection and data handling logic in `main/bluetooth_spp.c`.
  - Example: Track connected clients, relay data, implement a simple protocol.

## Troubleshooting
- Ensure ESP-IDF and Python are correctly installed.
- Check USB drivers for ESP32.
- For Bluetooth issues, verify device compatibility and permissions.
- Use serial monitor for debug output.

## Contributing
Contributions are welcome! Please open issues or pull requests for suggestions, bug fixes, or new features.

## License
This project is licensed under the MIT License.

## References
- [ESP-IDF Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/)
- [ESP-BLE-MESH](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/bluetooth/esp_ble_mesh.html)
- [ESP32 SPP](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/bluetooth/classic_bt.html) 