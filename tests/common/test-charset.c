#include <cgreen/cgreen.h>
#include <stdio.h>

#include "../../src/include/prototypes.h"

void debug(int lvl, const char *fmt, ...)
{
}

void fglog(const char *fmt, ...)
{
}

int verbose;

Ensure(charset_recodes_latin1)
{
	char *src = "\xf8";
	char *exp = "ø";
	char *res;
	size_t dstlen;
	size_t srclen = strlen(src) + 1;

	charset_recode_buf(&res, &dstlen, src, srclen, "LATIN-1", "UTF-8");

	assert_that(res, is_equal_to_string(exp));
	free(res);
}

static TestSuite *create_mime_suite(void)
{
    TestSuite *suite = create_named_test_suite(
	    "charset suite");
    add_test(suite, charset_recodes_latin1);
    return suite;
}

int main(int argc, char **argv)
{
    TestSuite *suite = create_mime_suite();

    if (argc > 1)
	return run_single_test(suite, argv[1], create_text_reporter());

    return run_test_suite(suite, create_text_reporter());
}
