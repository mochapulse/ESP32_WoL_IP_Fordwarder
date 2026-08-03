# ESP32 WoL LAN Controller    {#mainpage}

**Embedded firmware for ESP32** — Wi-Fi station, HTTP dashboard, Wake-on-LAN.

## What it does

- Connects to a WPA2-PSK Wi-Fi network with credentials from an embedded `.env`
- Serves a responsive web dashboard with **real-time device status** (chip info, memory, firmware) and a **WoL configuration panel** (placeholder)
- Exposes a JSON REST API at `/api/status`
- Retries Wi-Fi automatically on disconnect

## API reference

| Endpoint | Method | Description |
|----------|--------|-------------|
| `/` | `GET` | Dashboard HTML (sidebar + tabbed layout) |
| `/style.css` | `GET` | Dark-theme responsive stylesheet |
| `/app.js` | `GET` | Client-side tab switching + status polling |
| `/api/status` | `GET` | JSON: Wi-Fi state, IP, MAC, heap, chip info, firmware |

> All static assets are embedded in flash via `EMBED_FILES` — no SPIFFS or SD card.

## Architecture

```
app.c          — entry point, watchdog loop
utils/
  dotenv       — embedded .env parser (16 keys, objcopy blob)
  wifi_connect — Wi-Fi STA lifecycle (event group, retry guard)
  web_util     — HTTP server start/stop/health
  web_API      — endpoint registration + embedded file serving + JSON status
```

## Build & flash

```bash
cp main/.env.example main/.env   # set SSID_WIFI + PASSWD_WIFI
idf.py set-target esp32
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```

## Links

- [GitHub repository](https://github.com/mochapulse/lan-controller-esp32)
- [ESP-IDF v6.0.2 docs](https://docs.espressif.com/projects/esp-idf/en/v6.0.2/esp32/)

---

*Served from flash — no filesystem needed.*
