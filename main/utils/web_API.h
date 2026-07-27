/**
 * @file    web_API.h
 * @brief   HTTP endpoint registration — static files and /api/status JSON.
 *
 * Registers:
 *  - `/api/status`  → JSON payload with full ESP32 metadata.
 *  - `/`            → serves index.html (embedded in flash).
 *  - `\/\*`         → wildcard catch-all for /style.css, /app.js, etc.
 *
 * Registration order matters with wildcard matching enabled: exact routes
 * are registered before the `\/\*` catch-all.
 */

#pragma once

#include "esp_http_server.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Register all URI handlers on the given server instance.
 *
 * @param server  Valid httpd_handle_t from httpd_start().
 */
void web_API_init(httpd_handle_t server);

#ifdef __cplusplus
}
#endif
