/*
 *
 *  test-mime.c
 *
 *  January 15, 2017 Yauheni Kaliuta
 *
 */
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

/* check for writing beyond end of line. May not crash, but valgrind reports */
Ensure(hdr_enc_78_limit)
{
	char *src = "Subject: 75 лет назад: Четвертый Интернационал выдвигает революционные перспекти\n";
	char *exp = "Subject: 75 =?utf-8?B?0LvQtdGCINC90LDQt9Cw0LQ6INCn0LXRgtCy0LXRgNGC?=\n"
		" =?utf-8?B?0YvQuSDQmNC90YLQtdGA0L3QsNGG0LjQvtC90LDQuyDQstGL0LTQsg==?=\n"
		" =?utf-8?B?0LjQs9Cw0LXRgiDRgNC10LLQvtC70Y7RhtC40L7QvdC90YvQtSDQvw==?=\n"
		" =?utf-8?B?0LXRgNGB0L/QtdC60YLQuAo=?=\n";
	char *res = NULL;

	mime_header_enc(&res, src, "utf-8", MIME_B64);

	assert_that(res, is_equal_to_string(exp));
	free(res);
}

Ensure(hdr_enc_encodes_cyrillic)
{
	char *src = "Subject: tеsт кириллица latinitsa";
	char *exp = "Subject: =?utf-8?B?dNC1c9GCINC60LjRgNC40LvQu9C40YbQsA==?= latinitsa\n";
	char *res = NULL;

	mime_header_enc(&res, src, "utf-8", MIME_B64);

	assert_that(res, is_equal_to_string(exp));
	free(res);
}

Ensure(hdr_enc_encodes_cyrillic2)
{
	char *src = "Subject: tеsт кириллица latinitsa, оdna, двeee, триiiiiiii";
	char *exp = "Subject: =?utf-8?B?dNC1c9GCINC60LjRgNC40LvQu9C40YbQsA==?= latinitsa,\n =?utf-8?B?0L5kbmEsINC00LJlZWUsINGC0YDQuGlpaWlpaWk=?=\n";
	char *res = NULL;

	mime_header_enc(&res, src, "utf-8", MIME_B64);

	assert_that(res, is_equal_to_string(exp));
	free(res);
}

Ensure(hdr_enc_does_not_break_utf8)
{
	char *src = "Subject: Это ваше ФИДО (переиздание)";
	char *exp = "Subject: =?utf-8?B?0K3RgtC+INCy0LDRiNC1INCk0JjQlNCeICjQv9C10YDQtQ==?=\n"
		" =?utf-8?B?0LjQt9C00LDQvdC40LUp?=\n";
	char *res = NULL;

	mime_header_enc(&res, src, "utf-8", MIME_B64);

	assert_that(res, is_equal_to_string(exp));
	free(res);
}

Ensure(hdr_enc_encodes_long_line)
{
	char *src = "Field: aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
	char *exp = "Field: =?windows-1251?B?YWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFh?=\n"
		" =?windows-1251?B?YWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFh?=\n"
		" =?windows-1251?B?YWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYQ==?=\n";
	char *res = NULL;

	mime_header_enc(&res, src, "windows-1251", MIME_B64);

	assert_that(res, is_equal_to_string(exp));
	free(res);
}

Ensure(hdr_enc_qp_encodes_cyrillic)
{
	char *src = "Subject: tеsт кириллица latinitsa, оdna, двeee, триiiiiiii";
	char *exp = "Subject: =?utf-8?Q?t=D0=B5s=D1=82=20=D0=BA=D0=B8=D1=80=D0=B8=D0=BB?=\n =?utf-8?Q?=D0=BB=D0=B8=D1=86=D0=B0?= latinitsa, =?utf-8?Q?=D0=BE?=\n =?utf-8?Q?dna,=20=D0=B4=D0=B2eee,=20=D1=82=D1=80=D0=B8iiiiiii?=\n";
	char *res = NULL;

	mime_header_enc(&res, src, "utf-8", MIME_QP);

	assert_that(res, is_equal_to_string(exp));
	free(res);
}

