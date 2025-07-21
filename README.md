# ESP32 Bluetooth Multi-Connection Example

## Overview
This project demonstrates an ESP32 application (using ESP-IDF) that supports:
- Bluetooth Mesh networking (BLE Mesh)
- Serial-over-Bluetooth (CDC/SPP-like) communication
- Up to 8 concurrent connections

The project is designed for learning, prototyping, and extending Bluetooth multi-connection capabilities on ESP32 devices.

## Features
- **Mode selection:** Mesh or CDC/SPP (selectable at compile time or runtime)
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

## Setup Instructions
1. **Clone the repository:**
   ```sh
   git clone https://github.com/hadefuwa/esp32-8-mesh.git
   cd esp32-8-mesh
   ```
2. **Install ESP-IDF:**
   - Follow the [official ESP-IDF setup guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/).
3. **Configure the project:**
   - Use `idf.py menuconfig` to select Bluetooth mode and other options.
4. **Build and flash:**
   ```sh
   idf.py build
   idf.py -p <PORT> flash
   ```
5. **Monitor output:**
   ```sh
   idf.py -p <PORT> monitor
   ```

## Usage
- **Mode selection:** Choose Mesh or CDC/SPP at build time or via serial command (if supported).
- **Connecting devices:**
  - For Mesh: Use ESP-BLE-MESH compatible apps or other ESP32 nodes.
  - For CDC/SPP: Connect from Android/PC (SPP) or iOS (BLE UART).
- **Messaging:**
  - Send/receive text messages between connected devices.
- **Status:**
  - Monitor serial output or LEDs for connection/activity status.

## Configuration
- **Build flags:** Set via `menuconfig` or `sdkconfig.defaults`.
- **Persistent mode:** Last used mode can be saved in NVS (non-volatile storage).
- **Serial commands:** Use the serial console to switch modes or debug.

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