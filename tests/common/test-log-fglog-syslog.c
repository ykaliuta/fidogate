#include <cgreen/mocks.h>

/* syslog calls mocked via mock_vfprintf, so expect that too */

#if !defined(HAVE_SYSLOG) || !defined(HAVE_SYSLOG_H)
void add_fglog_syslog(TestSuite *suite)
{
}
#else

Ensure(fglog_uses_printf_if_no_use_syslog)
{
    never_expect(mock_syslog);
    never_expect(mock_vsyslog);
    never_expect(mock_openlog);

    always_expect(mock_fprintf);
    always_expect(mock_vfprintf);
    always_expect(mock_fclose);

    log_file("stderr");
    fglog(normal_log_msg);
}

Ensure(fglog_opens_syslog_first_time)
{
    expect(mock_openlog,
	   when(ident, is_equal_to_string(default_name)),
	   when(option, is_equal_to(LOG_PID)),
	   when(facility, is_equal_to(FACILITY)));
    always_expect(mock_vsyslog);
    always_expect(mock_vfprintf);

    log_file("syslog");
    fglog(normal_log_msg);
}

Ensure(fglog_doesnot_open_syslog_after_debug)
{
    expect(mock_openlog);
    never_expect(mock_openlog);
    always_expect(mock_vsyslog);
    always_expect(mock_vfprintf);

    log_file("syslog");
    debug(syslog_debug_level, normal_log_msg);
    fglog(normal_log_msg);
}

Ensure(fglog_doesnot_open_syslog_after_fglog)
{
    expect(mock_openlog);
    never_expect(mock_openlog);
    always_expect(mock_vsyslog);
    always_expect(mock_vfprintf);

    log_file("syslog");
    fglog(normal_log_msg);
    fglog(normal_log_msg);
}

Ensure(fglog_simple_output_syslog)
{
    expect(mock_openlog);
    expect(mock_vsyslog,
	   when(priority, is_equal_to(LOG_NOTICE)));

    expect(mock_vfprintf,
	   when(stream, is_equal_to(NULL)),
	   when(res_str, is_equal_to_string(normal_log_msg)));

    log_file("syslog");
    fglog(normal_log_msg);
}

Ensure(fglog_errno_output_syslog)
{
    char *exp;

    expect(mock_openlog);
    expect(mock_vsyslog,
	   when(priority, is_equal_to(LOG_NOTICE)));

    expect(mock_vfprintf,
	   when(stream, is_equal_to(NULL)),
	   when(res_str, is_equal_to_string(errno_log_msg + 1)));

    exp = create_string("(errno=%d: %m)", errno);
    expect(mock_syslog,
	   when(priority, is_equal_to(LOG_NOTICE)));
    expect(mock_vfprintf,
	   when(stream, is_equal_to(NULL)),
	   when(res_str, is_equal_to_string(exp)));

    log_file("syslog");
    fglog(errno_log_msg);

    free(exp);
}

Ensure(fglog_outputs_after_log_file_syslog)
{
    expect(mock_openlog,
	   when(ident, is_equal_to_string(default_name)));
    expect(mock_vsyslog,
	   when(priority, is_equal_to(LOG_NOTICE)));

    expect(mock_vfprintf,
	   when(stream, is_equal_to(NULL)),
	   when(res_str, is_equal_to_string(normal_log_msg)));

    log_file("syslog");
    fglog(normal_log_msg);
}

Ensure(fglog_outputs_after_log_program_log_file_syslog)
{
    expect(mock_openlog,
	   when(ident, is_equal_to_string(custom_name)));
    expect(mock_vsyslog,
	   when(priority, is_equal_to(LOG_NOTICE)));

    expect(mock_vfprintf,
	   when(stream, is_equal_to(NULL)),
	   when(res_str, is_equal_to_string(normal_log_msg)));

    log_program(custom_name);
    log_file("syslog");
    fglog(normal_log_msg);
}

Ensure(fglog_outputs_after_log_file_syslog_log_program)
{
    expect(mock_openlog,
	   when(ident, is_equal_to_string(default_name)));
    expect(mock_openlog,
	   when(ident, is_equal_to_string(custom_name)));
    expect(mock_vsyslog,
	   when(priority, is_equal_to(LOG_NOTICE)));

    expect(mock_vfprintf,
	   when(stream, is_equal_to(NULL)),
	   when(res_str, is_equal_to_string(normal_log_msg)));

    log_file("syslog");
    log_program(custom_name);
    fglog(normal_log_msg);
}

void add_fglog_syslog(TestSuite *suite)
{
    add_test(suite, fglog_uses_printf_if_no_use_syslog);
    add_test(suite, fglog_opens_syslog_first_time);
    add_test(suite, fglog_doesnot_open_syslog_after_debug);
    add_test(suite, fglog_doesnot_open_syslog_after_fglog);
    add_test(suite, fglog_simple_output_syslog);
    add_test(suite, fglog_errno_output_syslog);
    add_test(suite, fglog_outputs_after_log_file_syslog);
    add_test(suite, fglog_outputs_after_log_program_log_file_syslog);
    add_test(suite, fglog_outputs_after_log_file_syslog_log_program);
}

#endif /* HAVE_SYSLOG HAVE_SYSLOG_H */
