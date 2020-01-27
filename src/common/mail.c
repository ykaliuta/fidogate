/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FIDO NetMail/EchoMail
 *
 *
 * Create RFC messages in mail/news dir
 *
 *****************************************************************************
 * Copyright (C) 1990-2002
 *  _____ _____
 * |     |___  |   Martin Junius             <mj@fidogate.org>
 * | | | |   | |   Radiumstr. 18
 * |_|_|_|@home|   D-51069 Koeln, Germany
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

char mail_dir[MAXPATH];
char news_dir[MAXPATH];

static char m_name[MAXPATH];
static char m_tmp[MAXPATH];
static FILE *m_file = NULL;

static char n_name[MAXPATH];
static char n_tmp[MAXPATH];
static FILE *n_file = NULL;

/*
 * Open RFC mail file
 */
int mail_open(int sel)
{
    long n;

    switch (sel) {
    case 'm':
    case 'M':
        n = sequencer(cf_p_seq_mail());
        str_printf(m_tmp, sizeof(m_tmp), "%s/%08ld.tmp", mail_dir, n);
        str_printf(m_name, sizeof(m_name), "%s/%08ld.rfc", mail_dir, n);
        m_file = fopen(m_tmp, W_MODE);
        if (!m_file) {
            fglog("$Can't create mail file %s", m_tmp);
            return ERROR;
        }
        break;

    case 'n':
    case 'N':
        n = sequencer(cf_p_seq_news());
        str_printf(n_tmp, sizeof(n_tmp), "%s/%08ld.tmp", news_dir, n);
        str_printf(n_name, sizeof(n_name), "%s/%08ld.rfc", news_dir, n);
        n_file = fopen(n_tmp, W_MODE);
        if (!n_file) {
            fglog("$Can't create mail file %s", n_tmp);
            return ERROR;
        }
        break;

    default:
        fglog("mail_open(%d): illegal value", sel);
        return ERROR;
    }

    return OK;
}

/*
 * Return mail file pointer
 */
FILE *mail_file(int sel)
{
    switch (sel) {
    case 'm':
    case 'M':
        return m_file;
        break;
    case 'n':
    case 'N':
        return n_file;
        break;
    }

    return NULL;
}

/*
 * Close mail file
 */
void mail_close(int sel)
{

    switch (sel) {
    case 'm':
    case 'M':
        fclose(m_file);
        if (rename(m_tmp, m_name) == ERROR)
            fglog("$Can't rename mail file %s to %s", m_tmp, m_name);

        m_tmp[0] = 0;
        m_name[0] = 0;
        m_file = NULL;
        break;

    case 'n':
    case 'N':
        fclose(n_file);
        if (rename(n_tmp, n_name) == ERROR)
            fglog("$Can't rename mail file %s to %s", n_tmp, n_name);

        n_tmp[0] = 0;
        n_name[0] = 0;
        n_file = NULL;
        break;
    }

    return;
}
