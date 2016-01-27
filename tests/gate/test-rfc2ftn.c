/*
 *  test-rfc2ftn.c
 */

#include <cgreen/cgreen.h>

TestSuite *create_cvt_user_name_suite(void);
TestSuite *create_snd_mail_suite(void);
TestSuite *create_rfc2ftn_main_suite(void);

static TestSuite *create_rfc2ftn_suite(void)
{
    TestSuite *suite = create_named_test_suite(
	"rfc2ftn suite");
    TestSuite *sub;

    sub = create_cvt_user_name_suite();
    add_suite(suite, sub);
    sub = create_snd_mail_suite();
    add_suite(suite, sub);
    sub = create_rfc2ftn_main_suite();
    add_suite(suite, sub);

    return suite;
}

int main(int argc, char **argv)
{
    TestSuite *suite = create_rfc2ftn_suite();
    return run_test_suite(suite, create_text_reporter());
}

