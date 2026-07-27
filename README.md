# ESP32_WoL_IP_Fordwarder

ESP32 firmware built with **ESP-IDF v6.0.2**. Connects to Wi-Fi, serves an
embedded web dashboard with real-time device status and Wake-on-LAN
configuration (WIP), and exposes a JSON REST API. Credentials are read from
an embedded `.env` file — no recompile needed for Wi-Fi or port changes.

## Screenshot

```
 ┌──────────┬─────────────────────────────────────────┐
 │ ESP32 WoL│ Device Status                            │
 │ IP Fwd   │                                          │
 │          │ ┌─ Network ────────────────────────────┐ │
 │ ■ Status │ │ Wi-Fi    Connected      LAN IP  .1.9 │ │
 │ ★ WoL Cfg│ │ MAC     3C:8A:1F:A3:D2:74            │ │
 │          │ └──────────────────────────────────────┘ │
 │          │ ┌─ Memory ─────────────────────────────┐ │
 │          │ │ Heap Free   212 kB   Heap Total 320 kB│ │
 │          │ │ Min Free    198 kB   Free Stack  12 kB│ │
 │          │ │ Tasks   14          Uptime    2m 34s  │ │
 │          │ └──────────────────────────────────────┘ │
 │          │ ┌─ Device ─┐ ┌─ Firmware ─────────────┐ │
 │  .1.9    │ │ ESP32    │ │ ESP32_WoL_IP_Fordwarder │ │
 │          │ │ 2 cores  │ │ 55a178f-dirty           │ │
 └──────────┴─────────────────────────────────────────┘
```

## Hardware

- **Target**: ESP32 (xtensa)
- **Flash**: 2 MB (single factory partition)
- **Framework**: ESP-IDF v6.0.2

## Project structure

```
├── CMakeLists.txt
├── sdkconfig
├── managed_components/     # (gitignored)
├── dependencies.lock
├── .env.example
└── main/
    ├── CMakeLists.txt       # EMBED_FILES + objcopy for .env
    ├── app.c                # Entry point
    ├── .env                 # Wi-Fi credentials (gitignored)
    ├── web/
    │   ├── index.html       # Sidebar + tabbed dashboard
    │   ├── style.css        # Dark theme, responsive sidebar
    │   └── app.js           # Tab switching, status polling
    └── utils/
        ├── dotenv.h / .c    # Embedded .env parser
        ├── wifi_connect.h/c # Wi-Fi STA lifecycle
        ├── web_util.h / .c  # HTTP server lifecycle
        └── web_API.h / .c   # REST endpoints + cJSON
```

## Quick start

### Configure

```bash
cp main/.env.example main/.env
```

Edit `main/.env`:

| Key | Required | Default | Description |
|-----|----------|---------|-------------|
| `SSID_WIFI` | yes | — | Wi-Fi SSID |
| `PASSWD_WIFI` | yes | — | Wi-Fi password |
| `APP_NAME` | no | `ESP32_WoL` | Logged at boot |
| `WEB_PORT` | no | `80` | HTTP port |

### Build & flash

```bash
idf.py set-target esp32
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```

## Web interface

The dashboard has a left sidebar with two tabs:

### Status

Displays real-time ESP32 metadata fetched from `/api/status` every 5 seconds:

| Section | Fields |
|---------|--------|
| Network | Wi-Fi state, LAN IP, MAC address |
| Memory | Heap free/total/min, free stack, task count, uptime |
| Device | Chip model, cores, revision, CPU freq, flash size, features |
| Firmware | App name, version, build date |

### WoL Config

Placeholder panel with UI skeletons for:
- Target device list (MAC addresses to wake)
- IP forwarding rules
- Broadcast settings (UDP port, broadcast IP, interface)

All inputs are currently disabled — backend not implemented.

## API Endpoints

| Method | URI | Response |
|--------|-----|----------|
| `GET` | `/` | Dashboard HTML |
| `GET` | `/index.html` | Dashboard HTML |
| `GET` | `/style.css` | Stylesheet |
| `GET` | `/app.js` | Client JavaScript |
| `GET` | `/api/status` | JSON with full device metadata |

Static assets are embedded in flash — no filesystem or SD card.

### `/api/status` response

```json
{
  "wifi": true,
  "ip": "192.168.1.9",
  "mac": "3C:8A:1F:A3:D2:74",
  "heap_free": 215460,
  "heap_min_free": 198216,
  "heap_total": 327680,
  "free_stack": 12300,
  "task_count": 14,
  "uptime": 154,
  "chip_model": "ESP32",
  "chip_cores": 2,
  "chip_revision": 3,
  "cpu_freq": 160000000,
  "flash_size": 4194304,
  "chip_features": ["Embedded Flash","Wi-Fi b/g/n","BT","BLE"],
  "app_name": "ESP32_WoL_IP_Fordwarder",
  "app_version": "55a178f-dirty",
  "app_date": "Jul 26 2026",
  "app_time": "18:39:49"
}
```

## Dependencies

[cJSON](https://github.com/DaveGamble/cJSON) via Espressif component registry.

```bash
idf.py add-dependency "namespace/name^1"
```

Lock file `dependencies.lock` is committed. `managed_components/` is gitignored.

## idf.py

| Command | Description |
|---------|-------------|
| `idf.py set-target esp32` | Configure for ESP32 (run once) |
| `idf.py build` | Compile |
| `idf.py -p PORT flash` | Flash binary |
| `idf.py -p PORT flash monitor` | Flash + serial monitor |
| `idf.py monitor` | Serial monitor only |
| `idf.py menuconfig` | Kconfig editor |
| `idf.py clean` | Delete build artifacts |
| `idf.py fullclean` | Delete everything, including config |
| `idf.py add-dependency "x"` | Add component dependency |
| `idf.py reconfigure` | Re-run CMake |

See [AGENTS.md](AGENTS.md) for where `idf.py` is available on this machine.
