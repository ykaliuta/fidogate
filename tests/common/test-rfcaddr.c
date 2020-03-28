#include <cgreen/cgreen.h>
#include <fidogate.h>


/* John Doe (2:2/9999) -> John Doe <John_Doe@f9999.n2.z2.fidonet.org> */
/* john doe (2:2/9999) -> john doe <john_doe@f9999.n2.z2.fidonet.org> */
/* Василий Пупкин (2:2/9999) -> Василий Пупкин <sysop@f9999.n2.z2.fidonet.org> */

static char sysop[] = "sysop";

/* config private data */
struct st_zones {
    int zone;                   /* Zone */
    char *inet_domain;          /* Internet domain */
    char *ftn_domain;           /* FTN domain */
    char *out;                  /* Outbound subdir */
};
void cf_zones_add(struct st_zones *zone);
/* end of config private date */

static struct st_zones zone = {
    .zone = 2,
    .inet_domain = ".fidonet.org",
    .ftn_domain = "fidonet",
    .out = "out",
};

Ensure(fromrfc_passes_noncapitals)
{
    char *src = "real <name@example.com>";
    char *user = "name";
    char *addr = "example.com";
    char *real = "real";

    RFCAddr res;

    res = rfcaddr_from_rfc(src);

    assert_that(res.user, is_equal_to_string(user));
    assert_that(res.addr, is_equal_to_string(addr));
    assert_that(res.real, is_equal_to_string(real));
}

Ensure(fromftn_passes_capitals)
{
    Node node = {
        .zone = 2,
        .net = 2,
        .node = 999,
        .point = 0,
        .domain = "",
        .flags = 0,
    };
    char *name = "John Doe";

    char *user = "John_Doe";
    char *addr = "f999.n2.z2.fidonet.org";
    char *real = "John Doe";

    RFCAddr res;

    res = rfcaddr_from_ftn(name, &node);

    assert_that(res.user, is_equal_to_string(user));
    assert_that(res.addr, is_equal_to_string(addr));
    assert_that(res.real, is_equal_to_string(real));
}

Ensure(fromftn_passes_noncapitals)
{
    Node node = {
        .zone = 2,
        .net = 2,
        .node = 999,
        .point = 0,
        .domain = "",
        .flags = 0,
    };
    char *name = "john doe";

    char *user = "john_doe";
    char *addr = "f999.n2.z2.fidonet.org";
    char *real = "john doe";

    RFCAddr res;

    res = rfcaddr_from_ftn(name, &node);

    assert_that(res.user, is_equal_to_string(user));
    assert_that(res.addr, is_equal_to_string(addr));
    assert_that(res.real, is_equal_to_string(real));
}

Ensure(fromftn_replaces_nonascii)
{
    Node node = {
        .zone = 2,
        .net = 2,
        .node = 999,
        .point = 0,
        .domain = "",
        .flags = 0,
    };
    char *name = "Василий Пупкин";

    char *user = sysop;
    char *addr = "f999.n2.z2.fidonet.org";
    char *real = "Василий Пупкин";

    RFCAddr res;

    res = rfcaddr_from_ftn(name, &node);

    assert_that(res.user, is_equal_to_string(user));
    assert_that(res.addr, is_equal_to_string(addr));
    assert_that(res.real, is_equal_to_string(real));
}

static void rfcaddr_fromftn_setup(void)
{
    rfcaddr_fallback_username(sysop);
    cf_zones_add(&zone);
}

static TestSuite *create_my_suite(void)
{
    TestSuite *suite = create_named_test_suite("RFCAddr suite");

    add_test(suite, fromrfc_passes_noncapitals);

    add_test(suite, fromftn_passes_capitals);
    add_test(suite, fromftn_passes_noncapitals);
    add_test(suite, fromftn_replaces_nonascii);

    set_setup(suite, rfcaddr_fromftn_setup);
    return suite;
}

int main(int argc, char **argv)
{
    TestSuite *suite = create_my_suite();

    if (argc > 1)
	return run_single_test(suite, argv[1], create_text_reporter());

    return run_test_suite(suite, create_text_reporter());
}
