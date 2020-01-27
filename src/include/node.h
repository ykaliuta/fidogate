/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FIDO NetMail/EchoMail
 *
 *
 * Node structure (zone, net, node, point, domain)
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

#ifndef FIDOGATE_NODE_H
#define FIDOGATE_NODE_H

#define MAX_DOMAIN  32

/*
 * FTN node address
 */
typedef struct st_node {
    int zone;
    int net;
    int node;
    int point;
    char domain[MAX_DOMAIN];
    int flags;
} Node;

/* Node flags */
#define NODE_RO		1           /* r - read-only */
#define NODE_WO		2           /* w - write-only */
#define NODE_UPLINK	4           /* u - is uplink */
#define NODE_DOWNLINK	8       /* d - is downlink */
#ifdef FTN_ACL
#define NODE_ACL_INV	16      /* ACL invertion */
#endif                          /* FTN_ACL */

/*
 * Linked node entry for list of nodes
 */
typedef struct st_lnode {
    Node node;
    struct st_lnode *next, *prev;
} LNode;

/*
 * List of nodes
 */
typedef struct st_lon {
    int size;
    Node **sorted;
    LNode *first, *last;
} LON;

#endif
