/**
 * @file    web_util.h
 * @brief   HTTP server lifecycle — init, start, health check, stop.
 *
 * Thin wrapper around esp_http_server. Stores the port and server handle in
 * module-level statics so callers only need to invoke web_start()/web_stop().
 * Endpoint registration is delegated to web_API_init().
 */

#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Store the HTTP server port. Must be called before web_start().
 *
 * @param port  TCP port number (default: 80).
 * @return      Always ESP_OK.
 */
esp_err_t web_init(uint16_t port);

/**
 * @brief   Start the HTTP server and register all URI handlers.
 *
 * If the server is already running this is a no-op.
 * Enables URI wildcard matching (httpd_uri_match_wildcard) so that '/' '*'
 * catch-all patterns work.
 *
 * @return  ESP_OK on success, or an esp_err_t from httpd_start().
 */
esp_err_t web_start(void);

/**
 * @brief   Check whether the HTTP server is currently running.
 *
 * @return  true if the server handle is non-NULL.
 */
bool web_health(void);

/**
 * @brief   Stop the HTTP server and NULL the handle.
 *
 * Safe to call when already stopped (no-op).
 *
 * @return  ESP_OK or an esp_err_t from httpd_stop().
 */
esp_err_t web_stop(void);

#ifdef __cplusplus
}
#endif
