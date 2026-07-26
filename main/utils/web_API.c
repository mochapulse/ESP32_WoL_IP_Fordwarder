#include "web_API.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cJSON.h"
#include "esp_err.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "wifi_connect.h"

static const char *TAG = "web_API";

extern const uint8_t index_html_start[] asm("_binary_index_html_start");
extern const uint8_t index_html_end[]   asm("_binary_index_html_end");
extern const uint8_t style_css_start[]  asm("_binary_style_css_start");
extern const uint8_t style_css_end[]    asm("_binary_style_css_end");
extern const uint8_t app_js_start[]     asm("_binary_app_js_start");
extern const uint8_t app_js_end[]       asm("_binary_app_js_end");

typedef struct {
    const char *uri;
    const char *content_type;
    const uint8_t *start;
    const uint8_t *end;
} embedded_file_t;

static const embedded_file_t s_files[] = {
    {"/",           "text/html",              index_html_start, index_html_end},
    {"/index.html", "text/html",              index_html_start, index_html_end},
    {"/style.css",  "text/css",               style_css_start,  style_css_end},
    {"/app.js",     "application/javascript", app_js_start,     app_js_end},
};

static esp_err_t static_handler(httpd_req_t *req)
{
    for (size_t i = 0; i < sizeof(s_files) / sizeof(s_files[0]); i++) {
        if (strcmp(req->uri, s_files[i].uri) != 0) continue;

        size_t len = (size_t)(s_files[i].end - s_files[i].start);
        httpd_resp_set_type(req, s_files[i].content_type);
        httpd_resp_send(req, (const char *)s_files[i].start, len);
        return ESP_OK;
    }

    httpd_resp_send_404(req);
    return ESP_FAIL;
}

static esp_err_t status_handler(httpd_req_t *req)
{
    const char *lan_ip = get_LAN_ip();

    cJSON *root = cJSON_CreateObject();
    cJSON_AddBoolToObject(root, "wifi", wifi_health());
    cJSON_AddStringToObject(root, "ip", lan_ip ? lan_ip : "");
    cJSON_AddNumberToObject(root, "heap_free", esp_get_free_heap_size());

    char *json_str = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, json_str, strlen(json_str));

    free(json_str);
    return ESP_OK;
}

void web_API_init(httpd_handle_t server)
{
    httpd_uri_t wildcard = {
        .uri     = "/*",
        .method  = HTTP_GET,
        .handler = static_handler,
    };
    httpd_register_uri_handler(server, &wildcard);

    httpd_uri_t status_uri = {
        .uri     = "/api/status",
        .method  = HTTP_GET,
        .handler = status_handler,
    };
    httpd_register_uri_handler(server, &status_uri);

    ESP_LOGI(TAG, "Endpoints registered");
}
