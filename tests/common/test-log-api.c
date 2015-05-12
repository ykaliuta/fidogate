
const char *log_get_program(void)
{
    return the_logger.strid;
}

const char *log_get_filename(void)
{
    return the_logger.logfile_fullname;
}

#if defined(HAVE_SYSLOG) && defined(HAVE_SYSLOG_H)
int log_is_use_syslog(void)
{
    return the_logger.ops == &syslog_ops;
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

void log_suppress_debug(void)
{
    the_logger.suppress_debug = TRUE;
}

void log_set_verbose(int v)
{
    verbose = v;
}

const char *log_get_log_filename(void)
{
    return the_logger.logfile_fullname;
}

void log_set_log_filename(const char *s)
{
    char *n = the_logger.logfile_fullname;
    int size = sizeof(the_logger.logfile_fullname);

    strncpy(n, s, size);
    n[size - 1] = '\0';
}
static void log_init_globals()
{
    verbose = 0;
    the_logger.suppress_debug = FALSE;
    the_logger.logfile_fullname[0] = '\0';
    the_logger.logfile = NULL;
    strcpy(the_logger.strid, default_name);
    the_logger.debugfile = NULL;
}

