/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FTN NetMail/EchoMail
 *
 * $Id: structs.h,v 4.23 2004/08/22 20:19:12 n0ll Exp $
 *
 * An assortment of FIDOGATE data structure definitions
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
 * Textline, Textlist
 */
typedef struct st_textline {
    char		*line;
    struct st_textline	*next;
    struct st_textline	*prev;
} Textline;

typedef struct st_textlist {
    Textline		*first;
    Textline		*last;
    unsigned long        n;
} Textlist;



/*
 * Alias
 */
typedef struct st_alias {
    struct st_alias *next;
    Node node;
    char *username;
    char *userdom;
    char *fullname;
} Alias;



/*
 * Area
 */
typedef struct st_area {	/* Area/newsgroup entry with options */
    struct st_area *next;	    /* For linked list */
    char *area;			    /* FTN area */
    char *group;		    /* Newsgroup */
    int   zone;			    /* FTN zone AKA */
    Node  addr;			    /* FTN node address */
    char *origin;		    /* FTN origin line */
    char *distribution;		    /* Distribution */
    int   flags;		    /* Area/group flags, see AREA_xx defines */
    int   rfc_lvl;		    /* -R ^ARFC header level */
    long  maxsize;		    /* -m max. size of message for split */
    long  limitsize;		    /* -L message size limit */
    Textlist x_hdr;		    /* -X "Xtra: xyz" extra RFC headers */
    char *charset;		    /* -C def:in:out charset setting */
} Area;

#define AREA_LOCALXPOST	1	/* -l  Only local crosspostings */
#define AREA_NOXPOST	2	/* -x  No crosspostings */
#define AREA_NOGATE	8	/* -g  No messages from other gateways */
#define AREA_8BIT	16	/* -8  Use 8 bit ISO-8859-1 character set */
#define AREA_HIERARCHY	32	/* -H  Area/group names match entire hierar. */
#define AREA_NO		64	/* -!  Don't gate area/group */
#define AREA_QP		128	/* -Q  Use quoted-printable ISO-8859-1 */



/*
 * AreasBBS
 */
typedef struct st_areasbbs	/* AREAS.BBS entry */
{
    char *dir;			    /* Directory */
    char *key;			    /* Areafix access key */
    int   lvl;			    /* Areafix access level */
    int   zone;			    /* Zone AKA for area */
    Node  addr;			    /* Node AKA for area */
    char *area;			    /* Area tag */
    LON   nodes;		    /* Nodes linked to this area */
    int   flags;		    /* Area flags, see AREASBBS_xx defines */
    char *desc;			    /* Area description */
    char *state;		    /* Area state flags */
    struct st_areasbbs *next;	    /* For linked list */
}
AreasBBS;

#define AREASBBS_PASSTHRU 1	/* Passthru area (#dir) */
#define AREASBBS_READONLY 2	/* Read-only for new downlinks */



/*
 * TIMEINFO
 */
typedef struct _TIMEINFO {
    time_t	time;
    long	usec;
    long	tzone;
} TIMEINFO;



/*
 * Host
 */
typedef struct st_host {	/* hosts entry */
    struct st_host *next;	/*     for linked list */
    Node	node;		/*     FTN address */
    char       *name;		/*     Internet address */
    int		flags;		/*     flags */
} Host;

#define HOST_POINT	1	/* Addresses with pX point address */
#define HOST_DOWN	2	/* Temporary down */
#ifdef AI_1
#define HOST_ADDR	4	/* Real Fidonet address in .pkt file. 
                                   Like key '-a' in command line rfc2ftn    */
#endif

#ifdef AI_8
/*
 * Acl
 */
typedef struct st_acl {	        /* acl entry */
    struct st_acl *next;	/*     for linked list */
    char	  *email_pat;	/*     E-Mail address pattern */
    char          *ngrp_pat;	/*     Newsgroup pattern */
} Acl;
#endif


/*
 * Maus
 */
typedef struct st_maus {	/* MAUSNET domain list entry */
    struct st_maus *next;	    /* for linked list */
    char *name;			    /* MAUS system name */
    char *domain;		    /* Internet domain address */
    Node  gate;			    /* FTN gateway for MAUS system */
} Maus;



/*
 * Passwd
 */
typedef struct st_passwd	/* Password list entry */
{
    char *context;			/* "ffx" | "packet" | "af" ... */
    Node node;				/* Address */
    char *passwd;			/* Password */
    char *args;				/* More args in PASSWD file */
    struct st_passwd *next;		/* For linked list */
}
Passwd;



/*
 * Routing commands, Remap, Rewrite
 */
#define TYPE_NETMAIL	'n'
#define TYPE_ECHOMAIL	'e'

#define CMD_SEND	's'
#define CMD_ROUTE	'r'
#define CMD_CHANGE	'c'
#define CMD_HOSTROUTE	'h'
#define CMD_HUBROUTE	'u'
#define CMD_REMAP	'm'
#define CMD_REMAP_TO	'm'
#define CMD_REMAP_FROM	'f'
#define CMD_REWRITE	'w'
#define CMD_SENDMOVE	'v'
#define CMD_XROUTE	'x'
#define CMD_BOSSROUTE	'b'

