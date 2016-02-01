/*
 */

#include "rfc2ftn.h"
#include "prototypes.h"
#include "mock-log.h"
#include "buffer2c.h"
#include <cgreen/cgreen.h>
#include <cgreen/mocks.h>
#include <stdio.h>

#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

static FILE *fake_stdin;
static FILE *fake_pkt;
static char *pkt_buf;
static size_t pkt_size;

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

    expect(acl_ngrp_lookup,
	   will_return(1));
    expect(pkt_open,
	   will_return(fake_pkt));

    r = rfc2ftn_main(ARRAY_SIZE(args), args);
    assert_that(r, is_equal_to(0));
}

static void setup_stdin(char *buf)
{
    size_t size = strlen(buf) + 1;

    fake_stdin = fmemopen(buf, size, "r");
    assert_that(fake_stdin, is_not_equal_to(NULL));
    rfc2ftn_stdin = fake_stdin;
}

static void setup_pkt_buffer()
{
    fake_pkt = open_memstream(&pkt_buf, &pkt_size);
    assert_that(fake_stdin, is_not_equal_to(NULL));
}

static void setup(void)
{
    log_buffer_release();
    debug_buffer_release();

    setup_stdin(rfc_message);
    setup_pkt_buffer();
}

static void teardown(void)
{
    char *b;

    printf("DEBUG: %s\n", debug_buffer());
    printf("LOG: %s\n", log_buffer());
    fclose(fake_pkt);
    b = buffer2c(pkt_buf);
    printf("PKT (size %zd): %s\n", pkt_size, b);
    free(pkt_buf);
    free(b);
}

TestSuite *create_rfc2ftn_main_suite(void)
{
    TestSuite *suite = create_named_test_suite("rfc2ftn_main suite");

    add_test(suite, rfc2ftn_dummy);

    set_setup(suite, setup);
    set_teardown(suite, teardown);

    return suite;
}
