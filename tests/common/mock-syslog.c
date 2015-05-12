#include <cgreen/mocks.h>

#if defined(HAVE_SYSLOG) && defined(HAVE_SYSLOG_H)

static void mock_openlog(const char *ident, int option, int facility)
{
    mock(ident, option, facility);
}

static void mock_syslog(int priority, const char *format, ...)
{
    va_list args;

    va_start(args, format);
    mock(priority, format, args);
    mock_vfprintf(NULL, format, args);
    va_end(args);
}

static void mock_vsyslog(int priority,
			 const char *format, va_list args)
{
    mock(priority, format, args);
    mock_vfprintf(NULL, format, args);
}

#define openlog mock_openlog
#define syslog mock_syslog
#define vsyslog mock_vsyslog
#endif
