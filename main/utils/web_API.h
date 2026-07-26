#pragma once

#include "esp_http_server.h"

#ifdef __cplusplus
extern "C" {
#endif

void web_API_init(httpd_handle_t server);

#ifdef __cplusplus
}
#endif
