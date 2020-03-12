#include "fidogate.h"
#include <cgreen/cgreen.h>
#include <stdio.h>


Ensure(date_handles_via_format)
{
    char *res;

    res = date(DATE_VIA, NULL);

    assert_that(res, is_not_null);
}

static TestSuite *create_suite(void)
{
    TestSuite *suite = create_named_test_suite("Date suite");

    add_test(suite, date_handles_via_format);

    return suite;
}

int main(int argc, char **argv) {
    TestSuite *suite = create_suite();

    if (argc > 1)
	return run_single_test(suite, argv[1], create_text_reporter());

    return run_test_suite(suite, create_text_reporter());
}
