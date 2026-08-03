# ESP32 WoL LAN Controller    {#mainpage}

**Embedded firmware for ESP32** — Wi-Fi station, HTTP dashboard, Wake-on-LAN.

## What it does

- Connects to a WPA2-PSK Wi-Fi network with credentials from an embedded `.env`
- Serves a responsive web dashboard with **real-time device status** (chip info, memory, firmware) and uPlot charting
- Exposes a JSON REST API at `/api/status` and `/api/wol`
- Sends Wake-on-LAN magic packets via UDP broadcast
- API key authentication (`X-API-Key` header) on all `/api/*` routes
- Returns `503 Service Unavailable` when free heap drops below the safety guard
- Retries Wi-Fi automatically on disconnect

## API reference

| Endpoint | Method | Description |
|----------|--------|-------------|
| `/` | `GET` | Dashboard HTML (sidebar + tabbed layout) |
| `/style.css` | `GET` | Dark-theme responsive stylesheet |
| `/app.js` | `GET` | Client-side tab switching + status polling + uPlot chart |
| `/api/status` | `GET` | JSON: Wi-Fi state, IP, MAC, heap, chip info, firmware |
| `/api/wol` | `GET` \| `POST` | Trigger WoL magic packet, returns `{"ok":true/false}` |

> All static assets are embedded in flash via `EMBED_FILES` — no SPIFFS or SD card.

## Architecture

```
app.c          — entry point, watchdog loop
utils/
  dotenv       — embedded .env parser (16 keys, objcopy blob)
  wifi_connect — Wi-Fi STA lifecycle (event group, retry guard)
  web_util     — HTTP server start/stop/health
  web_API      — endpoint registration, embedded file serving, JSON status + WoL
  wol          — Wake-on-LAN destination config + magic packet sender via UDP
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
