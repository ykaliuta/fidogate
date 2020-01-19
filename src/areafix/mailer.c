#include "fidogate.h"

#define DEFAULT_MAILER	"/usr/lib/sendmail -t"

char *areafix_name(void);

/*
 * Open mailer for sending reply
 */
FILE *mailer_open(char *to, int forward, char *robotname, char *password)
{
    FILE *fp;
    char *cc;
    char *mailer;
    char *fix_name;

    mailer = cf_get_string("AreafixMailer", TRUE);
    if (!mailer)
        mailer = DEFAULT_MAILER;

    fp = popen(mailer, W_MODE);
    if (!fp) {
        fglog("ERROR: can't open pipe to %s", mailer);
        return NULL;
    }

    if ((fix_name = cf_get_string("AreaFixName", TRUE)) && forward == TRUE) {
        fprintf(fp, "From: %s@%s (%s)\n", fix_name, cf_fqdn(), fix_name);
    } else
        fprintf(fp, "From: %s@%s (%s)\n",
                areafix_name(), cf_fqdn(), areafix_name());
    if (forward)
        fprintf(fp, "To: %s@%s\n", robotname, to);
    else
        fprintf(fp, "To: %s\n", to);
    if ((cc = cf_get_string("AreafixCC", TRUE)) && (!forward))
        fprintf(fp, "Cc: %s\n", cc);
    if (forward)
        fprintf(fp, "Subject: %s\n", password);
    else
        fprintf(fp, "Subject: Your %s request\n", areafix_name());

    fprintf(fp, "\n");

    return fp;
}

/*
 * Close mailer
 */
int mailer_close(FILE * fp)
{
    return pclose(fp);
}
