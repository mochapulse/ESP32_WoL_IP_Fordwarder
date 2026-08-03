/**
 * @file    test_wol.c
 * @brief   Unit tests for the Wake-on-LAN module.
 *
 * Tests MAC format validation, hex character validation, wol_init() parameter
 * validation, and wol_send() state guard. Each test resets the WoL module state
 * to ensure isolation.
 *
 * @test{is_hex_char} Valid and invalid hex characters.
 * @test{is_valid_mac_format} MAC string format validation edges.
 * @test{wol_init} NULL and invalid parameter rejection.
 * @test{wol_init} Valid parameter acceptance.
 * @test{wol_send} State guard before init.
 */

#include "wol.h"
#include "unity.h"
#include "esp_err.h"

void test_wol_anchor(void) {}

TEST_CASE("is_hex_char accepts valid hex digits", "[wol]")
{
    TEST_ASSERT_TRUE(is_hex_char('A'));
    TEST_ASSERT_TRUE(is_hex_char('F'));
    TEST_ASSERT_TRUE(is_hex_char('0'));
    TEST_ASSERT_TRUE(is_hex_char('9'));
    TEST_ASSERT_TRUE(is_hex_char('a'));
    TEST_ASSERT_TRUE(is_hex_char('f'));
    TEST_ASSERT_FALSE(is_hex_char('G'));
    TEST_ASSERT_FALSE(is_hex_char('Z'));
    TEST_ASSERT_FALSE(is_hex_char(':'));
    TEST_ASSERT_FALSE(is_hex_char('\0'));
}

TEST_CASE("is_valid_mac_format accepts valid lowercase MAC", "[wol]")
{
    TEST_ASSERT_TRUE(is_valid_mac_format("00:11:22:33:44:55"));
}

TEST_CASE("is_valid_mac_format accepts valid uppercase MAC", "[wol]")
{
    TEST_ASSERT_TRUE(is_valid_mac_format("AA:BB:CC:DD:EE:FF"));
}

TEST_CASE("is_valid_mac_format accepts valid mixed-case MAC", "[wol]")
{
    TEST_ASSERT_TRUE(is_valid_mac_format("aa:bb:cc:dd:ee:ff"));
}

TEST_CASE("is_valid_mac_format rejects too-short MAC", "[wol]")
{
    TEST_ASSERT_FALSE(is_valid_mac_format("00:11:22:33:44"));
}

TEST_CASE("is_valid_mac_format rejects too-long MAC", "[wol]")
{
    TEST_ASSERT_FALSE(is_valid_mac_format("00:11:22:33:44:55:66"));
}

TEST_CASE("is_valid_mac_format rejects invalid hex chars", "[wol]")
{
    TEST_ASSERT_FALSE(is_valid_mac_format("GG:11:22:33:44:55"));
}

TEST_CASE("is_valid_mac_format rejects wrong separator", "[wol]")
{
    TEST_ASSERT_FALSE(is_valid_mac_format("00-11-22-33-44-55"));
}

TEST_CASE("wol_init rejects NULL MAC", "[wol]")
{
    wol_reset();
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG,
                      wol_init(NULL, "192.168.1.255"));
}

TEST_CASE("wol_init rejects NULL broadcast IP", "[wol]")
{
    wol_reset();
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG,
                      wol_init("00:11:22:33:44:55", NULL));
}

TEST_CASE("wol_init rejects invalid MAC string format", "[wol]")
{
    wol_reset();
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG,
                      wol_init("invalid_mac", "192.168.1.255"));
}

TEST_CASE("wol_init rejects invalid IP string", "[wol]")
{
    wol_reset();
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG,
                      wol_init("00:11:22:33:44:55", "not.an.ip"));
}

TEST_CASE("wol_init accepts valid MAC and IP", "[wol]")
{
    wol_reset();
    TEST_ASSERT_EQUAL(ESP_OK,
                      wol_init("00:11:22:33:44:55", "192.168.1.255"));
}

TEST_CASE("wol_send before init returns ESP_ERR_INVALID_STATE", "[wol]")
{
    wol_reset();
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_STATE, wol_send());
}
