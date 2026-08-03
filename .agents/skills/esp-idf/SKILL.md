---
name: esp-idf
description: "Trigger: ESP-IDF, ESP32, espressif, idf.py, FreeRTOS ESP, embedded C, xtensa, GPIO config, Wi-Fi station, NVS flash, SPI flash, I2C/SPI/UART driver, OTA update, BLE, NimBLE, unit test, Unity test, CMock. Programming API and testing for ESP-IDF v6.0.2 on ESP32."
license: Apache-2.0
metadata:
  author: gentleman-programming
  version: "1.1"
---

## Activation Contract

Load this skill when reading, writing, or reviewing ESP-IDF v6.0.2 C code for ESP32. Also load when the user asks about building, flashing, Kconfig, component CMake, peripheral drivers, FreeRTOS tasks, ESP-IDF project structure, unit testing with Unity, test files, test CMake, CMock, or running tests on-device.

## Hard Rules

- **Header convention**: `#include "driver/gpio.h"` for driver APIs; `#include "esp_xxx.h"` for system APIs. Check the exact component name before including.
- **CMakeLists.txt**: Every component declares `idf_component_register(SRCS "..." REQUIRES esp_driver_gpio)` (or `PRIV_REQUIRES`). Main project must include `project(my_project)` and the common boilerplate.
- **Error handling**: Always check `esp_err_t` return values against `ESP_OK`. Return `ESP_FAIL` or propagate. Never ignore a driver init error.
- **FreeRTOS**: Use `xTaskCreate` / `xTaskCreatePinnedToCore` for tasks. Stack sizes are in *words* (4 bytes), not bytes. Minimum stack ~2048 words for simple tasks, 4096+ for Wi-Fi or complex work.
- **ISR handlers**: `IRAM_ATTR` required for interrupt service routines. Keep ISR code minimal — defer work to a task via queue/semaphore.
- **NVS**: Always call `nvs_flash_init()` before using NVS. On first boot, erase with `nvs_flash_erase()`.
- **Wi-Fi + BLE coexistence**: ADC2 pins are unavailable when Wi-Fi is active. Use ADC1 instead.
- **Kconfig**: Run `idf.py menuconfig` before building when changing sdkconfig options. The alias `idf.py` wraps `activate_idf_v6.0.2.sh`.
- **Unit test files**: Place in a `test/` subdirectory of the component. Files must start with `test` (e.g. `test_dotenv.c`). Include `unity.h` and the module header.
- **Test CMakeLists.txt**: Every `test/` dir is a component requiring `unity`. Use `idf_component_register(SRC_DIRS "." INCLUDE_DIRS "." REQUIRES unity)`.
- **Assertion semicolons**: `TEST_ASSERT_*` macros require a trailing `;` (required since v5.3). Missing semicolons cause compilation errors.
- **No main function**: Do NOT add `UNITY_BEGIN()`/`UNITY_END()` — `unity_platform.c` handles this. Just write `TEST_CASE` macros.

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
| Unit test (Unity) | `unity.h` — `TEST_CASE(name, "[tag]")`, `TEST_ASSERT`, `TEST_ASSERT_EQUAL`, `TEST_ASSERT_TRUE`, `TEST_ASSERT_NULL` |
| Multi-device test | `TEST_CASE_MULTIPLE_DEVICES(name, "[tag]", func1, func2, ...)` — up to 5 DUT functions |
| Multi-stage test | `TEST_CASE_MULTIPLE_STAGES(name, "[tag]", stage1, stage2)` — for reset-aware tests |
| Test runner Kconfig | `CONFIG_UNITY_ENABLE_IDF_TEST_RUNNER` — enables TEST_CASE macro and interactive menu (default y) |
| Mocks (CMock, host only) | `idf_component_mock()` in CMake + `cmock/` component — requires Ruby, Linux/macOS host |

## Execution Steps

1. Source the environment: `source /home/javastral/.espressif/tools/activate_idf_v6.0.2.sh` (or use the `idf.py` alias which does this automatically).
2. Set the target: `idf.py set-target esp32`.
3. Configure Kconfig: `idf.py menuconfig`.
4. Build: `idf.py build`.
5. Flash (replace PORT): `idf.py -p /dev/ttyUSB0 flash`.
6. Monitor: `idf.py -p /dev/ttyUSB0 monitor`.
7. Clean build: `idf.py fullclean` then rebuild.

## Testing

### Test file structure

```c
// component/test/test_foo.c
#include "unity.h"
#include "foo.h"

TEST_CASE("foo_init returns ESP_OK with valid config", "[foo]")
{
    foo_config_t cfg = { .param = 42 };
    TEST_ASSERT_EQUAL(ESP_OK, foo_init(&cfg));
}

TEST_CASE("foo_init returns ESP_ERR_INVALID_ARG on NULL config", "[foo]")
{
    TEST_ASSERT(ESP_ERR_INVALID_ARG == foo_init(NULL));
}
```

```cmake
# component/test/CMakeLists.txt
idf_component_register(SRC_DIRS "."
                       INCLUDE_DIRS "."
                       REQUIRES unity)
```

### Running tests

Tests run on-device via a test app. Two approaches:

1. **Embedded test component**: Add the test component to the project's root `CMakeLists.txt` via `set(EXTRA_COMPONENT_DIRS "component/test")`, build, flash, and interact through the serial console menu.
2. **Standalone test app**: Use `idf.py create-project-from-example espressif/unit-test-app:unit-test-app`, add test components, build, flash.

After flashing, press Enter on the UART console to see the test menu. Run by name (`"test name"`), index (number), tag (`[foo]`), or `*` for all.

### Common Unity assertions

| Macro | Purpose |
|---|---|
| `TEST_ASSERT(cond)` | Boolean true |
| `TEST_ASSERT_TRUE(cond)` | Same, explicit |
| `TEST_ASSERT_FALSE(cond)` | Boolean false |
| `TEST_ASSERT_EQUAL(expected, actual)` | Integer equality |
| `TEST_ASSERT_EQUAL_INT(expected, actual)` | Explicit int |
| `TEST_ASSERT_EQUAL_STRING(expected, actual)` | String equality |
| `TEST_ASSERT_NULL(ptr)` | Pointer is NULL |
| `TEST_ASSERT_NOT_NULL(ptr)` | Pointer is not NULL |
| `TEST_ASSERT_EQUAL_FLOAT(expected, actual)` | Float equality (only if `CONFIG_UNITY_ENABLE_FLOAT=y`) |

### Mocks (advanced, Linux host only)

ESP-IDF integrates CMock via `idf_component_mock()`. Only works on Linux/POSIX host target. See [api-guides/unit-tests.html#mocks](https://docs.espressif.com/projects/esp-idf/en/v6.0.2/esp32/api-guides/unit-tests.html#mocks) for mock component setup. Requires Ruby.

## Output Contract

When generating ESP-IDF code, always produce:
- Correct `#include` lines with the v6.0.2 component name.
- A `CMakeLists.txt` with proper `REQUIRES`/`PRIV_REQUIRES` entries.
- Error checking on every `esp_err_t` return.
- `app_main()` as the entry point (not `main()`).
- Appropriate log tags via `static const char *TAG = "component_name"`.

When generating tests, also produce:
- `test_<module>.c` files in `test/` with `unity.h` and the module header.
- A `test/CMakeLists.txt` requiring `unity`.
- `TEST_CASE` macros with descriptive names and `[module]` tags.
- Assertion macros with trailing semicolons.

## References

- [references/docs.md](references/docs.md) — ESP-IDF v6.0.2 documentation index and key API reference links.
