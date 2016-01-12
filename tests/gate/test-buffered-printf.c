/*
 */

#include "buffered-printf.h"
#include <cgreen/cgreen.h>

static char *s;
static int buffer_id = 0;
static int buffer_id2 = 10;

#define TEST_STR "Example string"

static void
bp_test_setup(void)
{
    s = NULL;
}

static void
bp_test_teardown(void)
{
    bp_buffer_release(buffer_id);
    bp_buffer_release(buffer_id2);
}

Ensure(bp_is_empty_by_default)
{
    s = bp_buffer_content(buffer_id);
    assert_that(s, is_equal_to_string(""));
}

Ensure(bp_outputs_string_to_buffer)
{
    char *exp = TEST_STR;

    bp_printf(buffer_id, TEST_STR);

    s = bp_buffer_content(buffer_id);
    assert_that(s, is_equal_to_string(exp));
}

Ensure(bp_adds_string_to_buffer)
{
    char *exp = TEST_STR TEST_STR;

    bp_printf(buffer_id, TEST_STR);
    bp_printf(buffer_id, TEST_STR);

    s = bp_buffer_content(buffer_id);

    assert_that(s, is_equal_to_string(exp));
}

Ensure(bp_destinguishes_buffer_id)
{
    char *exp1 = TEST_STR;
    char *exp2 = TEST_STR TEST_STR;

    bp_printf(buffer_id, TEST_STR);
    bp_printf(buffer_id2, TEST_STR TEST_STR);

    s = bp_buffer_content(buffer_id);
    assert_that(s, is_equal_to_string(exp1));
    s = bp_buffer_content(buffer_id2);
    assert_that(s, is_equal_to_string(exp2));
}

Ensure(bp_understands_format)
{
    char *exp = TEST_STR "10" TEST_STR;

    bp_printf(buffer_id, TEST_STR "%d%s", 10, TEST_STR);

    s = bp_buffer_content(buffer_id);
    assert_that(s, is_equal_to_string(exp));
}
#define DEFAULT_SIZE 2048

static char exp_extends[DEFAULT_SIZE + 1];

static void
fill_array(char *arr, size_t size, char c)
{
    int i;

    for (i = 0; i < size - 1; i++)
	arr[i] = c;
    arr[size - 1] = '\0';
}

static char *
gen_extends_exp(void)
{
    fill_array(exp_extends, sizeof(exp_extends), 'a');
    return exp_extends;
}

static void
bp_print_n_chars(char c, size_t n)
{
    int i;

    for (i = 0; i < n; i++)
	bp_printf(buffer_id, "%c", c);
}

Ensure(bp_extends)
{
    char *exp = gen_extends_exp();

    bp_print_n_chars('a', DEFAULT_SIZE);

    s = bp_buffer_content(buffer_id);
    assert_that(s, is_equal_to_string(exp));
}

static char long_string[2048];

static char *
gen_long_string(void)
{
    fill_array(long_string, sizeof(long_string), 'a');
    return long_string;
}

Ensure(bp_extends_long)
{
    char *exp = gen_long_string();

    bp_print_n_chars('a', sizeof(long_string) - 1);

    s = bp_buffer_content(buffer_id);
    assert_that(s, is_equal_to_string(exp));
}

static TestSuite *create_bp_suite(void)
{
    TestSuite *suite = create_named_test_suite(
	"Buffered Printf suite");

    add_test(suite, bp_is_empty_by_default);
    add_test(suite, bp_outputs_string_to_buffer);
    add_test(suite, bp_adds_string_to_buffer);
    add_test(suite, bp_destinguishes_buffer_id);
    add_test(suite, bp_understands_format);
    add_test(suite, bp_extends);
    add_test(suite, bp_extends_long);

    set_setup(suite, bp_test_setup);
    set_teardown(suite, bp_test_teardown);

    return suite;
}

int main(int argc, char **argv)
{
    TestSuite *suite = create_bp_suite();
    return run_test_suite(suite, create_text_reporter());
}
