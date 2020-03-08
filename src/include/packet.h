/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FIDO NetMail/EchoMail
 *
 *
 * Packet structure
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

#ifndef FIDOGATE_PACKET_H
#define FIDOGATE_PACKET_H

#include <time.h>

/*
 * Packet header
 */

#define PKT_VERSION	2

#define PKT_MAXPASSWD	8

typedef struct {
    Node from;                  /* Originating node address */
    Node to;                    /* Destination node address */
    time_t time;                /* Date / time */
    int baud;
    int version;                /* Always PKT_VERSION */
    int product_l;              /* Product code low */
    int product_h;              /* Product code high */
    int rev_min;                /* Revision minor */
    int rev_maj;                /* Revision major */
    char passwd[PKT_MAXPASSWD + 1]; /* Password */
    int capword;                /* Capability word (== 1 fuer 2+) */
    char psd[4];                /* Produc Specific Data */
} Packet;

/*
 * Message header (see FTS-0001, packed message)
 */

#define MSG_TYPE	2
#define MSG_END		0

#define MSG_MAXDATE	20
#define MSG_MAXNAME	36
#define MSG_MAXSUBJ	72
#define MSG_MAXORIG	72

/* AttributeWord bits according to FTS-0001 */
#define MSG_PRIVATE	0x0001      /* Private */
#define MSG_CRASH	0x0002      /* Crash */
#define MSG_RECD	0x0004      /* Recd */
#define MSG_SENT	0x0008      /* Sent */
#define MSG_FILE	0x0010      /* FileAttached */
#define MSG_INTRANSIT	0x0020  /* InTransit */
#define MSG_ORPHAN	0x0040      /* Orphan */
#define MSG_KILLSENT	0x0080  /* KillSent */
#define MSG_LOCAL	0x0100      /* Local */
#define MSG_HOLD	0x0200      /* HoldForPickup */
/* Unused:              0x0400 */
#define MSG_DIRECT	0x0400      /* Local used only */
#define MSG_FREQUEST	0x0800  /* FileRequest */
#define MSG_RRREQ	0x1000      /* ReturnReceiptRequest */
#define MSG_ISRR	0x2000      /* IsReturnReceiptRequest */
#define MSG_AUDIT	0x4000      /* AuditRequest */
#define MSG_FUPDATE	0x8000      /* FileUpdateReq */

#define MSG_MASK	0x7413      /* attr AND before packeting */

/* max UTF-8 char is 4 */
#define NAMEBUFSIZE (MSG_MAXNAME * 4)
#define SUBJBUFSIZE (MSG_MAXNAME * 4)

typedef struct {
    Node node_from, node_to;    /* FTN address from, to */
    Node node_orig;             /* FTN address sender */
    int attr;                   /* Attribute flags */
    int cost;                   /* Cost */
    time_t date;                /* Date, seconds from 1970-01-01 UTC */
    char *tz;			/* Keep timezone info */
    char name_to[NAMEBUFSIZE];  /* To name */
    char name_from[NAMEBUFSIZE];    /* From name */
    char subject[SUBJBUFSIZE];  /* Subject */
    int translated;             /* name_* and subject already xlat'ed */
    char *area;                 /* EchoMail area or NULL */
} Message;

#endif
