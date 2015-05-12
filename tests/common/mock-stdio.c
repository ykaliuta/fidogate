#include <cgreen/mocks.h>

static FILE *mock_fopen(const char *path, const char *mode)
{
    FILE *ret;
    int _local_errno = 0;
    int *local_errno = &_local_errno;

    ret = (FILE *)mock(path, mode, local_errno);
    errno = *local_errno;
    return ret;
}

static int mock_fclose(FILE *file)
{
    return (int)mock(file);
}

static int mock_fprintf(FILE *stream, const char *format, ...)
{
    va_list args;
    int ret;
    char *res_str;

    va_start(args, format);
    ret = vasprintf(&res_str, format, args);
    assert_that(ret, is_not_equal_to(-1));
    ret = (int)mock(stream, format, res_str);
    va_end(args);
    free(res_str);
    return ret;
}

static int mock_vfprintf(FILE *stream, const char *format, va_list ap)
{
    int ret;
    char *res_str;

    ret = vasprintf(&res_str, format, ap);
    assert_that(ret, is_not_equal_to(-1));
    ret = (int)mock(stream, format, res_str);
    free(res_str);
    return ret;
}

static int mock_fflush(FILE *file)
{
    return (int)mock(file);
}

#define fclose mock_fclose
#define fopen mock_fopen
#define fprintf mock_fprintf
#define vfprintf mock_vfprintf
#define fflush mock_fflush
