ESP32 Bluetooth Multi-Connection Example – App Plan (Revised)

1. Objective
Develop an ESP32 application (using ESP-IDF) demonstrating:
- Bluetooth Mesh networking (BLE Mesh) and/or
- Serial-over-Bluetooth (CDC/SPP-like) communication
- Support for up to 8 concurrent connections

2. Requirements
Hardware:
- ESP32 DevKit (or compatible)

Software:
- ESP-IDF (latest stable)
- Python (for ESP-IDF tools)
- VS Code + PlatformIO or ESP-IDF extension

Bluetooth:
- Mesh: ESP-BLE-MESH component
- CDC/SPP: Classic Bluetooth SPP (for Android/PC) or BLE UART-like service (for iOS compatibility)

3. Key Features
- Mode selection: Mesh or CDC/SPP (selectable at compile time or runtime)
- Multi-connection: Device can accept/connect to up to 8 peers
- Bidirectional messaging: Each connection can send and receive text/data messages
- Status indication: Serial output and/or LEDs for connection/activity status
- Serial console: For debugging, status, and command input

4. System Architecture
- Main app:
  - Initializes Bluetooth (BLE Mesh stack or SPP/BLE UART)
  - Handles connection events (connect/disconnect)
  - Tracks list of connected devices (up to 8)
  - For mesh: Publishes/subscribes to group messages
  - For CDC/SPP: Receives/sends serial-like data to each peer

5. Tasks Breakdown
A. Bluetooth Mesh Mode
- Initialize BLE Mesh (node or provisioner)
- Join or create mesh network
- Send/receive mesh messages (relay to UART/serial)
- Manage node addresses (up to 8)
- Mesh provisioning and configuration steps for new nodes

B. CDC/SPP Mode
- Initialize Bluetooth Classic SPP (for Android/PC) or BLE UART service (for iOS)
- Accept up to 8 connections
- Implement simple text protocol (e.g., newline-terminated)
- Broadcast/relay messages between clients (optional)
- Connection timeout and error handling for robustness

C. User Interface
- Serial console for status, debugging, switching mode
- Optional: LEDs for connection/activity indication
- Button input for mode switching or reset

D. Configuration
- Build flag or serial command to switch between Mesh/CDC
- Persistent storage (NVS) for saving last used mode

E. Example Usage
- Documentation for connecting from PC/phone
- Example scripts for connecting and sending data
- Troubleshooting section for common issues

6. Deliverables
- main.c or app_main.c with core logic
- bluetooth_mesh.c / bluetooth_spp.c with mode-specific code
- README.md with setup, usage, and testing instructions
- CMakeLists.txt and sdkconfig.defaults for reproducible builds

7. Stretch Goals
- Web-based dashboard (using ESP32 web server)
- OTA updates
- Secure pairing/authentication
- Unit tests for protocol handling

