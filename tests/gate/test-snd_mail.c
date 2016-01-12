/*
 */

#include "rfc2ftn.h"
#include "prototypes.h"
#include <cgreen/cgreen.h>

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


TestSuite *create_snd_mail_suite(void)
{
    TestSuite *suite = create_named_test_suite("snd_mail suite");

    add_test(suite, dummy);

    return suite;
}
