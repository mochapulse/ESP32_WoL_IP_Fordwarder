/**
 * @file    web_API.h
 * @brief   HTTP endpoint registration — static files, /api/status, /api/wol.
 *
 * Registers:
 *  - `/api/status`  → JSON payload with full ESP32 metadata.
 *  - `/api/wol`     → trigger Wake-on-LAN magic packet send.
 *  - `/`            → serves index.html (embedded in flash).
 *  - `\/\*`         → wildcard catch-all for /style.css, /app.js, etc.
 *
 * API routes enforce:
 *  - Header auth via `X-API-Key` (token from WEB_API_TOKEN in .env).
 *  - Heap guard that rejects requests when free heap is below a safe threshold.
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