Ensure(hdr_enc_b64_wraps_reminder)
{
	/* =?utf-8?Q?XXXXxxxx?= 20 chars */
	char *src = "Subject: aaaaaaaaaaaaaaaaaaaaaa aaaaaaaaaaaaa aaaaa ббббб";
	char *exp = "Subject: aaaaaaaaaaaaaaaaaaaaaa aaaaaaaaaaaaa aaaaa =?utf-8?B?0LHQsQ==?=\n =?utf-8?B?0LHQsdCx?=\n";
	char *res = NULL;

	mime_header_enc(&res, src, "utf-8", MIME_B64);

	assert_that(res, is_equal_to_string(exp));
	free(res);
}

/*
 * Regression: on line-break inside a word where the current chunk has a
 * 1- or 2-byte remainder, the encoder must close `?=` and open a fresh
 * `=?utf-8?B?` on the next line. It must NOT emit `==` padding in the
 * middle of an encoded-word.
 */
Ensure(hdr_enc_b64_no_mid_word_padding)
{
	char *src = "Subject: Re: ааааааааа ааааааа - аааааааааааааааааааааааааа\n";
	char *res = NULL;

	mime_header_enc(&res, src, "utf-8", MIME_B64);

	/* should never contain `==` followed by more base64 in the same word */
	assert_that(strstr(res, "==0"), is_equal_to(NULL));
	assert_that(strstr(res, "==d"), is_equal_to(NULL));
	free(res);
}

Ensure(hdr_enc_b64_only_splits_plain)
{
	char *src = "Subject: aaaaaaaaaaaaaaaaaaaaaa aaaaaaaaaaaaa aaaaaaaa aaaaaa aaaaa aaaaaa aaaaaa";
	char *exp = "Subject: aaaaaaaaaaaaaaaaaaaaaa aaaaaaaaaaaaa aaaaaaaa aaaaaa aaaaa aaaaaa\n aaaaaa\n";
	char *res = NULL;

	mime_header_enc(&res, src, "utf-8", MIME_B64);

	assert_that(res, is_equal_to_string(exp));
	free(res);
}

Ensure(body_enc_b64_encodes_cyrillic)
{
	Textlist src;
	Textlist res;
	char *exp = "dNC1c9GCINC60LjRgNC40LvQu9C40YbQsA==\n";

	tl_init(&src);
	tl_init(&res);
	tl_append(&src, "tеsт кириллица");

	mime_b64_encode_tl(&src, &res);

	assert_that(res.first->line, is_equal_to_string(exp));

	tl_clear(&src);
	tl_clear(&res);
}

#define MSG_MAXSUBJ	72
#define J "\xe9" /* Cyrillic й cp1251 */
static char dres[MSG_MAXSUBJ];

Ensure(hdr_dec_decodes_le28chars_line)
{
	char *src = "=?UTF-8?B?0LnQudC50LnQudC50LnQudC50LnQudC50LnQudC50LnQudC50LnQudC5?=";
	char *exp =
		J J J J J J J J J J J J J J J J
		J J J J J; /* 21 */

	mime_header_dec(dres, sizeof(dres), src, "windows-1251",
			CHARSET_STDRFC, NULL);

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

	mime_header_dec(dres, sizeof(dres), src, "windows-1251",
			CHARSET_STDRFC, NULL);

	assert_that(dres, is_equal_to_string(exp));
}

Ensure(hdr_dec_decodes_nonmime_at_start)
{
	char *src = "Re: Re: =?utf-8?B?0LnQudC50LnQuQ==?=";
	char *exp = "Re: Re: " J J J J J;

	mime_header_dec(dres, sizeof(dres), src, "windows-1251",
			CHARSET_STDRFC, NULL);

	assert_that(dres, is_equal_to_string(exp));
}
#undef J

Ensure(hdr_dec_handles_empty)
{
	char *src = "";
	char *exp = "";

	mime_header_dec(dres, sizeof(dres), src, "ASCII",
			CHARSET_STDRFC, NULL);

	assert_that(dres, is_equal_to_string(exp));
}

