/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FTN NetMail/EchoMail
 *
 *
 * Active group
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

#ifdef FTN_ACL
/* ******** Работа со связными списками *********************************** */

/* вставить элемент в конец списка
 */
#define LIST_INS_END(list, node)		\
  do {						\
      if( NULL == list ) {			\
          ( list ) = ( node );			\
      } else {					\
          ( list )->ll_prev->ll_next = ( node );\
          ( node )->ll_prev = ( list )->ll_prev;\
      }						\
      ( list )->ll_prev = ( node );		\
      ( node )->ll_next = NULL;			\
    } while ( 0 )

/* Проверить, последний ли элемент
 */
#define LIST_IS_END( list, node )		\
    ( ( NULL == ( list ) ) || ( NULL == ( node )->ll_next) )

/* ************************************************************************ */

typedef struct ftn_acl_s {

    LON nodes;
    char *str;
    char mtype;                 /* 'e' -> echo, 'f' -> fecho */
    char atype;                 /* 'm' -> mandatory, 'r' -> readonly */
    struct ftn_acl_s *ll_prev, *ll_next;
    time_t date;                /* date in UNIX format */

} ftn_acl_t;

static ftn_acl_t *ftn_acl = NULL;
static char atype = '\0';
static char mtype = '\0';

static ftn_acl_t *ftnacl_parse_line(char *buf)
{

    ftn_acl_t *a = NULL;
    Node node, old;
    char *t1, *t2, *t3;
    char *p, *p2;

    t1 = strtok(buf, " \t");

    if (t1 == NULL)
        return NULL;

    t2 = strtok(NULL, " \t");
    t3 = strtok(NULL, " \t");

    if (t2 == NULL) {
        if (strieq(t1, "echo")) {
            mtype = TYPE_ECHO;
            atype = '\0';
        } else if (strieq(t1, "fecho")) {
            mtype = TYPE_FECHO;
            atype = '\0';
        } else if (strieq(t1, "readonly"))
            atype = TYPE_READONLY;
        else if (strieq(t1, "mandatory"))
            atype = TYPE_MANDATORY;
        else
            fglog("acl_ftn: area mask not specified, ignoring line");
    } else {
        if (strieq(t1, "include"))
            ftnacl_do_file(t2);
        else if ((mtype == '\0') || (atype == '\0'))
            fglog("acl_ftn: acl type not specified, ignoring line");
        else {
            a = xmalloc(sizeof(ftn_acl_t));
            lon_init(&a->nodes);
            a->mtype = mtype;
            a->atype = atype;

            old.zone = cf_zone();
            old.net = old.node = old.point = -1;
            p = t1;
            while (p) {
                p2 = strchr(p, ',');
                if (p2 != NULL)
                    *p2++ = '\0';
                if (asc_to_node_diff_acl(p, &node, &old) == OK) {
                    old = node;
                    lon_add(&a->nodes, &node);
                } else {
                    fglog("acl_ftn: parse error");
                    lon_delete(&a->nodes);
                    xfree(a);
                    return NULL;
                }
                p = p2;
            }
            if (t3) {
                struct tm r;

                r.tm_mday = atoi(strtok(t3, "."));
                r.tm_mon = atoi(strtok(NULL, "."));
                r.tm_year = atoi(strtok(NULL, ".")) + 100;
                r.tm_sec = r.tm_min = r.tm_hour = 0;
                a->date = mktime(&r);
            } else {
                a->date = 0;
            }
            a->str = strsave(t2);
        }
    }
    return a;
}

int ftnacl_search(Node * node, char *area, char atype, char mtype)
{

    ftn_acl_t *acl;

    if (ftn_acl != NULL) {
        for (acl = ftn_acl; NULL != acl; acl = acl->ll_next) {
            if (atype == acl->atype && mtype == acl->mtype &&
                lon_search_acl(&acl->nodes, node) &&
                wildmatch_string(area, acl->str, TRUE) &&
                (!acl->date || acl->date > time(NULL)))
                return TRUE;
        }
    }
    return FALSE;
}

void ftnacl_do_file(char *name)
{

    FILE *fp;
    ftn_acl_t *p;

    debug(14, "Reading FTNACL file %s", name);

    fp = fopen_expand_name(name, R_MODE_T, FALSE);
    if (fp != NULL) {
        while (cf_getline(buffer, BUFFERSIZE, fp)) {
            p = ftnacl_parse_line(buffer);
            if (p != NULL)
                LIST_INS_END(ftn_acl, p);
        }
        fclose(fp);
    } else {
        fglog("$acl_ftn: can't open %s", name);
    }

    return;
}

void acl_ftn_free(void)
{
    ftn_acl_t *acl, *acl1;

    for (acl = ftn_acl; NULL != acl; acl = acl1) {

        acl1 = acl->ll_next;
        xfree(acl->str);

        if ((&acl->nodes)->size > 0) {
            lon_delete(&acl->nodes);
        }
        xfree(acl);
    }
}
#endif                          /* FTN_ACL */
