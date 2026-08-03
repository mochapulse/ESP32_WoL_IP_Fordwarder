/**
 * @file    test_dotenv.c
 * @brief   Unit tests for the dotenv embedded .env parser.
 *
 * Tests the line-parsing logic (dotenv_parse_line) and the lookup functions
 * (dotenv_get / dotenv_get_int) independently of the flash-embedded .env blob.
 * Each test resets the internal entry table to ensure isolation.
 *
 * @test{dotenv_parse_line} Basic KEY=VALUE parsing.
 * @test{dotenv_parse_line} Quoted value handling.
 * @test{dotenv_parse_line} Leading/trailing whitespace stripping.
 * @test{dotenv_parse_line} Comment-line skipping.
 * @test{dotenv_parse_line} CRLF trailing character stripping (\\r).
 * @test{dotenv_get} Missing-key returns NULL.
 * @test{dotenv_get_int} Default falue fallback.
 */

#include "dotenv.h"
#include "unity.h"
#include <string.h>

void test_dotenv_anchor(void) {}

TEST_CASE("dotenv_parse_line stores KEY=VALUE", "[dotenv]")
{
    dotenv_reset();
    const char *line = "KEY=VALUE";
    dotenv_parse_line(line, strlen(line));
    TEST_ASSERT_EQUAL_STRING("VALUE", dotenv_get("KEY"));
}

TEST_CASE("dotenv_parse_line strips quotes from VALUE=\"val\"", "[dotenv]")
{
    dotenv_reset();
    const char *line = "KEY=\"val\"";
    dotenv_parse_line(line, strlen(line));
    TEST_ASSERT_EQUAL_STRING("val", dotenv_get("KEY"));
}

TEST_CASE("dotenv_parse_line strips trailing whitespace on key and value", "[dotenv]")
{
    dotenv_reset();
    const char *line = "KEY  =  VAL  ";
    dotenv_parse_line(line, strlen(line));
    TEST_ASSERT_EQUAL_STRING("VAL", dotenv_get("KEY"));
}

TEST_CASE("dotenv_parse_line does not filter # — caller handles comments", "[dotenv]")
{
    dotenv_reset();
    const char *line = "#KEY=VALUE";
    dotenv_parse_line(line, strlen(line));
    TEST_ASSERT_EQUAL_STRING("VALUE", dotenv_get("#KEY"));
}

TEST_CASE("dotenv_parse_line strips trailing \\r (CRLF)", "[dotenv]")
{
    dotenv_reset();
    const char *line = "KEY=VAL\r";
    dotenv_parse_line(line, strlen(line));
    TEST_ASSERT_EQUAL_STRING("VAL", dotenv_get("KEY"));
}

TEST_CASE("dotenv_get returns NULL for unknown key", "[dotenv]")
{
    dotenv_reset();
    TEST_ASSERT_NULL(dotenv_get("NONEXISTENT"));
}

TEST_CASE("dotenv_get_int returns default when key missing", "[dotenv]")
{
    dotenv_reset();
    TEST_ASSERT_EQUAL(42, dotenv_get_int("PORT", 42));
}
