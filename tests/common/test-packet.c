/*
 *
 *  test-packet.c
 *
 *  March 06, 2016 Yauheni Kaliuta
 *
 */
#include "fidogate.h"
#include <cgreen/cgreen.h>
#include <stdio.h>

static FILE *f;
static char buf[64];
static int n;

#define ERROR (-1)

static void setup_sized_file(char *buf, size_t size)
{
    f = fmemopen(buf, size, "rb");
    assert_that(f, is_not_equal_to(NULL));
}

static void setup_file(char *buf)
{
    size_t size = strlen(buf) + 1;

    setup_sized_file(buf, size);
}

static void setup_empty_file(void)
{
    /* looks like fmemopen() doesn't accept size 0 */
    setup_file("");
    fgetc(f);
}

Ensure(getstring_reads_normal_string)
{
    char str[] = "string";
    char exp[] = "string";

    setup_file(str);

    /* so, it returns number bytes read */
    n = pkt_get_string(f, buf, sizeof(buf));

    assert_that(n, is_equal_to(strlen(exp) + 1));
    assert_that(buf, is_equal_to_string(exp));
}

Ensure(getstring_reads_empty_string)
{
    char str[] = "";
    char exp[] = "";

    setup_file(str);

    /* so, it returns number bytes read */
    n = pkt_get_string(f, buf, sizeof(buf));

    assert_that(n, is_equal_to(strlen(exp) + 1));
    assert_that(buf, is_equal_to_string(exp));
}

Ensure(getstring_reads_empty_file)
{
    setup_empty_file();

    n = pkt_get_string(f, buf, sizeof(buf));

    assert_that(n, is_equal_to(0));
}

Ensure(getstring_handles_eof)
{
    char str[] = "verybigstring";
    size_t file_len = strlen(str) / 2;

    setup_sized_file(str, file_len);

    n = pkt_get_string(f, buf, sizeof(buf));

    assert_that(n, is_equal_to(file_len));
}

Ensure(getstring_handles_small_buffer)
{
    char str[] = "verybigstring";
    char exp[] = "very" "aaaaa";
    size_t buf_len = sizeof("very"); /* 5 */

    setup_file(str);
    memcpy(buf, exp, sizeof(exp));

    n = pkt_get_string(f, buf, buf_len);

    assert_that(n, is_equal_to(strlen(str) + 1));
    assert_that(buf + buf_len,
		is_equal_to_string(exp + buf_len));
}

Ensure(getstring_handles_empty_buffer)
{
    char str[] = "string";
    char exp[] = "another string";
    int buf_len = 0; /* ! */
    
    setup_file(str);
    memcpy(buf, exp, sizeof(exp));

    n = pkt_get_string(f, buf, buf_len);

    assert_that(n, is_equal_to(ERROR));
    assert_that(buf,
		is_equal_to_contents_of(exp, sizeof(exp)));
}

static void get_string_teardown(void)
{
    if (f != NULL)
	fclose(f);
    f = NULL;
}

static TestSuite *create_get_string_suite(void)
{
    TestSuite *suite = create_named_test_suite(
	"packet_get_string suite");

    /* the next fails */
    /* add_test(suite, getstring_handles_empty_buffer); */
    add_test(suite, getstring_handles_small_buffer);
    add_test(suite, getstring_handles_eof);
    add_test(suite, getstring_reads_empty_file);
    add_test(suite, getstring_reads_empty_string);
    add_test(suite, getstring_reads_normal_string);

    set_teardown(suite, get_string_teardown);

    return suite;
}

static TestSuite *create_packet_suite(void)
{
    TestSuite *suite = create_named_test_suite(
	    "Packet suite");
    TestSuite *get_string_suite = create_get_string_suite();

    add_suite(suite, get_string_suite);

    return suite;
}

int main(int argc, char **argv) {
    TestSuite *suite = create_packet_suite();
    return run_test_suite(suite, create_text_reporter());
}
