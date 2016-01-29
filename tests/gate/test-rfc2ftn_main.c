/*
 */

#include "rfc2ftn.h"
#include "prototypes.h"
#include "mock-log.h"
#include <cgreen/cgreen.h>
#include <cgreen/mocks.h>
#include <stdio.h>

#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

static FILE *fake_stdin;

static char *rfc_message =
    "From: one@two.com\n"
    "To: someuser@somewhere.org\n"
    "Subject: SUBJ\n"
    "\n"
    "Test\n";

Ensure(rfc2ftn_dummy)
{
    char *args[] = {
	"rfc2ftn",
	"fido.user@p1.f22.n333.z2.fidonet.org"
    };
    int r;

    r = rfc2ftn_main(ARRAY_SIZE(args), args);
    assert_that(r, is_equal_to(0));
}

static void setup_stdin(char *buf)
{
    size_t size = strlen(buf) + 1;

    fake_stdin = fmemopen(buf, size, "r");
    assert_that(fake_stdin, is_not_equal_to(NULL));
}

static void setup(void)
{
    log_buffer_release();
    debug_buffer_release();

    setup_stdin(rfc_message);
    rfc2ftn_stdin = fake_stdin;
}

TestSuite *create_rfc2ftn_main_suite(void)
{
    TestSuite *suite = create_named_test_suite("rfc2ftn_main suite");

    add_test(suite, rfc2ftn_dummy);

    set_setup(suite, setup);

    return suite;
}
