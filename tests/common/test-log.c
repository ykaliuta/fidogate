#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <cgreen/cgreen.h>
#include <cgreen/mocks.h>
#include <errno.h>

#include "mock-stdio.c"
#include "mock-syslog.c"

#include <log.c>

static const char date_const[] = "Stub date";
static const char default_name[] = "FIDOGATE";
static char custom_name[] = "PROGRAM";

static const char normal_log_msg[] = "some log";
static const char errno_log_msg[]  = "$some log";

static char config_file[] = "mocked_file";
static char config_stdout[] = "stdout";

#if defined(HAVE_SYSLOG) && defined(HAVE_SYSLOG_H)
static int syslog_debug_level = -1;
#endif

static FILE *dummy_fp = (FILE *)0x12345678;

#include "test-log-api.c"

/* mocks */
char *date_buf(char *buf, size_t len, char *fmt, time_t *t, long tz)
{
    strncpy(buf, date_const, len);
    buf[len - 1] = '\0';
    return buf;
}

char *str_copy( char *d, size_t n, char *s ) {

    memccpy( ( void * )d, ( void * )s, 0, n );
    d[n-1] = '\0';

    return d;
}

char *cf_p_logfile(void)
{
    return (char *)mock();
}

char *str_expand_name(char *d, size_t n, char *s)
{
    return (char *)mock(d, n, s);
}

/* ----------- end of mocks -------------- */

static char *create_string(const char *format, ...)
{
    char *ret;
    va_list args;
    int rc;

    va_start(args, format);

    rc = vasprintf(&ret, format, args);
    assert_that(rc, is_not_equal_to(-1));

    va_end(args);
    return ret;
}
Ensure(default_progname) {
    const char *res;

    res = log_get_program();
    assert_that(res, is_equal_to_string(default_name));
}

Ensure(log_program_sets_prog) {
    char str[] = "the name";
    const char *res;

    log_program(str);
    res = log_get_program();
    assert_that(res, is_equal_to_string(str));
}

#include "test-log-fglog.c"
#include "test-log-debug.c"

TestSuite *create_log_suite(void)
{
    TestSuite *suite = create_named_test_suite(
	"Logger suite");
    TestSuite *fglog_suite = create_log_fglog_suite();
    TestSuite *debug_suite = create_log_debug_suite();

    add_test(suite, default_progname);
    add_test(suite, log_program_sets_prog);

    add_suite(suite, fglog_suite);
    add_suite(suite, debug_suite);

    return suite;
}

int main(int argc, char **argv) {
    TestSuite *suite = create_log_suite();
    return run_test_suite(suite, create_text_reporter());
}
