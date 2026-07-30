/**
 * @file wol.c
 * @brief ESP-IDF UDP example that sends a Wake-on-LAN (WoL) Magic Packet.
 */

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "esp_err.h"
#include "esp_log.h"
#include "lwip/inet.h"
#include "lwip/sockets.h"

/** @brief Target host IPv4 address where the Magic Packet is sent. */
#define SERVER_IP  "192.168.1.17"
/** @brief Target host MAC address used to build the Magic Packet payload. */
#define SERVER_MAC "3c:7c:3f:25:41:90"
/** @brief UDP destination port for Wake-on-LAN. */
#define PORT       9

/** @brief Log tag used by this example module. */
static const char *TAG = "wol_example";

/**
 * @brief Build and send a Wake-on-LAN Magic Packet over UDP.
 *
 * The packet format is:
 * - 6 bytes of 0xFF
 * - 16 repetitions of the target MAC address (6 bytes each)
 *
 * @param[in] target_ip  Destination IPv4 address in dotted string format.
 * @param[in] target_mac Target MAC address as "xx:xx:xx:xx:xx:xx".
 * @param[in] port       Destination UDP port.
 *
 * @return
 * - ESP_OK on success.
 * - ESP_ERR_INVALID_ARG if the MAC format is invalid.
 * - ESP_FAIL on socket/configuration/send failure.
 */
static esp_err_t send_magic_packet(const char *target_ip, const char *target_mac, uint16_t port)
{
    uint8_t mac[6];
    uint8_t packet[102];

    /** Parse MAC string into 6 binary bytes. */
    if (sscanf(target_mac, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
               &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]) != 6) {
        ESP_LOGE(TAG, "Failed to parse MAC address: %s", target_mac);
        return ESP_ERR_INVALID_ARG;
    }

    /** Build Magic Packet payload. */
    memset(packet, 0xFF, 6);
    for (int i = 0; i < 16; i++) {
        memcpy(packet + 6 + (i * 6), mac, 6);
    }

    /** Create UDP socket. */
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        ESP_LOGE(TAG, "Failed to create socket, errno=%d", errno);
        return ESP_FAIL;
    }

    /** Enable broadcast transmission on the socket. */
    int broadcast_enable = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast_enable, sizeof(broadcast_enable)) < 0) {
        ESP_LOGE(TAG, "Failed to enable SO_BROADCAST, errno=%d", errno);
        close(sockfd);
        return ESP_FAIL;
    }

    /** Configure destination IPv4 socket address. */
    struct sockaddr_in target_addr;
    memset(&target_addr, 0, sizeof(target_addr));
    target_addr.sin_family = AF_INET;
    target_addr.sin_port = htons(port);
    target_addr.sin_addr.s_addr = inet_addr(target_ip);

    /** Send the packet to the configured destination. */
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

/**
 * @brief Example entry point.
 *
 * @note Wi-Fi and network stack initialization must be completed before this
 * function is executed.
 */
void app_main(void)
{
    esp_err_t err = send_magic_packet(SERVER_IP, SERVER_MAC, PORT);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Wake-on-LAN send failed: %s", esp_err_to_name(err));
    }
}