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

Ensure(bp_understand_format)
{
    char *exp = TEST_STR "10" TEST_STR;

    bp_printf(buffer_id, TEST_STR "%d%s", 10, TEST_STR);

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
    add_test(suite, bp_understand_format);

    set_setup(suite, bp_test_setup);
    set_teardown(suite, bp_test_teardown);

    return suite;
}

int main(int argc, char **argv)
{
    TestSuite *suite = create_bp_suite();
    return run_test_suite(suite, create_text_reporter());
}
