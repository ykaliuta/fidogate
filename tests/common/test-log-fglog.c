#include <cgreen/mocks.h>
#include <string.h>

static const int do_expand = TRUE;
static const int do_not_expand = FALSE;

static void expect_config_lookup(const char *ret_from_config,
				 int do_expand)
{
    int len = strlen(ret_from_config) + 1;

    expect(cf_p_logfile,
	   will_return(ret_from_config));
    if (!do_expand)
	return;

    expect(str_expand_name,
	   when(s, is_equal_to(ret_from_config)),
	   will_set_contents_of_parameter(d,
					  ret_from_config,
					  len),
	   will_return(ret_from_config));
}

/*
 * Since the functions are mocked, it's needed to check mocks.
 * Otherwise, complains, that mocked function call wasn't expected.
 * So, we cannot call just log_file()
 */

static void expect_file_config_lookup(void)
{
    expect_config_lookup(config_file, do_expand);
}

static void expect_stdout_config_lookup(void)
{
    expect_config_lookup(config_stdout, do_not_expand);
}

static void *expect_output_beginning(FILE *fp,
				     const char *msg, const char *name)
{
    char *exp1;

    exp1 = create_string("%s %s ", date_const, name);

    expect(mock_fprintf,
	   when(res_str, is_equal_to_string(exp1)),
	   when(stream, is_equal_to(fp)));
    expect(mock_vfprintf,
	   when(res_str,
		is_equal_to_string(msg)),
	   when(stream, is_equal_to(fp)));
    return exp1;
}

static void *expect_full_output(FILE *fp,
				const char *msg, const char *name)
{
    void *to_free;

    to_free = expect_output_beginning(fp, msg, name);

    expect(mock_fprintf,
	   when(res_str, is_equal_to_string("\n")),
	   when(stream, is_equal_to(fp)));
    return to_free;
}

static void *expect_full_output_file(FILE *fp,
				     const char *msg,
				     const char *name,
				     const char *log_filename)
{
    void *to_free;

    expect(mock_fopen,
	   when(path, is_equal_to_string(log_filename)),
	   when(mode, is_equal_to_string(A_MODE)),
	   will_return(fp));

    to_free = expect_full_output(fp, msg, name);
    expect(mock_fclose,
	   when(file, is_equal_to(fp)));
    return to_free;
}

static void expect_full_errno_output(FILE *fp,
				     const char *msg, const char *name,
				     void **to_free1, void **to_free2)
{
    void *to_free;
    char *exp;

    to_free = expect_output_beginning(fp, msg + 1, name);

    exp = create_string(" (errno=%d: %m)", errno);

    expect(mock_fprintf,
	   when(res_str, is_equal_to_string(exp)),
	   when(stream, is_equal_to(fp)));

    expect(mock_fprintf,
	   when(res_str, is_equal_to_string("\n")),
	   when(stream, is_equal_to(fp)));
    *to_free1 = to_free;
    *to_free2 = exp;
}

Ensure(fglog_sets_file_from_config) {
    const char *res;

    expect_file_config_lookup();

    expect(mock_fprintf);
    expect(mock_fopen);

    fglog(normal_log_msg);

    res = log_get_filename();

    assert_that(res, is_equal_to_string(config_file));
}

Ensure(fglog_does_not_change_file) {
    const char *res;

    expect_file_config_lookup();

    /* will set to config_file */
    log_file(cf_p_logfile());

    never_expect(cf_p_logfile); /* not anymore */

    expect(mock_fprintf);
    expect(mock_fopen);

    fglog(normal_log_msg);

    res = log_get_filename();

    assert_that(res, is_equal_to_string(config_file));
}

Ensure(fglog_does_simple_output_stdout) {
    FILE *fp = stdout;
    void *to_free;

    expect_stdout_config_lookup();

    to_free = expect_full_output(fp, normal_log_msg, default_name);

    fglog(normal_log_msg);
    free(to_free);
}

Ensure(fglog_does_errno_output_stdout) {
    FILE *fp = stdout;
    void *to_free1;
    void *to_free2;

    expect_stdout_config_lookup();

    expect_full_errno_output(fp, errno_log_msg, default_name,
			     &to_free1, &to_free2);

    fglog(errno_log_msg);

    free(to_free2);
    free(to_free1);
}

Ensure(fglog_does_simple_output_file) {
    FILE *fp = dummy_fp;
    char *to_free;

    expect_file_config_lookup();

    to_free = expect_full_output_file(fp,
				      normal_log_msg, default_name,
				      config_file);

    fglog(normal_log_msg);
    free(to_free);
}

Ensure(fglog_prints_for_wrong_file) {
    char *exp1;
    const char *exp_log_filename = config_file;
    int new_errno = ECONNRESET;
    const char *exp_log_prog = default_name;

    expect_file_config_lookup();

    expect(mock_fopen,
	   when(path, is_equal_to_string(exp_log_filename)),
	   when(mode, is_equal_to_string(A_MODE)),
	   will_set_contents_of_parameter(local_errno,
					  &new_errno,
					  sizeof(int)),
	   will_return(NULL));

    exp1 = create_string("%s WARNING: can't open log file %s "
			 "(errno=%d: %s)\n",
			 exp_log_prog, exp_log_filename,
			 new_errno, strerror(new_errno));

    expect(mock_fprintf,
	   when(res_str, is_equal_to_string(exp1)),
	   when(stream, is_equal_to(stderr)));
    never_expect(mock_vfprintf);
    never_expect(mock_fprintf);
    never_expect(mock_fclose);

    fglog(normal_log_msg);

    free(exp1);
}

