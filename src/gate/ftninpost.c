/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FTN NetMail/EchoMail
 *
 * Processing inbound packets
 *
 *****************************************************************************
 * Copyright (C) 2001
 *
 *    Dmitry Fedotov            FIDO:      2:5030/1229
 *				Internet:  dyff@users.sourceforge.net
 *
 * This file is part of FIDOGATE.
 *
 * FIDOGATE is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * FIDOGATE is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FIDOGATE; see the file COPYING.  If not, write to the Free
 * Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *****************************************************************************/

#include "fidogate.h"
#include "getopt.h"

#define PROGRAM 	"ftninpost"
#define CONFIG		DEFAULT_CONFIG_MAIN

typedef struct split_t {
    char *id;
    int part;
    int parts;
    char *fname;
    struct split_t *next;
} split;

/*
 * Prototypes
 */
void short_usage(void);
void usage(void);

int do_dir(char *, int);

char ftnin_sendmail[MAXPATH] = { 0 };

char *ftnin_rnews;
split *split_first = NULL;
split *split_last;

/*
 * Usage messages
 */
void short_usage(void)
{
    fprintf(stderr, "usage: %s [-options]\n", PROGRAM);
    fprintf(stderr, "       %s --help  for more information\n", PROGRAM);
}

void usage(void)
{
    fprintf(stderr, "FIDOGATE %s  %s %s\n\n",
            version_global(), PROGRAM, version_local(VERSION));

    fprintf(stderr, "usage:   %s [-options]\n\n", PROGRAM);
    fprintf(stderr, "\
options: -v --verbose                 more verbose\n\
	 -h --help                    this help\n\
         -c --config name             read config file (\"\" = none)\n");
}

/*
int unsplit_mail_count(char *fname)
{
    FILE *fp;
    char line[100];
    int found = FALSE;
    char *s;
    int idlen;
    split *spl;

    debug(1, "Processing %s", fname);

    fp = fopen(fname, "r");
    if(!fp) {
	fglog("can't open %s", fname);
        return ERROR;
    }
    while((fgets(line, sizeof(line), fp)))
	if(!strncasecmp(line, "X-SPLIT:", 8)) {
	    found = TRUE;
	    break;
	}
    if(!found)
        return OK;

    for(idlen=0, s=&line[9]; s; s++, idlen++) {
	if(isdigit(*s) && isdigit(*(s+1)) && *(s+2) == '/' &&
	   isdigit(*(s+3)) && isdigit(*(s+4))) {

	      spl = (split *)xmalloc(sizeof(split));
	      if(!split_first)
	         split_first = spl;
	      else
	         split_last->next = spl;
	      spl->next = NULL;

	      s[idlen-1]='\0';
	      spl->id = strsave(line);
	      s[2] = '\0';
	      spl->parts = atoi(s+3);
	      s[5] = '\0';
	      spl->part = atoi(s);
	      spl->fname = strsave(fname);
	      break;
	   }
    }
    debug(8,"ID: %s %d/%d", spl->id, spl->part, spl->parts);

    fclose(fp);

    return OK;
}

int unsplit_mail()
{
    split *spl;

    debug(1, "Processing split mails\n");

    for(spl = split_first; spl; spl=spl->next)
    {
    split *splt;
    split *sfirst;
	int count = 0;
	for(splt = split_first; splt; splt=splt->next)
	{
	    if(!strcmp(spl->id, splt->id))
	    {
	       if(spl->part == 1)
	          sfirst = spl;
	       count++;
	    }
 	}
// fixme - processing from first split //
	if(spl->parts == count) {
	    FILE *fp, *usp;
            char path[MAXPATH];
            int linesline=0;
            int lines;
            int flag=0;

	    debug(3, "Message %s complete, recombining", spl->id);

	    fp = fopen(sfirst->fname, "r");
	    if(!fp) {
	       fglog("can't open %s", sfirst->fname);
	       continue;
	    }
            BUF_COPY2(path, sfirst->fname, ".unsplit");
	    usp = fopen(path, "w");
            if(!usp) {
		fglog("can't open %s for writing", path);
                continue;
            }
            while(fgets(buffer, BUFSIZ, fp)) {
           	if(*buffer == '\n')
                    flag++;
                if(flag == 0) {
           	    if(strnicmp(buffer, "Lines: ", 7)) {
                        lines = atoi(&buffer[7]);
               	        flag++;
                    }
	            else
                        linesline++;
                } else if(flag == 1) {
                    if(strnicmp(buffer, "X-SPLIT:", 8))
                        flag++;
                }
            }
	} else {
	    fglog("Message %s incomplete", spl->id);
	}
    }

    return 0;
}
*/
int do_dir(char *cdir, int mode)
{
    char *rfc_file;
    char buf[MAXPATH + MAXINETADDR];

    if (dir_open(cdir, "*.rfc", TRUE) == ERROR) {
        debug(7, "can't open directory %s", cdir);
        return ERROR;
    }
    for (rfc_file = dir_get(TRUE); rfc_file; rfc_file = dir_get(FALSE)) {
        FILE *fp;
        char *p = NULL;
        int ret, pr = FALSE;

        debug(8, "Processing %s", rfc_file);

        switch (mode) {
        case 0:
/*		unsplit_mail_count(rfc_file);
		unsplit_mail();*/
            break;
        case 1:
            if ((p = strstr(ftnin_sendmail, " -f"))) {
                for (p += 3; p && *p == ' '; p++) ; /* skip spaces */

                if (*p == '%' && p[1] == 's') {
                    fp = fopen(rfc_file, "r");
                    if (!fp)
                        break;
                    p = fgets(buffer, sizeof(buffer), fp);
                    fclose(fp);
                    if (!p) {
                        fglog("ERROR: could not read %s", rfc_file);
                        break;
                    }
                    if (strncmp(p, "From ", 5)) {
                        debug(9, "WARNING: \"From\" string not found");
                        break;
                    } else {
                        p = xstrtok(p + 5, " \t");
                        str_printf(buf, sizeof(buf), ftnin_sendmail, p);
                        p = buf;
                    }
                } else {
                    p = ftnin_sendmail;
                }
            } else {
                p = ftnin_sendmail;
            }

            pr = TRUE;
            break;

        case 2:
            p = ftnin_rnews;
            pr = TRUE;
            break;

        default:
            debug(1, "unknown mode %d", mode);
            break;
        }
        if (pr == TRUE) {
            ret = OK;
            debug(8, "exec: %s", p);
            fp = freopen(rfc_file, R_MODE, stdin);
            if (fp != NULL) {
                ret = run_system(p);
                fclose(stdin);
            } else {
                fglog("$ERROR: could not redirect stdin");
            }
            if ((fp == NULL) || (ret != OK)) {
                char bad[MAXPATH];
                fglog("$WARNING: %s returned non-zero status", p);
                str_change_ext(bad, sizeof(bad), rfc_file, ".bad");
                fglog("ERROR: bad rfcbatch renamed to %s", bad);
                rename(rfc_file, bad);
            }
            unlink(rfc_file);
        }
    }
    dir_close();

    return OK;
}

