/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FIDO NetMail/EchoMail
 *
 * $Id: packet.h,v 4.9 2004/08/22 20:19:12 n0ll Exp $
 *
 * Packet structure
 *
 *****************************************************************************
 * Copyright (C) 1990-2004
 *  _____ _____
 * |     |___  |   Martin Junius             <mj.at.n0ll.dot.net>
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

/*
 * Packet header
 */

#define PKT_VERSION	2

#define PKT_MAXPASSWD	8

typedef struct 
{
    Node   from;			/* Originating node address */
    Node   to;				/* Destination node address */
    time_t time;			/* Date / time */
    int    baud;
    int    version;			/* Always PKT_VERSION */
    int    product_l;			/* Product code low */
    int    product_h;			/* Product code high */
    int    rev_min;			/* Revision minor */
    int    rev_maj;			/* Revision major */
    char   passwd[PKT_MAXPASSWD+1];	/* Password */
    int    capword;			/* Capability word (== 1 fuer 2+) */
}
Packet;


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
#define MSG_PRIVATE	0x0001			/* Private */
#define MSG_CRASH	0x0002			/* Crash */
#define MSG_RECD	0x0004			/* Recd */
#define MSG_SENT	0x0008			/* Sent */
#define MSG_FILE	0x0010			/* FileAttached */
#define MSG_INTRANSIT	0x0020			/* InTransit */
#define MSG_ORPHAN	0x0040			/* Orphan */
#define MSG_KILLSENT	0x0080			/* KillSent */
#define MSG_LOCAL	0x0100			/* Local */
#define MSG_HOLD	0x0200			/* HoldForPickup */
/* Unused:              0x0400 */
#define MSG_FREQUEST	0x0800			/* FileRequest */
#define MSG_RRREQ	0x1000			/* ReturnReceiptRequest */
#define MSG_ISRR	0x2000			/* IsReturnReceiptRequest */
#define MSG_AUDIT	0x4000			/* AuditRequest */
#define MSG_FUPDATE	0x8000			/* FileUpdateReq */

#define MSG_MASK	0x7413			/* attr AND before packeting */

typedef struct 
{
    Node node_from, node_to;			/* FTN address from, to */
    Node node_orig;				/* FTN address sender */
    int attr;					/* Attribute flags */
    int cost;					/* Cost */
    time_t date;				/* Date */
    char name_to[MSG_MAXNAME];			/* To name */
    char name_from[MSG_MAXNAME];		/* From name */
    char subject[MSG_MAXSUBJ];			/* Subject */

    char *area;					/* EchoMail area or NULL */
}
Message;