Ensure(hdr_dec_preserves_non_mime)
{
	char *src =
		"abcdefghijklmnopqrstuvwxyz123456"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ123456"; /* 64 */
	char *exp = src;

	mime_header_dec(dres, sizeof(dres), src, "ASCII", CHARSET_STDRFC, NULL);

	assert_that(dres, is_equal_to_string(exp));
}

#define MIME_MAX_ENC_LEN 31

Ensure(hdr_dec_skips_long_charset_decoding)
{
	char *src = "=?UTF-8longlonglonglonglonglonglong?B?0LnQudC50LnQudC50LnQudC50LnQudC50LnQudC50LnQudC50LnQudC5?=";
        /* cut to 71 + '\0' symbols */
	char *exp = "=?UTF-8longlonglonglonglonglonglong?B?0LnQudC50LnQudC50LnQudC50LnQudC50";

	mime_header_dec(dres, sizeof(dres), src, "ASCII", CHARSET_STDRFC, NULL);

	assert_that(dres, is_equal_to_string(exp));
}

Ensure(hdr_dec_decodes_email1)
{
	char *src = "=?UTF-8?B?0KfQtdGC0LLQtdGA0LjQutC+0LIg0Jou0JIu?= <chetverikov@mann-schroeder.ru>";
	char *exp = "Четвериков К.В. <chetverikov@mann-schroeder.ru>";

	mime_header_dec(dres, sizeof(dres), src, "UTF-8", CHARSET_STDRFC, NULL);

	assert_that(dres, is_equal_to_string(exp));
}

Ensure(hdr_dec_decodes_email2)
{
	char *src = "chetverikov@mann-schroeder.ru (=?UTF-8?B?0KfQtdGC0LLQtdGA0LjQutC+0LIg0Jou0JIu?=)";
	char *exp = "chetverikov@mann-schroeder.ru (Четвериков К.В.)";

	mime_header_dec(dres, sizeof(dres), src, "UTF-8", CHARSET_STDRFC, NULL);

	assert_that(dres, is_equal_to_string(exp));
}

#define J "\xe9" /* Cyrillic й cp1251 */
Ensure(hdr_dec_decodes_qp_header)
{
	/* =D0=B9 is UTF-8 for й (Cyrillic) */
	char *src = "=?UTF-8?Q?=D0=B9=D0=B9=D0=B9=D0=B9=D0=B9?=";
	char *exp = J J J J J; /* 5 × й in cp1251 */

	mime_header_dec(dres, sizeof(dres), src, "windows-1251",
			CHARSET_STDRFC, NULL);

	assert_that(dres, is_equal_to_string(exp));
}
#undef J

Ensure(body_enc_qp_encodes_cyrillic)
{
	Textlist src;
	Textlist res;
	/* "tеsт кириллица" in UTF-8 quoted-printable body encoding */
	char *exp = "t=D0=B5s=D1=82=20=D0=BA=D0=B8=D1=80=D0=B8=D0=BB=D0=BB=D0=B8=D1=86=D0=B0\n";

	tl_init(&src);
	tl_init(&res);
	tl_append(&src, "tеsт кириллица");

	mime_qp_encode_tl(&src, &res);

	assert_that(res.first->line, is_equal_to_string(exp));

	tl_clear(&src);
	tl_clear(&res);
}

Ensure(dequote_handles_qp_and_underscores)
{
	char dst[32];
	/* =D0=B5 is UTF-8 for е, underscore decodes to space */
	char *src = "t=D0=B5s_test";
	char *exp = "t\xD0\xB5s test";

	mime_dequote(dst, sizeof(dst), src);

	assert_that(dst, is_equal_to_string(exp));
}

/*
 * RFC 2047 §2: "Both 'encoding' and 'charset' names are case-independent."
 * Lowercase 'b' must be treated the same as 'B'.
 */
#define J "\xe9" /* Cyrillic й cp1251 */
Ensure(hdr_dec_b_encoding_case_insensitive)
{
	/* lowercase ?b? instead of ?B? */
	char *src = "=?UTF-8?b?0LnQudC50LnQuQ==?=";
	char *exp = J J J J J; /* 5 й */

	mime_header_dec(dres, sizeof(dres), src, "windows-1251",
			CHARSET_STDRFC, NULL);

	assert_that(dres, is_equal_to_string(exp));
}