Ensure(fglog_resets_default_verbose_for_wrong_file)
{
    expect_file_config_lookup();

    expect(mock_fopen,
	   will_return(NULL));
    expect(mock_fprintf);
    log_set_verbose(0);

    fglog(normal_log_msg);
    assert_that(log_get_verbose(), is_equal_to(-1));
}

Ensure(fglog_ignores_high_verbose_for_wrong_file)
{
    expect_file_config_lookup();

    expect(mock_fopen,
	   will_return(NULL));
    expect(mock_fprintf);
    log_set_verbose(10);

    fglog(normal_log_msg);
    assert_that(log_get_verbose(), is_equal_to(10));
}

Ensure(fglog_opens_file_every_time)
{
    FILE *fp = dummy_fp;
    const char *exp_log_filename = config_file;

    expect_file_config_lookup();

    expect(mock_fopen,
	   when(path, is_equal_to_string(exp_log_filename)),
	   when(mode, is_equal_to_string(A_MODE)),
	   will_return(fp));

    expect(mock_fprintf);
    expect(mock_vfprintf);
    expect(mock_fprintf);
    expect(mock_fclose);

    /* Second time */
    expect(mock_fopen,
	   when(path, is_equal_to_string(exp_log_filename)),
	   when(mode, is_equal_to_string(A_MODE)),
	   will_return(fp));

    expect(mock_fprintf);
    expect(mock_vfprintf);
    expect(mock_fprintf);
    expect(mock_fclose);

    fglog(normal_log_msg);
    fglog(normal_log_msg);
}

Ensure(fglog_outputs_after_log_program)
{
    FILE *fp = stdout;
    void *to_free;

    expect_stdout_config_lookup();
    to_free = expect_full_output(fp, normal_log_msg, custom_name);

    log_program(custom_name);
    fglog(normal_log_msg);

    free(to_free);
}

Ensure(fglog_outputs_after_log_file_stdout)
{
    FILE *fp = stdout;
    void *to_free;

    never_expect(cf_p_logfile);
    to_free = expect_full_output(fp, normal_log_msg, default_name);

    log_file("stdout");
    fglog(normal_log_msg);

    free(to_free);
}

Ensure(fglog_outputs_after_log_file_file)
{
    FILE *fp = dummy_fp;
    void *to_free;

    never_expect(cf_p_logfile);
    expect(str_expand_name,
	   when(s, is_equal_to(config_file)),
	   will_set_contents_of_parameter(d,
					  config_file,
					  sizeof(config_file)),
	   will_return(config_file));
    to_free = expect_full_output_file(fp,
				      normal_log_msg, default_name,
				      config_file);
    log_file(config_file);
    fglog(normal_log_msg);

    free(to_free);
}

Ensure(fglog_outputs_after_log_program_log_file_stdout)
{
    FILE *fp = stdout;
    void *to_free;

    never_expect(cf_p_logfile);
    to_free = expect_full_output(fp, normal_log_msg, custom_name);

    log_program(custom_name);
    log_file("stdout");
    fglog(normal_log_msg);

    free(to_free);
}

Ensure(fglog_outputs_after_log_program_log_file_file)
{
    FILE *fp = dummy_fp;
    void *to_free;

    never_expect(cf_p_logfile);
    expect(str_expand_name,
	   when(s, is_equal_to(config_file)),
	   will_set_contents_of_parameter(d,
					  config_file,
					  sizeof(config_file)),
	   will_return(config_file));
    to_free = expect_full_output_file(fp,
				      normal_log_msg, custom_name,
				      config_file);

    log_program(custom_name);
    log_file(config_file);
    fglog(normal_log_msg);

    free(to_free);
}

Ensure(fglog_outputs_after_log_file_stdout_log_program)
{
    FILE *fp = stdout;
    void *to_free;

    never_expect(cf_p_logfile);
    to_free = expect_full_output(fp, normal_log_msg, custom_name);

    log_file("stdout");
    log_program(custom_name);
    fglog(normal_log_msg);

    free(to_free);
}

Ensure(fglog_outputs_after_log_file_file_log_program)
{
    FILE *fp = dummy_fp;
    void *to_free;

    never_expect(cf_p_logfile);
    expect(str_expand_name,
	   when(s, is_equal_to(config_file)),
	   will_set_contents_of_parameter(d,
					  config_file,
					  sizeof(config_file)),
	   will_return(config_file));
    to_free = expect_full_output_file(fp,
				      normal_log_msg, custom_name,
				      config_file);
    log_file(config_file);
    log_program(custom_name);
    fglog(normal_log_msg);

    free(to_free);
}

#include "test-log-fglog-syslog.c"

TestSuite *create_log_fglog_suite(void)
{
    TestSuite *suite = create_named_test_suite("fglog() suite");

    add_test(suite, fglog_sets_file_from_config);
    add_test(suite, fglog_does_not_change_file);
    add_test(suite, fglog_does_simple_output_stdout);
    add_test(suite, fglog_does_errno_output_stdout);
    add_test(suite, fglog_does_simple_output_file);
    add_test(suite, fglog_prints_for_wrong_file);
    add_test(suite, fglog_resets_default_verbose_for_wrong_file);
    add_test(suite, fglog_ignores_high_verbose_for_wrong_file);
    add_test(suite, fglog_opens_file_every_time);
    add_test(suite, fglog_outputs_after_log_program);
    add_test(suite, fglog_outputs_after_log_file_stdout);
    add_test(suite, fglog_outputs_after_log_file_file);
    add_test(suite, fglog_outputs_after_log_program_log_file_stdout);
    add_test(suite, fglog_outputs_after_log_program_log_file_file);
    add_test(suite, fglog_outputs_after_log_file_stdout_log_program);
    add_test(suite, fglog_outputs_after_log_file_file_log_program);

    add_fglog_syslog(suite);

    set_setup(suite, log_init_globals);

    return suite;
}
