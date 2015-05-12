#include <cgreen/mocks.h>

/* syslog calls mocked via mock_vfprintf, so expect that too */

#if !defined(HAVE_SYSLOG) || !defined(HAVE_SYSLOG_H)
void add_debug_syslog(TestSuite *suite)
{
}
#else
static int exp_priority = LOG_DEBUG;

Ensure(debug_uses_printf_if_no_use_syslog)
{
    never_expect(mock_syslog);
    never_expect(mock_vsyslog);
    never_expect(mock_openlog);

    always_expect(mock_fprintf);
    always_expect(mock_vfprintf);
    always_expect(mock_fflush);

    log_file("stderr");
    debug(syslog_debug_level, normal_log_msg);
}

Ensure(debug_stub_no_debug_syslog) {
    char *exp;

    exp = create_no_debug_exp();

    expect(mock_openlog);
    always_expect(mock_vsyslog,
		  when(priority, is_equal_to(exp_priority)));
    always_expect(mock_vfprintf,
		  when(stream, is_equal_to(NULL)),
		  when(res_str, is_equal_to_string(exp)));

    log_file("syslog");
    log_suppress_debug();
    debug(syslog_debug_level, normal_log_msg);
    debug(syslog_debug_level, normal_log_msg);

    free(exp);
}

Ensure(debug_opens_syslog_first_time)
{
    expect(mock_openlog,
	   when(ident, is_equal_to_string(default_name)),
	   when(option, is_equal_to(LOG_PID)),
	   when(facility, is_equal_to(FACILITY)));
    always_expect(mock_vsyslog);
    always_expect(mock_vfprintf);

    log_file("syslog");
    debug(syslog_debug_level, normal_log_msg);
}

Ensure(debug_doesnot_open_syslog_after_debug)
{
    expect(mock_openlog);
    never_expect(mock_openlog);
    always_expect(mock_vsyslog);
    always_expect(mock_vfprintf);

    log_file("syslog");
    debug(syslog_debug_level, normal_log_msg);
    debug(syslog_debug_level, normal_log_msg);
}

Ensure(debug_doesnot_open_syslog_after_fglog)
{
    expect(mock_openlog);
    never_expect(mock_openlog);
    always_expect(mock_vsyslog);
    always_expect(mock_vfprintf);

    log_file("syslog");
    fglog(normal_log_msg);
    debug(syslog_debug_level, normal_log_msg);
}

Ensure(debug_simple_output_syslog)
{
    expect(mock_openlog);
    expect(mock_vsyslog,
	   when(priority, is_equal_to(LOG_DEBUG)));

    expect(mock_vfprintf,
	   when(stream, is_equal_to(NULL)),
	   when(res_str, is_equal_to_string(normal_log_msg)));

    log_file("syslog");
    debug(syslog_debug_level, normal_log_msg);
}

void add_debug_syslog(TestSuite *suite)
{
    add_test(suite, debug_uses_printf_if_no_use_syslog);
    add_test(suite, debug_stub_no_debug_syslog);
    add_test(suite, debug_opens_syslog_first_time);
    add_test(suite, debug_doesnot_open_syslog_after_debug);
    add_test(suite, debug_doesnot_open_syslog_after_fglog);
    add_test(suite, debug_simple_output_syslog);
}

#endif /* HAVE_SYSLOG HAVE_SYSLOG_H */
