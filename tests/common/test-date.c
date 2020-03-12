#include "fidogate.h"
#include <cgreen/cgreen.h>
#include <stdio.h>


Ensure(date_handles_via_format)
{
    char *res;

    res = date(DATE_VIA, NULL);

    assert_that(res, is_not_null);
}

Ensure(parsedate_handles_rfc822)
{
    char *src = "Wed, 11 Mar 2020 05:45:29 -0700";
    time_t exp = 1583930729;
    time_t res;
    TIMEINFO *default_now = NULL;

    res = parsedate(src, default_now);

    assert_that(res, is_equal_to(exp));
}

Ensure(parsedate_handles_google)
{
    char *src = "Wed, 11 Mar 2020 05:45:29 -0700 (PDT)";
    time_t exp = 1583930729;
    time_t res;
    TIMEINFO *default_now = NULL;

    res = parsedate(src, default_now);

    assert_that(res, is_equal_to(exp));
}

Ensure(date_tz_converts_positive_tz)
{
    time_t src = 1583930729;
    char *exp = "11 Mar 20  14:45:29";
    char *tz = "0200";
    char *res;

    res = date_tz(DATE_FTS_0001, &src, tz);

    assert_that(res, is_equal_to_string(exp));
}

Ensure(date_tz_converts_negative_tz)
{
    time_t src = 1583930729;
    char *exp = "11 Mar 20  05:45:29";
    char *tz = "-0700";
    char *res;

    res = date_tz(DATE_FTS_0001, &src, tz);

    assert_that(res, is_equal_to_string(exp));
}

static TestSuite *create_suite(void)
{
    TestSuite *suite = create_named_test_suite("Date suite");

    add_test(suite, date_handles_via_format);
    add_test(suite, parsedate_handles_rfc822);
    add_test(suite, parsedate_handles_google);
    add_test(suite, date_tz_converts_positive_tz);
    add_test(suite, date_tz_converts_negative_tz);

    return suite;
}

int main(int argc, char **argv) {
    TestSuite *suite = create_suite();

    if (argc > 1)
	return run_single_test(suite, argv[1], create_text_reporter());

    return run_test_suite(suite, create_text_reporter());
}
