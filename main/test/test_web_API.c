/**
 * @file    test_web_API.c
 * @brief   Unit tests for the web API utility functions.
 *
 * Tests the chip_model_str() enum-to-string mapping for all known ESP32
 * chip variants and the unknown fallback.
 *
 * @test{chip_model_str} All known chip model enum values map correctly.
 * @test{chip_model_str} Out-of-range falls back to "Unknown".
 */

#include "web_API.h"
#include "unity.h"
#include "esp_chip_info.h"

void test_web_API_anchor(void) {}

TEST_CASE("chip_model_str maps CHIP_ESP32", "[web_API]")
{
    TEST_ASSERT_EQUAL_STRING("ESP32", chip_model_str(CHIP_ESP32));
}

TEST_CASE("chip_model_str maps CHIP_ESP32S2", "[web_API]")
{
    TEST_ASSERT_EQUAL_STRING("ESP32-S2", chip_model_str(CHIP_ESP32S2));
}

TEST_CASE("chip_model_str maps CHIP_ESP32S3", "[web_API]")
{
    TEST_ASSERT_EQUAL_STRING("ESP32-S3", chip_model_str(CHIP_ESP32S3));
}

TEST_CASE("chip_model_str maps CHIP_ESP32C3", "[web_API]")
{
    TEST_ASSERT_EQUAL_STRING("ESP32-C3", chip_model_str(CHIP_ESP32C3));
}

TEST_CASE("chip_model_str returns Unknown for out-of-range", "[web_API]")
{
    TEST_ASSERT_EQUAL_STRING("Unknown", chip_model_str((esp_chip_model_t)999));
}
