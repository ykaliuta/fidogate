/*
 *
 *  test-mime.c
 *
 *  January 15, 2017 Yauheni Kaliuta
 *
 */
#include <cgreen/cgreen.h>
#include <stdio.h>


int mime_header_enc(char **dst, unsigned char *src, size_t len, char *encoding);

char *mime_header_dec(char *d, size_t n, char *s);

void debug(int lvl, const char *fmt, ...)
{
}

void fglog(const char *fmt, ...)
{
}

Ensure(hdr_enc_encodes_long_line)
{
	char *src = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
	char *exp = "=?windows-1251?B?YWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFh?=\n =?windows-1251?B?YWFhYQ==?=";
	char *res = NULL;

	mime_header_enc(&res, src, strlen(src), "windows-1251");

	assert_that(res, is_equal_to_string(exp));
	free(res);
}

#define MSG_MAXSUBJ	72
#define J "\xe9" /* Cyrillic й */
static char dres[MSG_MAXSUBJ];

Ensure(hdr_dec_decodes_le28chars_line)
{
	char *src = "=?UTF-8?B?0LnQudC50LnQudC50LnQudC50LnQudC50LnQudC50LnQudC50LnQudC5?=";
	char *exp =
		J J J J J J J J J J J J J J J J
		J J J J J; /* 21 */

	mime_header_dec(dres, sizeof(dres), src);

	assert_that(dres, is_equal_to_string(exp));
}

Ensure(hdr_dec_decodes_ge35chars_line)
{
	char *src =
		"=?UTF-8?B?0LnQudC50LnQudC50LnQudC50LnQudC50LnQudC50LnQudC50LnQudC5?=\n"
		" =?UTF-8?B?0LnQudC50LnQudC50LnQudC50LnQudC50LnQudC50Lk=?=";
	char *exp =
		J J J J J J J J J J J J J J J J
		J J J J J J J J J J J J J J J J
		J J J J J; /* 37 */

	mime_header_dec(dres, sizeof(dres), src);

	assert_that(dres, is_equal_to_string(exp));
}

Ensure(hdr_dec_decodes_nonmime_at_start)
{
	char *src = "Re: Re: =?utf-8?B?0LnQudC50LnQuQ==?=";
	char *exp = "Re: Re: " J J J J J;

	mime_header_dec(dres, sizeof(dres), src);

	assert_that(dres, is_equal_to_string(exp));
}
#undef J

Ensure(hdr_dec_handles_empty)
{
	char *src = "";
	char *exp = "";

	mime_header_dec(dres, sizeof(dres), src);

	assert_that(dres, is_equal_to_string(exp));
}

Ensure(hdr_dec_preserves_non_mime)
{
	char *src =
		"abcdefghijklmnopqrstuvwxyz123456"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ123456"; /* 64 */
	char *exp = src;

	mime_header_dec(dres, sizeof(dres), src);

	assert_that(dres, is_equal_to_string(exp));
}

Ensure(hdr_dec_cuts_long_non_mime)
{
	char *src =
		"abcdefghijklmnopqrstuvwxyz123456"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ123456" /* 64 */
		"abcdefghijklmnopqrstuvwxyz123456"; /* 96 */
	char *exp =
		"abcdefghijklmnopqrstuvwxyz123456"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ123456" /* 64 */
		"abcdefg"; /* 71 */

	mime_header_dec(dres, sizeof(dres), src);

	assert_that(dres, is_equal_to_string(exp));
}

#define MIME_MAX_ENC_LEN 31

Ensure(hdr_dec_skips_long_charset_decoding)
{
	char *src = "=?UTF-8longlonglonglonglonglonglong?B?0LnQudC50LnQudC50LnQudC50LnQudC50LnQudC50LnQudC50LnQudC5?=";
        /* cut to 71 + '\0' symbols */
	char *exp = "=?UTF-8longlonglonglonglonglonglong?B?0LnQudC50LnQudC50LnQudC50LnQudC50";

	mime_header_dec(dres, sizeof(dres), src);

	assert_that(dres, is_equal_to_string(exp));
}

static TestSuite *create_mime_suite(void)
{
    TestSuite *suite = create_named_test_suite(
	    "MIME suite");
    add_test(suite, hdr_enc_encodes_long_line);
    add_test(suite, hdr_dec_decodes_le28chars_line);
    add_test(suite, hdr_dec_decodes_ge35chars_line);
    add_test(suite, hdr_dec_decodes_nonmime_at_start);
    add_test(suite, hdr_dec_handles_empty);
    add_test(suite, hdr_dec_preserves_non_mime);
    add_test(suite, hdr_dec_cuts_long_non_mime);
    add_test(suite, hdr_dec_skips_long_charset_decoding);
    return suite;
}

int main(int argc, char **argv) {
    TestSuite *suite = create_mime_suite();
    return run_test_suite(suite, create_text_reporter());
}
