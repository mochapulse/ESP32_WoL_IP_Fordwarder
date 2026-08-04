/**
 * @file    wifi_connect.h
 * @brief   Wi-Fi station lifecycle: init, connect, disconnect, health check,
 *          retry, and IP retrieval.
 *
 * Uses a FreeRTOS event group (WIFI_GOT_IP_BIT) to track connectivity state.
 * Event handlers run in the default system event loop; all public functions
 * are safe to call from app_main() or a monitoring task.
 */

#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Initialise NVS, netif, event loop, and Wi-Fi station.
 *
 * Registers event handlers for WIFI_EVENT and IP_EVENT so that
 * wifi_connect() can be driven asynchronously. The Wi-Fi stack is left in
 * STOPPED state; call wifi_connect() to start scanning.
 *
 * @param ssid       Wi-Fi SSID (copied into wifi_config_t).
 * @param password   Wi-Fi password (copied into wifi_config_t).
 * @param static_ip  Optional fixed LAN IPv4 (dotted-decimal). When set,
 *                   the DHCP client is stopped and this address is applied
 *                   statically (gateway derived as x.y.z.1, netmask
 *                   255.255.255.0). NULL or empty → DHCP.
 * @return
 *  - ESP_OK on success.
 *  - ESP_ERR_INVALID_ARG if ssid or password is NULL.
 *  - Other esp_err_t propagated from NVS, netif, event loop, or wifi init.
 */
esp_err_t wifi_init(const char *ssid, const char *password,
                    const char *static_ip);

/**
 * @brief   Start the Wi-Fi station.
 *
 * After esp_wifi_start() succeeds, the event handler catches
 * WIFI_EVENT_STA_START and calls esp_wifi_connect() automatically.
 *
 * @return esp_err_t from esp_wifi_start().
 */
esp_err_t wifi_connect(void);

/**
 * @brief   Disconnect from AP and stop the Wi-Fi station.
 *
 * Clears the GOT_IP event bit. Safe to call when Wi-Fi is already stopped.
 *
 * @return ESP_OK on success or if already stopped.
 */
esp_err_t wifi_disconnect(void);

/**
 * @brief   Check whether the station is connected and has an IP.
 *
 * @return true if the WIFI_GOT_IP_BIT event-group bit is set.
 */
bool wifi_health(void);

/**
 * @brief   Attempt to reconnect if currently disconnected.
 *
 * Idempotent — a guard flag (s_retrying) prevents duplicate connection
 * attempts while the first one is in flight.
 *
 * @return ESP_OK or an esp_err_t from esp_wifi_connect()/esp_wifi_start().
 */
esp_err_t wifi_retry(void);

/**
 * @brief   Block until an IP is obtained (or forever if it never arrives).
 *
 * @deprecated Use wifi_wait_for_ip() with an explicit timeout instead.
 * @return   Pointer to the static IP string buffer (never NULL while hosted).
 */
const char *check_LAN_ip(void);

/**
 * @brief   Wait up to @p timeout_ms milliseconds for the station to obtain
 *          a DHCP lease.
 *
 * @param timeout_ms  Maximum wait time in milliseconds.
 * @return            Pointer to static IP string on success, NULL on timeout.
 */
const char *wifi_wait_for_ip(uint32_t timeout_ms);

/**
 * @brief   Non-blocking IP getter.
 *
 * @return Pointer to static IP string if wifi_health() is true, else NULL.
 */
const char *get_LAN_ip(void);

#ifdef __cplusplus
}
#endif
