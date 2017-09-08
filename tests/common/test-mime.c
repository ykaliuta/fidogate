/*
 *
 *  test-mime.c
 *
 *  January 15, 2017 Yauheni Kaliuta
 *
 */
#include <cgreen/cgreen.h>
#include <stdio.h>


int mime_enheader(char **dst, unsigned char *src, size_t len, char *encoding);

void debug(int lvl, const char *fmt, ...)
{
}


Ensure(enheader_encodes_long_line)
{
	char *src = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
	char *exp = "=?windows-1251?B?YWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFh?=\n =?windows-1251?B?YWFhYQ==?=";
	char *res = NULL;

	mime_enheader(&res, src, strlen(src), "windows-1251");

	assert_that(res, is_equal_to_string(exp));
	free(res);
}

static TestSuite *create_mime_suite(void)
{
    TestSuite *suite = create_named_test_suite(
	    "MIME suite");
    add_test(suite, enheader_encodes_long_line);

    return suite;
}

int main(int argc, char **argv) {
    TestSuite *suite = create_mime_suite();
    return run_test_suite(suite, create_text_reporter());
}
