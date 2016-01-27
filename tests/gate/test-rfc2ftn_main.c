/*
 */

#include "rfc2ftn.h"
#include "prototypes.h"
#include "mock-log.h"
#include <cgreen/cgreen.h>
#include <cgreen/mocks.h>

Ensure(rfc2ftn_dummy)
{
    char *args[] = {
	"rfc2ftn",
    };
    int r;

    r = rfc2ftn_main(1, args);
    assert_that(r, is_equal_to(0));
}

static void setup(void)
{
    log_buffer_release();
    debug_buffer_release();
    rfc2ftn_stdin = stdin;
}

TestSuite *create_rfc2ftn_main_suite(void)
{
    TestSuite *suite = create_named_test_suite("rfc2ftn_main suite");

    add_test(suite, rfc2ftn_dummy);

    set_setup(suite, setup);

    return suite;
}