/*
 * RFC 2047 §2: case-independence applies to Q encoding as well.
 * Lowercase 'q' must be treated the same as 'Q'.
 */
Ensure(hdr_dec_q_encoding_case_insensitive)
{
	/* lowercase ?q? instead of ?Q? */
	char *src = "=?UTF-8?q?=D0=B9=D0=B9=D0=B9=D0=B9=D0=B9?=";
	char *exp = J J J J J; /* 5 й */

	mime_header_dec(dres, sizeof(dres), src, "windows-1251",
			CHARSET_STDRFC, NULL);

	assert_that(dres, is_equal_to_string(exp));
}
#undef J

/*
 * RFC 2047 §4.2 rule 2: "The 8-bit hexadecimal value 20 ... may be
 * represented as '_' (underscore)."
 * Underscore in a Q-encoded header word must decode to SPACE.
 */
Ensure(hdr_dec_q_underscore_is_space)
{
	char *src = "=?US-ASCII?Q?hello_world?=";
	char *exp = "hello world";

	mime_header_dec(dres, sizeof(dres), src, "UTF-8",
			CHARSET_STDRFC, NULL);

	assert_that(dres, is_equal_to_string(exp));
}

/*
 * RFC 2047 §6.2: "any 'linear-white-space' that separates a pair of
 * adjacent 'encoded-word's is ignored."
 * A SPACE between two encoded-words must not appear in the decoded output.
 */
Ensure(hdr_dec_whitespace_between_encoded_words_ignored)
{
	/* single SPACE separating two encoded-words on the same line */
	char *src = "=?US-ASCII?Q?hello?= =?US-ASCII?Q?world?=";
	char *exp = "helloworld";

	mime_header_dec(dres, sizeof(dres), src, "UTF-8",
			CHARSET_STDRFC, NULL);

	assert_that(dres, is_equal_to_string(exp));
}

static TestSuite *create_mime_suite(void)
{
    TestSuite *suite = create_named_test_suite(
	    "MIME suite");
    add_test(suite, hdr_enc_78_limit);
    add_test(suite, hdr_enc_encodes_cyrillic);
    add_test(suite, hdr_enc_encodes_cyrillic2);
    add_test(suite, hdr_enc_does_not_break_utf8);
    add_test(suite, hdr_enc_encodes_long_line);
    add_test(suite, hdr_enc_qp_encodes_cyrillic);
    add_test(suite, hdr_enc_b64_wraps_reminder);
    add_test(suite, hdr_enc_b64_no_mid_word_padding);
    add_test(suite, hdr_enc_b64_only_splits_plain);
    add_test(suite, body_enc_b64_encodes_cyrillic);
    add_test(suite, hdr_dec_decodes_le28chars_line);
    add_test(suite, hdr_dec_decodes_ge35chars_line);
    add_test(suite, hdr_dec_decodes_nonmime_at_start);
    add_test(suite, hdr_dec_handles_empty);
    add_test(suite, hdr_dec_preserves_non_mime);
    add_test(suite, hdr_dec_skips_long_charset_decoding);
    add_test(suite, hdr_dec_decodes_email1);
    add_test(suite, hdr_dec_decodes_email2);
    add_test(suite, hdr_dec_decodes_qp_header);
    add_test(suite, body_enc_qp_encodes_cyrillic);
    add_test(suite, dequote_handles_qp_and_underscores);
    add_test(suite, hdr_dec_b_encoding_case_insensitive);
    add_test(suite, hdr_dec_q_encoding_case_insensitive);
    add_test(suite, hdr_dec_q_underscore_is_space);
    add_test(suite, hdr_dec_whitespace_between_encoded_words_ignored);
    return suite;
}

int main(int argc, char **argv)
{
    TestSuite *suite = create_mime_suite();

    if (argc > 1)
	return run_single_test(suite, argv[1], create_text_reporter());

    return run_test_suite(suite, create_text_reporter());
}
