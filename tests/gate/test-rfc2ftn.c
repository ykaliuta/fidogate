/*
 *  test-rfc2ftn.c
 */

#include "rfc2ftn.h"
#include "prototypes.h"
#include <cgreen/cgreen.h>

TestSuite *create_cvt_user_name_suite(void);

int snd_mail(RFCAddr rfc_to, Textlist *body, long size);

Ensure(dummy)
{
    RFCAddr to = { 0 };
    Textlist body = { 0 };
    int r;
    long size = 1000;

    r = snd_mail(to, &body, size);
    assert_that(r, is_equal_to(0));
}

static TestSuite *create_rfc2ftn_suite(void)
{
    TestSuite *suite = create_named_test_suite(
	"rfc2ftn suite");
    TestSuite *sub;

    add_test(suite, dummy);

    sub = create_cvt_user_name_suite();
    add_suite(suite, sub);

    return suite;
}

int main(int argc, char **argv)
{
    TestSuite *suite = create_rfc2ftn_suite();
    return run_test_suite(suite, create_text_reporter());
}