#define FLAV_NONE	'-'
#define FLAV_NORMAL	'n'
#define FLAV_HOLD	'h'
#define FLAV_CRASH	'c'
#define FLAV_DIRECT	'd'

typedef struct st_routing
{
    int type;
    int cmd;
    int flav;
    int flav_new;
    LON nodes;

    struct st_routing *next;
}
Routing;

typedef struct st_remap
{
    int  type;				/* 'f' = RemapFrom, 'm' = RemapTo */
    Node from;				/* From/to pattern */
    Node to;				/* New dest. address */
    char *name;				/* Name pattern */

    struct st_remap *next;
}
Remap;

typedef struct st_rewrite
{
    Node from;				/* From pattern */
    Node to;				/* To pattern */

    struct st_rewrite *next;
}
Rewrite;



/*
 * Archiver or program
 */
#define PACK_NORMAL	'n'		/* Pack to pkt dest archive */
#define PACK_ROUTE	'r'		/* Pack to other archive */
#define PACK_FLO	'f'		/* Attach archive to other FLO */
#define PACK_DIR	'd'		/* Pack to separate directory */
#define PACK_MOVE	'm'		/* Move to separate directory */

#define PACK_ARC	'a'
#define PACK_PROG	'p'
#define PACK_PROGN	'q'

typedef struct st_arcprog
{
    int pack;
    char *name;
    char *prog;
    
    struct st_arcprog *next;
}
ArcProg;

/*
 * Packing entry
 */
typedef struct st_packing
{
    int pack;
    char *dir;
    ArcProg *arc;
    LON nodes;
    
    struct st_packing *next;
}
Packing;



/*
 * Descriptor for packet files
 */
typedef struct st_pktdesc
{
    Node from;
    Node to;
    int  grade;
    int  type;
    int  flav;
    int  move_only;
}
PktDesc;


/*
 * RFCAddr
 */
typedef struct st_rfcaddr {
    /* user@addr (real) */
    char user[MAXUSERNAME];
    char addr[MAXINETADDR];
    char real[MAXUSERNAME];
    int  flags;
} RFCAddr;

#define ADDR_MAUS	1


/*
 * MsgBody
 */
typedef struct st_body {
    /* structured FTN message body */
    char    *area;		/* AREA:xxxx echo tag */
    Textlist kludge;		/* ^A kludges at start of message */
    Textlist rfc;		/* RFC headers at start of message */
    Textlist body;		/* text body */
    char    *tear;		/* NetMail / EchoMail: --- tear line */
    char    *origin;		/* EchoMail:  * Origin: xxxx (addr) line */
    Textlist seenby;		/* EchoMail: SEEN-BY lines */
    Textlist path;		/* EchoMail: ^APATH lines */
    Textlist via;		/* NetMail: ^AVia lines */
} MsgBody;
    


/*
 * TIC file
 */
typedef struct st_tick 		/* .TIC file description, see also FSC-0028 */
{
    Node     origin;			/* Origin address */
    Node     from;			/* From (this tic) address */
    Node     to;			/* To (this tic) addresss */
    char    *area;			/* Area name */
    char    *file;			/* File name */
    char    *replaces;			/* File to replace in file area */
    Textlist desc;			/* Description (multiple lines) */
    Textlist ldesc;			/* Description (multiple lines) */
    unsigned long crc;			/* File CRC32 checksum */
    char    *created;			/* Creator */
    unsigned long size;			/* File size */
    Textlist path;			/* Path (TL because of extra stuff) */
    LON      seenby;			/* Seenby */
    char    *pw;			/* Password */
    time_t   release;			/* Release date */
    time_t   date;			/* File date */
    Textlist app;			/* Application specific */
}
Tick;



/*
 * Temporary string
 */
typedef struct st_tmps
{
    char *s;
    size_t len;
    struct st_tmps *next;
}
TmpS;



/*
 * FTN address
 */
typedef struct st_ftnaddr {
    char name[MAXUSERNAME];		/* No FTS-0001 limits here! */
    Node node;
} FTNAddr;



/*
 * Charset mapping
 */
#define MAX_CHARSET_NAME	16
#define MAX_CHARSET_IN		128
#define MAX_CHARSET_OUT		4

#define CHARSET_FILE_ALIAS	'A'	/* Id for binary file */
#define CHARSET_FILE_TABLE	'T'	/* Id for binary file */

typedef struct st_charset_alias
{
    char alias[MAX_CHARSET_NAME];	/* Alias charset name */
    char name[MAX_CHARSET_NAME];	/* Real charset name */
    struct st_charset_alias *next;
}
CharsetAlias;

typedef struct st_charset_table
{
    char in[MAX_CHARSET_NAME];		/* Input charset name */
    char out[MAX_CHARSET_NAME];		/* Output charset name */
    char map[MAX_CHARSET_IN][MAX_CHARSET_OUT];
    struct st_charset_table *next;
}
CharsetTable;