/***** main() ****************************************************************/

int main(int argc, char **argv)
{
    int c;
    char *c_flag = NULL;
    char buf[MAXPATH] = { 0 };
    char *p;

    int option_index;
    static struct option long_options[] = {
        {"verbose", 0, 0, 'v'}, /* More verbose */
        {"help", 0, 0, 'h'},    /* Help */
        {"config", 1, 0, 'c'},  /* Config file */
        {0, 0, 0, 0}
    };

    log_program(PROGRAM);

    /* Init configuration */
    cf_initialize();

    while ((c = getopt_long(argc, argv, "vhc:",
                            long_options, &option_index)) != EOF)
        switch (c) {
    /***** Common options *****/
        case 'v':
            verbose++;
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

    /*
     * Read config file
     */
    cf_read_config_file(c_flag ? c_flag : CONFIG);

    cf_debug();

    passwd_init();

    ftnin_rnews = NULL;

    if ((p = cf_get_string("FTNInSendmail", TRUE))) {
        BUF_COPY(ftnin_sendmail, p);
    } else
        debug(9, "WARNING: FTNInSendmail not defined in %s",
              c_flag ? c_flag : CONFIG);

    if ((p = cf_get_string("FTNInRnews", TRUE))) {
        ftnin_rnews = p;
    } else
        debug(9, "WARNING: FTNInRnews not defined in %s",
              c_flag ? c_flag : CONFIG);

    if (!cf_get_string("SingleArticles", TRUE)) {
        if ((p = cf_get_string("FTNInRecombine", TRUE))) {
            BUF_EXPAND(buf, p);
        } else
            debug(9, "WARNING: FTNInRecombine not defined in %s",
                  c_flag ? c_flag : CONFIG);
    }

    if (*buf)
        run_system(buf);

    BUF_EXPAND(buf, cf_p_outrfc_mail());
//    do_dir(buf, 0);
    do_dir(buf, 1);
    BUF_EXPAND(buf, cf_p_outrfc_news());
    do_dir(buf, 2);
/*
    unsplit_mail("test");
*/
    exit_free();
    return EX_OK;
}
