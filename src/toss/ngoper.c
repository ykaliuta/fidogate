/*
 */

#include "fidogate.h"

#include "getopt.h"

#define PROGRAM		"ngoper"    /* NewsGroup OPERations */
#define CONFIG		DEFAULT_CONFIG_MAIN

#define GROUP_MAX_LENGTH	128

void short_usage();
void usage();

int main(int argc, char **argv)
{
    int c;
    int option_index;
    int remove;
    char *cmd_ng, *cmd_rn = NULL, *cmd_rm;
    char *c_flag = NULL;
    char *p;
    char buf[BUFSIZ];
    FILE *fp;

    static struct option long_options[] = {
        {"verbose", 0, 0, 'v'}, /* More verbose */
        {"help", 0, 0, 'h'},    /* Help */
        {"config", 1, 0, 'c'},  /* Config */
        {0, 0, 0, 0}
    };

    /* Init configuration
     */
    cf_initialize();

    while (EOF != (c = getopt_long(argc, argv, "vhc:",
                                   long_options, &option_index)))
        switch (c) {
            /* Common options
             */
        case 'v':
            ++verbose;
            break;
        case 'h':
            usage();
            return 0;
            break;
        case 'c':
            c_flag = optarg;
            break;
        default:
            short_usage();
            return EX_USAGE;
            break;
        }

    /* Read config file
     */
    cf_read_config_file(c_flag ? c_flag : CONFIG);

    if (2 != (argc - optind)) {
        short_usage();
        exit_free();
        return EX_USAGE;
    }

    if (0 == stricmp(argv[optind], "create"))
        remove = FALSE;
    else if (0 == stricmp(argv[optind], "remove"))
        remove = TRUE;
    else {
        short_usage();
        exit_free();
        return EX_USAGE;
    }

    if (GROUP_MAX_LENGTH < strlen(argv[optind + 1])) {
        printf("Newsgroup name too long (max %u characters)\n",
               GROUP_MAX_LENGTH);
        exit_free();
        return 1;
    }

    /* AutoCreateNewgroupCmd */
    if (NULL != (p = cf_get_string("AutoCreateNewgroupCmd", TRUE))) {
        cmd_ng = p;
    } else {
        printf("config: parameter `AutoCreateNewgroupCmd' not defined!\n");
        exit_free();
        return 1;
    }

    /* AutoCreateRenumberCmd */
    if (NULL != (p = cf_get_string("AutoCreateRenumberCmd", TRUE))) {
        cmd_rn = p;
    }

    /* AutoCreateRmgroupCmd */
    if (NULL != (p = cf_get_string("AutoCreateRmgroupCmd", TRUE))) {
        cmd_rm = p;
    } else {
        printf("config: parameter `AutoCreateRmgroupCmd' not defined!\n");
        exit_free();
        return 1;
    }

    fp = freopen("/dev/null", "w", stdout);
    if (fp == NULL)
        fglog("$ERROR: could not redirect stdout");
    fp = freopen("/dev/null", "w", stderr);
    if (fp == NULL)
        fglog("$ERROR: could not redirect stderr");
    fp = freopen("/dev/null", "r", stdin);
    if (fp == NULL)
        fglog("$ERROR: could not redirect stdin");

    if (!remove) {
        str_printf(buf, sizeof(buf), cmd_ng, argv[optind + 1]);
        if (0 != run_system(buf))
            return 1;

        if (cmd_rn) {
            str_printf(buf, sizeof(buf), cmd_rn, argv[optind + 1]);
            if (0 != run_system(buf))
                return 1;
        }
    } else {
        str_printf(buf, sizeof(buf), cmd_rm, argv[optind + 1]);
        if (0 != run_system(buf))
            return 1;
    }

    return 0;
}

/* Usage messages
 */
void short_usage()
{
    fprintf(stderr, "usage: %s create|remove NEWSGROUP\n", PROGRAM);
    fprintf(stderr, "       %s --help  for more information\n", PROGRAM);
}

void usage()
{
    fprintf(stderr, "FIDOGATE %s  %s %s\n\n",
            version_global(), PROGRAM, version_local(VERSION));

    fprintf(stderr, "usage:   %s create|remove NEWSGROUP\n\n", PROGRAM);

    exit_free();
}
