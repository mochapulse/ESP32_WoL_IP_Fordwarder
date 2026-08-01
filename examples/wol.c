#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "esp_err.h"
#include "esp_log.h"
#include "lwip/inet.h"
#include "lwip/sockets.h"

// --- Server-specific parameters ---
#define SERVER_IP  "192.168.1.17"
#define SERVER_MAC "3c:7c:3f:25:41:90" //enp5s0 in ubuntu server
#define PORT       9

static const char *TAG = "wol_example";

static esp_err_t send_magic_packet(const char *target_ip, const char *target_mac, uint16_t port)
{
    uint8_t mac[6];
    uint8_t packet[102];

    // 1. Convert the defined MAC into 6 binary bytes
    if (sscanf(target_mac, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
               &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]) != 6) {
        ESP_LOGE(TAG, "Failed to parse MAC address: %s", target_mac);
        return ESP_ERR_INVALID_ARG;
    }

    // 2. Build the Magic Packet (6x 0xFF followed by 16x the MAC)
    memset(packet, 0xFF, 6);
    for (int i = 0; i < 16; i++) {
        memcpy(packet + 6 + (i * 6), mac, 6);
    }

    // 3. Create the UDP socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        ESP_LOGE(TAG, "Failed to create socket, errno=%d", errno);
        return ESP_FAIL;
    }

    // 4. Enable SO_BROADCAST
    int broadcast_enable = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast_enable, sizeof(broadcast_enable)) < 0) {
        ESP_LOGE(TAG, "Failed to enable SO_BROADCAST, errno=%d", errno);
        close(sockfd);
        return ESP_FAIL;
    }

    // 5. Configure the destination address using your IP (192.168.1.17)
    struct sockaddr_in target_addr;
    memset(&target_addr, 0, sizeof(target_addr));
    target_addr.sin_family = AF_INET;
    target_addr.sin_port = htons(port);
    target_addr.sin_addr.s_addr = inet_addr(target_ip);

    // 6. Send the packet
    ssize_t bytes_sent = sendto(sockfd, packet, sizeof(packet), 0,
                                (struct sockaddr*)&target_addr, sizeof(target_addr));

    if (bytes_sent < 0) {
        ESP_LOGE(TAG, "Failed to send packet, errno=%d", errno);
        close(sockfd);
        return ESP_FAIL;
    } else {
        ESP_LOGI(TAG, "Magic Packet sent to %s [%s]", target_ip, target_mac);
    }

    close(sockfd);
    return ESP_OK;
}

void app_main(void)
{
    /* Example only: Wi-Fi and network initialization must be done before calling this. */
    esp_err_t err = send_magic_packet(SERVER_IP, SERVER_MAC, PORT);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Wake-on-LAN send failed: %s", esp_err_to_name(err));
    }
}