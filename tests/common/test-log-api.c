
static void log_init_globals()
{
    verbose = 0;
    no_debug = FALSE;
    logname[0] = '\0';
    logfile = NULL;
    strcpy(logprog, default_name);
    debugfile = NULL;
#if defined(HAVE_SYSLOG) && defined(HAVE_SYSLOG_H)
    use_syslog   = FALSE;
    must_openlog = TRUE;
#endif
}

const char *log_get_program(void)
{
    return logprog;
}

const char *log_get_filename(void)
{
    return logname;
}

#if defined(HAVE_SYSLOG) && defined(HAVE_SYSLOG_H)
int log_is_use_syslog(void)
{
    return use_syslog;
}
#else
int log_is_use_syslog(void)
{
    return 0;
}
#endif

int log_get_verbose(void)
{
    return verbose;
}

void log_set_verbose(int v)
{
    verbose = v;
}

const char *log_get_log_filename(void)
{
    return logname;
}

void log_set_log_filename(const char *s)
{
    strncpy(logname, s, sizeof(logname));
    logname[sizeof(logname) - 1] = '\0';
}

void log_suppress_debug(void)
{
    no_debug = TRUE;
}
