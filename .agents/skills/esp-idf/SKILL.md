---
name: esp-idf
description: "Trigger: ESP-IDF, ESP32, espressif, idf.py, FreeRTOS ESP, embedded C, xtensa, GPIO config, Wi-Fi station, NVS flash, SPI flash, I2C/SPI/UART driver, OTA update, BLE, NimBLE. Programming API for ESP-IDF v6.0.2 on ESP32."
license: Apache-2.0
metadata:
  author: gentleman-programming
  version: "1.0"
---

## Activation Contract

Load this skill when reading, writing, or reviewing ESP-IDF v6.0.2 C code for ESP32. Also load when the user asks about building, flashing, Kconfig, component CMake, peripheral drivers, FreeRTOS tasks, or ESP-IDF project structure.

## Hard Rules

- **Header convention**: `#include "driver/gpio.h"` for driver APIs; `#include "esp_xxx.h"` for system APIs. Check the exact component name before including.
- **CMakeLists.txt**: Every component declares `idf_component_register(SRCS "..." REQUIRES esp_driver_gpio)` (or `PRIV_REQUIRES`). Main project must include `project(my_project)` and the common boilerplate.
- **Error handling**: Always check `esp_err_t` return values against `ESP_OK`. Return `ESP_FAIL` or propagate. Never ignore a driver init error.
- **FreeRTOS**: Use `xTaskCreate` / `xTaskCreatePinnedToCore` for tasks. Stack sizes are in *words* (4 bytes), not bytes. Minimum stack ~2048 words for simple tasks, 4096+ for Wi-Fi or complex work.
- **ISR handlers**: `IRAM_ATTR` required for interrupt service routines. Keep ISR code minimal — defer work to a task via queue/semaphore.
- **NVS**: Always call `nvs_flash_init()` before using NVS. On first boot, erase with `nvs_flash_erase()`.
- **Wi-Fi + BLE coexistence**: ADC2 pins are unavailable when Wi-Fi is active. Use ADC1 instead.
- **Kconfig**: Run `idf.py menuconfig` before building when changing sdkconfig options. The alias `idf.py` wraps `activate_idf_v6.0.2.sh`.

## Decision Gates

| Need | API / Header |
|---|---|
| GPIO input/output | `driver/gpio.h` — `gpio_config()`, `gpio_set_level()`, `gpio_get_level()` |
| GPIO interrupts per pin | `gpio_install_isr_service()` + `gpio_isr_handler_add()` |
| UART | `driver/uart.h` — `uart_driver_install()`, `uart_write_bytes()` |
| I2C master | `driver/i2c_master.h` (v6.0.2+ new driver) or legacy `driver/i2c.h` |
| SPI master | `driver/spi_master.h` — `spi_bus_initialize()`, `spi_device_transmit()` |
| ADC oneshot | `esp_adc/adc_oneshot.h` |
| Timer (hardware) | `driver/gptimer.h` |
| Software timer / high-res | `esp_timer.h` |
| Wi-Fi station | `esp_wifi.h` + `esp_netif.h` + `nvs_flash.h` |
| HTTP client | `esp_http_client.h` |
| MQTT | `mqtt_client.h` |
| OTA update | `esp_https_ota.h` |
| BLE (Bluedroid) | `esp_bt.h` + `esp_gap_ble_api.h` |
| BLE (NimBLE) | `nimble/` component, lighter stack |
| Non-volatile storage | `nvs_flash.h` |
| FAT/SPIFFS filesystem | `esp_vfs_fat.h` / `esp_spiffs.h` |
| Deep sleep | `esp_sleep.h` + RTC GPIO via `driver/rtc_io.h` |
| Logging | `esp_log.h` — `ESP_LOGI`, `ESP_LOGE`, `ESP_LOGW`, `ESP_LOGD` |
| Console / REPL | `esp_console.h` |
| Event loop | `esp_event.h` |

## Execution Steps

1. Source the environment: `source /home/javastral/.espressif/tools/activate_idf_v6.0.2.sh` (or use the `idf.py` alias which does this automatically).
2. Set the target: `idf.py set-target esp32`.
3. Configure Kconfig: `idf.py menuconfig`.
4. Build: `idf.py build`.
5. Flash (replace PORT): `idf.py -p /dev/ttyUSB0 flash`.
6. Monitor: `idf.py -p /dev/ttyUSB0 monitor`.
7. Clean build: `idf.py fullclean` then rebuild.

## Output Contract

When generating ESP-IDF code, always produce:
- Correct `#include` lines with the v6.0.2 component name.
- A `CMakeLists.txt` with proper `REQUIRES`/`PRIV_REQUIRES` entries.
- Error checking on every `esp_err_t` return.
- `app_main()` as the entry point (not `main()`).
- Appropriate log tags via `static const char *TAG = "component_name"`.

Report files created, headers used, and any Kconfig options assumed.

## References

- [references/docs.md](references/docs.md) — ESP-IDF v6.0.2 documentation index and key API reference links.
