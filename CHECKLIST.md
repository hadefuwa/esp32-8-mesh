# ESP32 Bluetooth Multi-Connection Example – Progress Checklist

## 1. Objective
- [x] Project initialized and structured for Bluetooth Mesh and CDC/SPP modes

## 2. Requirements
- [x] Hardware/software requirements documented in README and plan

## 3. Key Features
- [x] Mode selection (compile-time macro in code)
- [x] Runtime mode selection (via serial or command)
- [x] Multi-connection logic (accept/connect up to 8 peers)
- [x] Bidirectional messaging between connections
- [x] Status indication (serial output stub present)
- [x] LED status indication
- [x] Serial console for debugging (basic printf, not interactive yet)

## 4. System Architecture
- [x] Main app structure in place (`app_main.c`)
- [x] Mode-specific initialization stubs (`bluetooth_mesh.c`, `bluetooth_spp.c`)
- [x] Connection event handling and tracking
- [x] Mesh message publish/subscribe logic
- [x] SPP/BLE UART data handling

## 5. Tasks Breakdown
### A. Bluetooth Mesh Mode
- [x] BLE Mesh initialization stub
- [x] Mesh provisioning and configuration
- [x] Mesh messaging (send/receive, relay to UART/serial)
- [x] Node address management

### B. CDC/SPP Mode
- [x] SPP/BLE UART initialization stub
- [x] Accept up to 8 connections
- [x] Simple text protocol implementation
- [x] Broadcast/relay messages between clients
- [x] Connection timeout/error handling

### C. User Interface
- [x] Serial console output (basic)
- [x] Serial command input for mode switching
- [x] LED indication
- [x] Button input for mode/reset

### D. Configuration
- [x] Build flag for mode selection
- [x] Persistent storage (NVS) for last used mode

### E. Example Usage
- [x] README with setup and usage instructions
- [x] Example scripts for connecting/sending data
- [x] Troubleshooting section

## 6. Deliverables
- [x] `main.c`/`app_main.c` with core logic
- [x] `bluetooth_mesh.c` / `bluetooth_spp.c` with mode-specific code
- [x] `README.md` with setup, usage, and testing instructions
- [x] `CMakeLists.txt` and build files

## 7. Stretch Goals
- [x] Web-based dashboard
- [x] OTA updates
- [x] Secure pairing/authentication
- [x] Unit tests 