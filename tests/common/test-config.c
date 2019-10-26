#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <cgreen/cgreen.h>
#include <cgreen/mocks.h>

#include <config.c>

#include "config_cf_p_tests.h"

#include "mock-config.c"

#include "test-config-api.c"

char key1[] = "ConfigVar1";
char val1[] = "Value1";
char key2[] = "ConfigVar2";
char val2[] = "Value2";
char key3[] = "ConfigVar3";
char val3[] = "Value3";
char key_unexisting[] = "Some Key";

void fill_config(void)
{
    cf_add_string(key1, val1);
    cf_add_string(key2, val2);
    cf_add_string(key3, val3);
    cf_add_string(key1, val2); /* Different value */
    cf_add_string(key3, val2); /* Different value, end */
    /* leak the momory */
}


Ensure(config_inits)
{
    int env_owr = 1;

    setenv("FIDOGATE", "/tmp/mock-fidogate-dir", env_owr);
    expect(cf_s_configdir);

    cf_initialize();
}

GEN_CF_P_TESTS(origin, "FIDOGATE", "Origin")
GEN_CF_P_TESTS(organization, "FIDOGATE", "Organization")

Ensure(get_string_default_null)
{
    char *ret;

    ret = cf_get_string("some_var", TRUE);
    assert_that(ret, is_null);

    ret = cf_get_string("some_var", FALSE);
    assert_that(ret, is_null);
}

Ensure(get_string_finds_var)
{
    char *ret;
    char *exp = val2;

    fill_config();

    ret = cf_get_string(key2, TRUE);
 
    assert_that(ret, is_equal_to_string(exp));
}

Ensure(get_string_doesnt_find_unexisting)
{
    char *ret;

    fill_config();

    ret = cf_get_string(key_unexisting, TRUE);

    assert_that(ret, is_null);
}

Ensure(get_string_finds_same_var_from_start)
{
    char *ret1;
    char *ret2;

    fill_config();

    ret1 = cf_get_string(key1, TRUE);
    ret2 = cf_get_string(key1, TRUE);

    assert_that(ret1, is_equal_to_string(ret2));
}

Ensure(get_string_finds_different_when_continue)
{
    char *ret1;
    char *ret2;

    fill_config();

    ret1 = cf_get_string(key1, TRUE);
    ret2 = cf_get_string(key1, FALSE);

    assert_that(ret1, is_not_equal_to_string(ret2));
}

TestSuite *create_config_suite(void)
{
    TestSuite *suite = create_named_test_suite(
	"Config suite");

    add_test(suite, config_inits);
    ADD_CF_P_TESTS(suite, origin);
    ADD_CF_P_TESTS(suite, organization);
    add_test(suite, get_string_default_null);
    add_test(suite, get_string_finds_var);
    add_test(suite, get_string_doesnt_find_unexisting);
    add_test(suite, get_string_finds_same_var_from_start);
    add_test(suite, get_string_finds_different_when_continue);

    return suite;
}

int main(int argc, char **argv) {
    TestSuite *suite = create_config_suite();

    cgreen_mocks_are(loose_mocks);
    return run_test_suite(suite, create_text_reporter());
}
