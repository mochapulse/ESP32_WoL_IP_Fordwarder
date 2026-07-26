# ESP32_WoL_IP_Fordwarder

ESP32 Wi-Fi station example built with **ESP-IDF v6.0.2**. Connects to a WPA2-PSK
access point using credentials from an embedded `.env` file, then prints and
monitors the assigned LAN IP.

## Hardware

- **Target**: ESP32 (xtensa)
- **Framework**: ESP-IDF v6.0.2

## Project structure

```
main/
├── CMakeLists.txt          # Component registration + .env embedding
├── app.c         # Application entry point (app_main)
├── .env.example            # Template — copy to .env
└── utils/
    ├── dotenv.h / .c        # Embedded .env loader (up to 16 keys)
    └── wifi_connect.h / .c  # WiFi STA: init, connect, disconnect, health, LAN IP
```

## Quick start

### 1. Configure Wi-Fi credentials

```bash
cp main/.env.example main/.env
```

Edit `main/.env`:

```
SSID_WIFI=your-network
PASSWD_WIFI=your-password
```

### 2. Build

```bash
idf.py set-target esp32
idf.py build
```

### 3. Flash & monitor

```bash
idf.py -p /dev/ttyUSB0 flash monitor
```

## Utils library

### `dotenv`

Loads a `.env` file embedded at link time via `objcopy`.

```c
dotenv_init();
const char *val = dotenv_get("KEY");
```

| Limit | Value |
|-------|-------|
| Max keys | 16 |
| Key length | 32 |
| Value length | 64 |

### `wifi_connect`

Wi-Fi station wrapper with event-driven state tracking.

```c
wifi_init("SSID", "password");   // Init NVS, netif, WiFi, register handlers
wifi_connect();                  // Start WiFi (auto-connects on STA_START)
const char *ip = check_LAN_ip(); // Block until connected, return IP string
wifi_health();                   // true if connected and has IP
wifi_disconnect();               // Disconnect and stop
```
