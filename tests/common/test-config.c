#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <cgreen/cgreen.h>
#include <cgreen/mocks.h>

#include <config.c>

/* local mocks/stubs */

int verbose;
int no_debug;

void debug(int lvl, const char *fmt, ...)
{
    return;
}
void fglog(const char *fmt, ...)
{
    return;
}

char *znfp1(Node *node)
{
    return NULL;
}

char *znfp2(Node *node)
{
    return NULL;
}

char *znfp3(Node *node)
{
    return NULL;
}

char buffer[BUFFERSIZE];
void *xmalloc(int size)
{
    return NULL;
}
void xfree(void *p)
{
    return;
}
void *xrealloc(void *ptr, int size)
{
    return NULL;
}

char *str_copy2(char *d, size_t n, char *s1, char *s2)
{
    return NULL;
}

Node *inet_to_ftn( char *addr)
{
    return NULL;
}

int asc_to_node(char *asc, Node *node, int point_flag)
{
    return 0;
}
FILE *xfopen(char *name, char *mode)
{
    return NULL;
}

char *strsave(char *s)
{
    return NULL;
}

char *xstrtok(char *s, const char *delim)
{
    return NULL;
}

char *str_copy(char *d, size_t n, char *s)
{
    return NULL;
}

void strip_crlf(char *line)
{
    return;
}

int is_space(int c)
{
    return 0;
}

int is_blank(int c)
{
    return 0;
}


#include "mock-config-cf_s.c"

#include "test-config-api.c"
/* ----------- end of mocks -------------- */

Ensure(config_inits)
{
    int env_owr = 1;

    setenv("FIDOGATE", "/tmp/mock-fidogate-dir", env_owr);
    expect(cf_s_configdir);

    cf_initialize();
}

Ensure(cf_p_origin_default)
{
    char *exp = "FIDOGATE";
    char *ret;

    ret = cf_p_origin();

    assert_that(ret, is_equal_to_string(exp));
}

Ensure(cf_p_origin_returns_config)
{
    char *exp = "MY FIDOGATE";
    char *ret;
    void *to_free;

    to_free = cf_add_string("Origin", exp);

    ret = cf_p_origin();

    assert_that(ret, is_equal_to_string(exp));

    free(to_free);
}

TestSuite *create_config_suite(void)
{
    TestSuite *suite = create_named_test_suite(
	"Config suite");

    add_test(suite, config_inits);
    add_test(suite, cf_p_origin_default);
    add_test(suite, cf_p_origin_returns_config);

    return suite;
}

int main(int argc, char **argv) {
    TestSuite *suite = create_config_suite();
    return run_test_suite(suite, create_text_reporter());
}
