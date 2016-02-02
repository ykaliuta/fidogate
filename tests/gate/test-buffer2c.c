/*
 *  cc -Wall -g -o test_buffer2c test-buffer2c.c buffer2c.c -lcgreen
 *
 *  January 30, 2016 Yauheni Kaliuta
 */

#include <cgreen/cgreen.h>

#include "buffer2c.h"

static char *s;
static char *exp;
static size_t string_limit;

Ensure(b2c_returns_quotes_for_empty)
{
    s = "";
    exp = "\"\"";
}

Ensure(b2c_returns_null_for_null)
{
    s = NULL;
    exp = NULL;
}

Ensure(b2c_converts_eol)
{
    s = "abc\n";
    exp = "\"abc\\n\"\n\"\"";
}

Ensure(b2c_converts_quotes)
{
    s = "abc\"\"\"def";
    exp = "\"abc\\\"\\\"\\\"def\"";
}

Ensure(b2c_converts_0x01)
{
    s = "abc\x01""de";
    exp = "\"abc\\x01\"\"de\"";
}

Ensure(b2c_converts_crs)
{
    s = "abc\rde\rfg";
    exp = "\"abc\\rde\\rfg\"";
}

Ensure(b2c_obeys_one_string_limit_for_ascii)
{
    s = "abcdef";
    exp = "\"abcd\"" "\n" "\"ef\"";
}

Ensure(b2c_obeys_many_strings_limit_for_ascii)
{
    s = "abcdefghigklmn";
    exp = "\"abcd\"" "\n" "\"efgh\"" "\n" "\"igkl\""
	"\n" "\"mn\"";
}

Ensure(b2c_obeys_one_string_limit_for_special)
{
    s = "ab\x01""de";
    exp = "\"ab\"" "\n" "\"\\x01\"" "\n" "\"de\"";
}

Ensure(b2c_obeys_buffer_size)
{
    char s[] = "abcdefgh";
    exp = "\"abcdefgh\\x00\"";
    char *r;

    r = buffer2c_size_limit(s, sizeof(s), 0);
    assert_that(r, is_equal_to_string(exp));
}

static void teardown(void)
{
    char *r;

    r = buffer2c(s);
    assert_that(r, is_equal_to_string(exp));
}

static void setup_limit(void)
{
    string_limit = 6;
}

static void teardown_limit(void)
{
    char *r;

    r = buffer2c_limit(s, string_limit);
    assert_that(r, is_equal_to_string(exp));
}


static TestSuite *create_b2c_suite(void)
{
    TestSuite *suite = create_named_test_suite(
        "Buffer to C");

    TestSuite *s1 = create_named_test_suite(
        "Buffer to C without line limit");
    TestSuite *s2 = create_named_test_suite(
        "Buffer to C with line limit");

    add_test(s1, b2c_converts_crs);
    add_test(s1, b2c_converts_0x01);
    add_test(s1, b2c_converts_quotes);
    add_test(s1, b2c_returns_null_for_null);
    add_test(s1, b2c_returns_quotes_for_empty);
    add_test(s1, b2c_converts_eol);

    set_teardown(s1, teardown);

    add_test(s2, b2c_obeys_one_string_limit_for_special);
    add_test(s2, b2c_obeys_many_strings_limit_for_ascii);
    add_test(s2, b2c_obeys_one_string_limit_for_ascii);

    set_setup(s2, setup_limit);
    set_teardown(s2, teardown_limit);

    add_test(suite, b2c_obeys_buffer_size);
    add_suite(suite, s1);
    add_suite(suite, s2);
    
    return suite;
}

int main(int argc, char **argv)
{
    TestSuite *suite = create_b2c_suite();
    return run_test_suite(suite, create_text_reporter());
}
