#include <cgreen/mocks.h>

static char *create_no_debug_exp(void)
{
    int uid = getuid();
    int euid = geteuid();
    char *exp;

    exp = create_string("debug called for uid=%d euid=%d, "
			"output disabled\n",
			uid, euid);
    return exp;
}

Ensure(debug_no_output_low_verbose) {
    int level = 10;

    assert_that(log_get_verbose(), is_less_than(level));

    never_expect(mock_vfprintf);
    never_expect(mock_fprintf);

    debug(level, "some debug");
}

Ensure(debug_default_output_stderr) {
    int level = -1;

    expect(mock_vfprintf,
	   when(stream, is_equal_to(stderr)));
    expect(mock_fprintf,
	   when(stream, is_equal_to(stderr)));
    expect(mock_fflush,
	   when(file, is_equal_to(stderr)));

    debug(level, "some debug");
}

Ensure(debug_sets_default_keeps_filename) {
    char exp_logname[] = "my cool name";

    log_set_log_filename(exp_logname);
    debug_default_output_stderr();
    assert_that(log_get_log_filename(),
		is_equal_to_string(exp_logname));
}

Ensure(debug_stub_no_debug) {
    char *exp;
    FILE *fp = stderr;
    int level = -1;

    exp = create_no_debug_exp();
    always_expect(mock_vfprintf,
		  when(stream, is_equal_to(fp)),
		  when(res_str, is_equal_to_string(exp)));
    never_expect(mock_fflush);

    log_file("stderr");
    log_suppress_debug();

    debug(level, "some debug");
    debug(level, "some debug");

    free(exp);
}

static void expect_debug_output(FILE *fp, const char *msg)
{
    expect(mock_vfprintf,
	   when(stream, is_equal_to(fp)),
	   when(res_str, is_equal_to_string(msg)));
    expect(mock_fprintf,
	   when(stream, is_equal_to(fp)),
	   when(res_str, is_equal_to_string("\n")));
    expect(mock_fflush,
	   when(file, is_equal_to(fp)));
}

Ensure(debug_output) {
    int level = -1;
    char my_debug[] = "some debug";

    expect_debug_output(stderr, my_debug);

    debug(level, my_debug);
}

#include "test-log-debug-syslog.c"

TestSuite *create_log_debug_suite(void)
{
    TestSuite *suite = create_named_test_suite("debug() suite");

    add_test(suite, debug_no_output_low_verbose);
    add_test(suite, debug_default_output_stderr);
    add_test(suite, debug_stub_no_debug);
    add_test(suite, debug_output);
    add_test(suite, debug_sets_default_keeps_filename);

    add_debug_syslog(suite);

    set_setup(suite, log_init_globals);

    return suite;
}
