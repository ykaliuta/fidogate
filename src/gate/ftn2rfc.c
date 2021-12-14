/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FIDO NetMail/EchoMail
 *
 * $Id: ftn2rfc.c,v 4.65 2004/08/22 20:19:12 n0ll Exp $
 *
 * Convert FTN mail packets to RFC mail and news batches
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

#include "fidogate.h"
#include "getopt.h"

#include <pwd.h>
#include <fcntl.h>
#include <sys/wait.h>



#define PROGRAM 	"ftn2rfc"
#define VERSION 	"$Revision: 4.65 $"
#define CONFIG		DEFAULT_CONFIG_GATE



/* Prototypes */
char   *get_from		(Textlist *, Textlist *);
char   *get_reply_to		(Textlist *);
char   *get_to			(Textlist *);
char   *get_cc			(Textlist *);
char   *get_bcc			(Textlist *);
Area   *news_msg		(char *);
int	unpack			(FILE *, Packet *);
int	rename_bad		(char *);
int	unpack_file		(char *);

void	short_usage		(void);
void	usage			(void);



/* Command line options */
int n_flag = FALSE;
int t_flag = FALSE;

char in_dir[MAXPATH];


/* X-FTN flags
 *    f    X-FTN-From
 *    t    X-FTN-To
 *    T    X-FTN-Tearline
 *    O    X-FTN-Origin
 *    V    X-FTN-Via
 *    D    X-FTN-Domain
 *    S    X-FTN-Seen-By
 *    P    X-FTN-Path      */
int x_ftn_f = FALSE;
int x_ftn_t = FALSE;
int x_ftn_T = FALSE;
int x_ftn_O = FALSE;
int x_ftn_V = FALSE;
int x_ftn_D = FALSE;
int x_ftn_S = FALSE;
int x_ftn_P = FALSE;


/* TrackerMail */
static char *tracker_mail_to = NULL;

/* MSGID handling */
static int no_unknown_msgid_zones    = FALSE;
static int no_messages_without_msgid = FALSE;

/* Use * Origin line for Organization header */
static int use_origin_for_organization = FALSE;

/* Don't bounce message from FTN address not listed in HOSTS */
static int ignore_hosts = FALSE;

/* Newsgroup name for unknown FTN areas (NULL = don't gate) */
static char *ftn_junk_group = NULL;

/* Address for Errors-To header (NULL = none) */
static char *errors_to = NULL;

/* Do not allow RFC addresses (chars !, &, @) in FTN to field */
static int no_address_in_to_field = FALSE;

/* Character conversion */
static int netmail_8bit = FALSE;
static int netmail_qp   = FALSE;

/* Use FTN to address (cvt to Internet address) for mail_to */
static int use_ftn_to_address = FALSE;

/* Kill split messages with ^ASPLIT kludge */
static int kill_split = FALSE;

/* Write single news article files, not one big news batch */
static int single_articles = FALSE;

/* Charset stuff */
static char *default_charset_def = NULL;
static char *default_charset_out = NULL;
static char *netmail_charset_def = NULL;
static char *netmail_charset_out = NULL;
static int netmail_charset_use_1st = FALSE;

/* String to add to news Path header */
static char *news_path_tail = "not-for-mail";



/*
 * Get header for
 *   - From/Reply-To/UUCPFROM
 *   - To
 *   - Cc
 *   - Bcc
 */
char *get_from(Textlist *rfc, Textlist *kludge)
{
    char *p, *q;
    Node dummy;
    
    p = rfcheader_get(rfc, "From");
    if(!p)
    {
	if( (p = kludge_get(kludge, "REPLYADDR", NULL)) )
	    if( (q = strchr(p,'%')) || (q = strchr(p,'@')) )
		if(znfp_parse_partial(q+1, &dummy) == OK)
		    return NULL;
    }
    if(!p)
	p = rfcheader_get(rfc, "UUCPFROM");

    return p;
}    

char *get_reply_to(Textlist *tl)
{
    return rfcheader_get(tl, "Reply-To");
}    

char *get_to(Textlist *tl)
{
    return rfcheader_get(tl, "To");
}    

char *get_cc(Textlist *tl)
{
    return rfcheader_get(tl, "Cc");
}    

char *get_bcc(Textlist *tl)
{
    return rfcheader_get(tl, "Bcc");
}    



/*
 * Test for EchoMail, return Area entry.
 */
Area *news_msg(char *line)
{
    char *p;
    Area *pa;
    static Area area;
    
    if(line)
    {
	/* Message is FIDO EchoMail */
	strip_crlf(line);
	
	for(p=line+strlen("AREA:"); *p && is_space(*p); p++);
	debug(7, "FIDO Area: %s", p);
	pa = areas_lookup(p, NULL);
	if(pa)
	{
	    debug(7, "Found: %s %s Z%d", pa->area, pa->group, pa->zone);
	    return pa;
	}

	/* Area not found */
	area.next  	  = NULL;
	area.area  	  = p;
	area.group 	  = NULL;
	area.zone         = 0;
	node_invalid(&area.addr);
	area.origin       = NULL;
	area.distribution = NULL;
	area.flags        = 0;
	area.rfc_lvl      = -1;
	area.maxsize      = -1;
	tl_init(&area.x_hdr);

	return &area;
    }

    /* Message is FIDO NetMail */
    return NULL;
}



/*
 * Check for 8bit characters in message body
 */
int check_8bit(Textlist *tl)
{
    Textline *pl;
    char *p;
    
    for(pl=tl->first; pl; pl=pl->next)
	for(p=pl->line; *p && *p!='\r'; p++)
	    if(*p & 0x80)
		return TRUE;

    return FALSE;
}

#ifdef AI_5
/*
 * Check for 8bit characters in any string
 */
int check_8bit_s(char *s, int len)
{
    char *p;

    if(!s)
	return FALSE;

    for(p=s; (int)(p-s)<len && *p!='\0'; p++)
	if(*p & 0x80)
	    return TRUE;

    return FALSE;
}
#endif


/*
 * Check for valid domain name string
 */
int check_valid_domain(char *s)
{
    if(!*s)
	return FALSE;
    while(*s)
    {
	if(!isalnum(*s) && *s!='-' && *s!='.')
	    return FALSE;
	s++;
    }
    return TRUE;
}



/*
 * Read and convert FTN mail packet
 */
int unpack(FILE *pkt_file, Packet *pkt)
{
    Message msg;			/* Message header */
    RFCAddr addr_from, addr_to;
    Textlist tl;			/* Textlist for message body */
    MsgBody body;			/* Message body of FTN message */
    int lines;				/* Lines in message body */
    Area *area;				/* Area entry for EchoMail */
    int type;
    Textline *pl;
    char *p, *s;
    char *msgbody_rfc_from;		/* RFC header From */
    char *msgbody_rfc_reply_to;		/* RFC header Reply-To */
    char *msgbody_rfc_to;		/* RFC header To */
    char *msgbody_rfc_cc;		/* RFC header Cc */
    char *msgbody_rfc_bcc;		/* RFC header Bcc */
    char mail_to[MAXINETADDR];		/* Addressee of mail */
    char x_orig_to[MAXINETADDR];
    char *from_line, *to_line;		/* From:, To: */
    char *reply_to_line;		/* Reply-To: */
    char *id_line, *ref_line;		/* Message-ID:, References: */
    char *cc_line, *bcc_line;		/* Cc:, Bcc: */
    char *thisdomain, *uplinkdomain;	/* FQDN addr this node, uplink,  */
    char *origindomain;			/*           node in Origin line */
    char *gateway;			/* ^AGATEWAY */
    Textlist theader;			/* RFC headers */
    Textlist tbody;    			/* RFC message body */
    int uucp_flag;			/* To == UUCP or GATEWAY */
    int ret;
    int rfc_lvl, rfc_lines;
    char *split_line;
    int cvt8 = 0;			/* AREA_8BIT | AREA_QP */
    char *cs_def, *cs_in, *cs_out;	/* Charset def, in(=FTN), out(=RFC) */
    char *cs_save;


    /*
     * Initialize
     */
    tl_init(&tl);
    tl_init(&theader);
    tl_init(&tbody);
    msg_body_init(&body);
    ret = OK;
    
    /*
     * Read packet
     */
    type = pkt_get_int16(pkt_file);
    if(type == ERROR)
    {
	if(feof(pkt_file))
	{
	    logit("WARNING: premature EOF reading input packet");
	    return OK;
	}
	
	logit("ERROR: reading input packet");
	return ERROR;
    }

    while(type == MSG_TYPE)
    {
	x_orig_to[0] = 0;
	tl_clear(&theader);
	tl_clear(&tbody);

	/*
	 * Read message header
	 */
	msg.node_from = pkt->from;
	msg.node_to   = pkt->to;
	if( pkt_get_msg_hdr(pkt_file, &msg) == ERROR )
	{
	    logit("ERROR: reading input packet");
	    ret = ERROR;
	    break;
	}

	/* Strip spaces at end of line */
	strip_space(msg.name_from);
	strip_space(msg.name_to);
	strip_space(msg.subject);

	/* Replace empty subject */
	if (!*msg.subject)
	    BUF_COPY(msg.subject, "(no subject)");
	
	/*
	 * Read message body
	 */
	type = pkt_get_body(pkt_file, &tl);
	if(type == ERROR)
	{
	    if(feof(pkt_file))
	    {
		logit("WARNING: premature EOF reading input packet");
	    }
	    else
	    {
		logit("ERROR: reading input packet");
		ret = ERROR;
		tl_clear(&theader);
		tl_clear(&tbody);
		tl_clear(&tl);
		msg_body_clear(&body);
		break;
	    }
	}

	/*
	 * Parse message body
	 */
	if( msg_body_parse(&tl, &body) == -2 )
	    logit("ERROR: parsing message body");
	/* Retrieve address information from kludges for NetMail */
	if(body.area == NULL)
	{
	    /* Don't use point address from packet for Netmail */
	    msg.node_from.point = 0;
	    msg.node_to  .point = 0;
	    /* Retrieve complete address from kludges */
	    kludge_pt_intl(&body, &msg, FALSE);
	    msg.node_orig = msg.node_from;
	}
	else 
	{
	    /* Retrieve address information from * Origin line */
	    if(msg_parse_origin(body.origin, &msg.node_orig) == ERROR)
		/* No * Origin line address, use header */
		node_invalid(&msg.node_orig);
	}
	debug(7, "FIDO sender (from/origin): %s", znfp1(&msg.node_orig));

	/*
	 * strip_crlf() all kludge and RFC lines
	 */
	for(pl=body.kludge.first; pl; pl=pl->next)
	    strip_crlf(pl->line);
	for(pl=body.rfc.first; pl; pl=pl->next)
	    strip_crlf(pl->line);
	
	/*
	 * X-Split header line
	 */
	split_line = NULL;
	if( (p = kludge_get(&body.kludge, "SPLIT", NULL)) )
	{
	    split_line = p;
	}
	else if( (p = kludge_get(&body.body, "SPLIT", NULL)) )
	{
	    strip_crlf(p);
	    split_line = p;
	}

	/*
	 * Remove empty first line after RFC headers and empty last line
	 */
	if( body.rfc.first && (pl = body.body.first) )
	{
	    if(pl->line[0] == '\r')
		tl_delete(&body.body, pl);
	}
	if( (pl = body.body.last) )
	{
	    if(pl->line[0] == '\r')
		tl_delete(&body.body, pl);
	}

	
	/*
	 * Check for mail or news.
	 *
	 * area == NULL  -  NetMail
	 * area != NULL  -  EchoMail
	 */
	if( (area = news_msg(body.area)) )
	{
	    cvt8 = area->flags & (AREA_8BIT | AREA_QP);
	    
	    /* Set AKA according to area's zone */
	    cf_set_zone(area->zone);

	    /* Skip, if unknown and FTNJunkGroup not set */
	    if(!area->group && !ftn_junk_group)
	    {
		logit("unknown area %s", area->area);
		tl_clear(&theader);
		tl_clear(&tbody);
		tl_clear(&tl);
		msg_body_clear(&body);
		continue;
	    }
	}
	else
	{
	    cvt8 = 0;
	    if(netmail_8bit)
		cvt8 |= AREA_8BIT;
	    if(netmail_qp)
		cvt8 |= AREA_QP;
	    
	    /* Set AKA according to sender's zone */
	    cf_set_zone(msg.node_orig.zone!=-1 
			? msg.node_orig.zone
			: msg.node_from.zone  );
	}

	/*
	 * Check for 8bit characters in message body. If none are
	 * found, don't use the quoted-printable encoding or 8bit.
	 */
#ifndef AI_5
	if(!check_8bit(&body.body))
#else
	if( !(check_8bit(&body.body)                         ||
	      check_8bit_s(body.origin, sizeof(body.origin)) ||
	      check_8bit_s(body.tear, sizeof(body.tear))     ||
	      check_8bit_s(msg.subject, sizeof(msg.subject))) )
#endif
	    cvt8 &= ~(AREA_QP | AREA_8BIT);
	
	/*
	 * Check for Content-Transfer-Encoding in RFC headers or ^ARFC kludges
	 */
	if( (p = rfcheader_get(&body.rfc, "Content-Transfer-Encoding"))  ||
	    (p = kludge_get(&body.kludge,
			    "RFC-Content-Transfer-Encoding", NULL))        )
	{
	    strip_space(p);
	    if(strieq(p, "7bit")) 
		cvt8 = 0;
	    if(strieq(p, "8bit")) 
		cvt8 = AREA_8BIT;
	    if(strieq(p, "quoted-printable")) 
		cvt8 = AREA_QP;
	}
	debug(5, "cvt8:%s%s",
	      (cvt8 & AREA_8BIT ? " 8bit" : ""),
	      (cvt8 & AREA_QP   ? " quoted-printable" : ""));
	
	/*
	 * Convert message body
	 */

	/* charset input (=FTN message) and output (=RFC message) */
	cs_save = NULL;
	cs_def  = NULL;
	cs_in   = NULL;
	cs_out  = NULL;
	
	if(area)				/* EchoMail -> News */
	{
	    if(area->charset)
	    {
		cs_save = strsave(area->charset);
		cs_def  = strtok(cs_save, ":");
		s       = strtok(NULL, ":");
		cs_out  = strtok(NULL, ":");
	    }
	}
	else					/* NetMail -> Mail */
	{
	    cs_def  = netmail_charset_def;
	    cs_out  = netmail_charset_out;
	}
	/* defaults */
	if(!cs_def)
	    cs_def = default_charset_def;
	if(!cs_def)
	    cs_def = CHARSET_STDFTN;
	if(!cs_out)
	    cs_out = default_charset_out;
	if(cvt8==0 || !cs_out)
	    cs_out = CHARSET_STD7BIT;
	
	if( (p = kludge_get(&body.kludge, "CHRS", NULL)) )
	    cs_in = charset_chrs_name(p);
	else if( (p = kludge_get(&body.kludge, "CHARSET", NULL)) )
	    cs_in = charset_chrs_name(p);
	if(!cs_in)
	    cs_in = cs_def;
	charset_set_in_out(cs_in, cs_out);
	/**FIXME: if ERROR is returned, use first matching alias for cs_in**/

	/* ^ARFC level and line break flag */
	rfc_lvl   = 0;
	rfc_lines = FALSE;
	if( (p = kludge_get(&body.kludge, "RFC", NULL)) )
	{
	    s = strtok(p, " \t");
	    if(s)
		rfc_lvl = atoi(s);
	    s = strtok(NULL, " \t");
	    if(s && !stricmp(s, "lines"))
		rfc_lines = TRUE;
	    if(s && atoi(s)==0)
		rfc_lines = TRUE;
	}
	    
	lines = 0;
	for(pl=body.body.first; pl; pl=pl->next)
	{
	    p = pl->line;
	    if(*p == '\001')			/* Kludge in message body */
	    {
		if(strnieq(p + 1, "CHRS: ", 6))
		    if( (s = charset_chrs_name(p + 6)) )
			cs_in = s;
		if(strnieq(p + 1, "CHARSET: ", 9))
		    if( (s = charset_chrs_name(p + 9)) )
			cs_in = s;
		/**FIXME: change in/out charset if needed**/
	    }
	    else				/* Normal line */
	    {
		msg_xlate_line(buffer, sizeof(buffer), p, cvt8 & AREA_QP);
		if(rfc_lines)
		{
		    tl_append(&tbody, buffer);
		    lines++;
		}
		else
		    lines += msg_format_buffer(buffer, &tbody);
	    }
	}


	/*
	 * Convert FTN from/to addresses to RFCAddr struct
	 */
	if(!cs_in)
	    cs_in = cs_def;
	charset_set_in_out(cs_in, CHARSET_STD7BIT);
	addr_from = rfcaddr_from_ftn(msg.name_from, &msg.node_orig);
	addr_to   = rfcaddr_from_ftn(msg.name_to,   &msg.node_to  );

	uucp_flag = FALSE;
	if(!stricmp(addr_to.real, "UUCP")    ||
	   !stricmp(addr_to.real, "GATEWAY")   )
	{
	    /* Don't output UUCP or GATEWAY as real name */
	    uucp_flag = TRUE;
	}

	/*
	 * RFC address headers from text body
	 */
	msgbody_rfc_from     = get_from    (&body.rfc, &body.kludge);
	msgbody_rfc_to       = get_to      (&body.rfc);
	msgbody_rfc_reply_to = get_reply_to(&body.rfc);
	msgbody_rfc_cc       = get_cc      (&body.rfc);
	msgbody_rfc_bcc      = get_bcc     (&body.rfc);

	/*
	 * If kill_split is set, skip messages with ^ASPLIT
	 */
	if(kill_split)
	{
	    /* ^A SPLIT */
	    if( (p = kludge_get(&body.kludge, "SPLIT", NULL)) )
	    {
		logit("skipping split message, origin=%s", znfp1(&msg.node_orig));
		tl_clear(&theader);
		tl_clear(&tbody);
		tl_clear(&tl);
		msg_body_clear(&body);
		continue;
	    }
	}
	
	/*
	 * If -g flag is set for area and message seems to come from
	 * another gateway, skip it.
	 */
	if(area && (area->flags & AREA_NOGATE))
	{
	    if(msgbody_rfc_from)
	    {
		logit("skipping message from gateway, area %s, origin=%s",
		    area->area, znfp1(&msg.node_orig));
		tl_clear(&theader);
		tl_clear(&tbody);
		tl_clear(&tl);
		msg_body_clear(&body);
		continue;
	    }
	
	    /* GIGO */
	    if( (p = kludge_get(&body.kludge, "PID", NULL))  &&
	       !strnicmp(p, "GIGO", 4)                         )
	    {
		logit("skipping message from gateway (GIGO), area %s, origin=%s",
		    area->area, znfp1(&msg.node_orig));
		tl_clear(&theader);
		tl_clear(&tbody);
		tl_clear(&tl);
		msg_body_clear(&body);
		continue;
	    }

	    /* Broken FidoZerb message splitting */
	    if( (p = kludge_get(&body.kludge, "X-FZ-SPLIT", NULL)) )
	    {
		logit("skipping message from gateway (X-FZ-SPLIT), area %s, origin=%s",
		    area->area, znfp1(&msg.node_orig));
		tl_clear(&theader);
		tl_clear(&tbody);
		tl_clear(&tl);
		msg_body_clear(&body);
		continue;
	    }
	}

	/*
	 * Do alias checking on both from and to names
	 */
	if(!msgbody_rfc_from)
	{
	    Alias *a;

	    debug(7, "Checking for alias: %s",
		  s_rfcaddr_to_asc(&addr_from, TRUE));
	    /**FIXME: why _strict()?**/
	    a = alias_lookup_strict(&msg.node_orig, NULL, addr_from.real);
	    if(a)
	    {
		if(a->userdom)
		{
		    debug(7, "Alias found: %s@%s %s %s", a->username, a->userdom,
			  znfp1(&a->node), a->fullname);
		    BUF_COPY(addr_from.addr, a->userdom);
		}
		else
		    debug(7, "Alias found: %s %s %s", a->username,
		          znfp1(&a->node), a->fullname);
		BUF_COPY(addr_from.user, a->username);
#ifdef ALIASES_ARE_LOCAL
		BUF_COPY(addr_from.addr, cf_fqdn());
#endif
	    }
	}
	if(!msgbody_rfc_to)
	{
	    Alias *a;
	    
	    debug(7, "Checking for alias: %s",
		  s_rfcaddr_to_asc(&addr_to, TRUE));
	    /**FIXME: why _strict()?**/
	    a = alias_lookup_strict(&msg.node_to, NULL, addr_to.real);
	    if(a)
	    {
		if(a->userdom)
		{
		    debug(7, "Alias (AI2) found: %s@%s %s \"%s\"",
			  a->username, a->userdom,
			  znfp1(&a->node), a->fullname);
		    /*BUF_COPY(addr_to.addr, a->userdom);*/
		    BUF_COPY  (mail_to, a->username);
		    BUF_APPEND(mail_to, "@");
		    BUF_APPEND(mail_to, a->userdom);
		}
		else
		{
		    debug(7, "Alias (old) found: %s %s \"%s\"",
			  a->username,
		          znfp1(&a->node), a->fullname);
		    BUF_COPY(mail_to, a->username);
		}
	    }
	    else
		BUF_COPY(mail_to, addr_to.user);
	}

	/* Special handling for -t flag (insecure): Messages with To
	 * line will be bounced */
	if(area==NULL && t_flag && msgbody_rfc_to)
	{
	    debug(1, "Insecure message with To line");
	    logit("BOUNCE: insecure mail from %s",
		s_rfcaddr_to_asc(&addr_from, TRUE));
	    bounce_mail("insecure", &addr_from, &msg, msgbody_rfc_to, &tbody);
	    tl_clear(&theader);
	    tl_clear(&tbody);
	    tl_clear(&tl);
	    continue;
	}
	    
	/* There are message trackers out there in FIDONET. Most of
	 * they can't handle addressing the gateway so we send mail
	 * from "MsgTrack..." etc. to TrackerMail.  */
	if(tracker_mail_to)
	    if(   !strnicmp(addr_from.user, "MsgTrack", 8)
	       || !strnicmp(addr_from.user, "Reflex_Netmail_Policeman", 24)
	       || !strnicmp(addr_from.user, "TrackM", 6)
	       || !strnicmp(addr_from.user, "ITrack", 6)
	       || !strnicmp(addr_from.user, "O/T-Track", 9)
	       /* || whatever ... */		                            )
	    {
		debug(1, "Mail from FIDO message tracker");
		BUF_COPY(x_orig_to, mail_to);
		BUF_COPY(mail_to  , tracker_mail_to);
		/* Reset uucp_flag to avoid bouncing of these messages */
		uucp_flag = FALSE;
	    }


	thisdomain   = s_ftn_to_inet(cf_addr(),      TRUE);
	uplinkdomain = s_ftn_to_inet(&msg.node_from, TRUE);
	origindomain = msg.node_orig.zone != -1
	    ? s_ftn_to_inet(&msg.node_orig, TRUE) : FTN_INVALID_DOMAIN;

	/* Bounce mail from nodes not registered in HOSTS, but allow
	 * mail to local users.  */
	if(addr_is_restricted() && !ignore_hosts &&
	   area==NULL && msgbody_rfc_to && !addr_is_domain(msgbody_rfc_to))
	{
	    Host *h;
	    
	    /* Lookup host */
	    if( (h = hosts_lookup(&msg.node_orig, NULL)) == NULL )
	    {
		/* Not registered in HOSTS */
		debug(1, "Not a registered node: %s",
		      znfp1(&msg.node_orig));
		logit("BOUNCE: mail from unregistered %s",
		    s_rfcaddr_to_asc(&addr_from, TRUE));
		bounce_mail("restricted", &addr_from, &msg,
			    msgbody_rfc_to, &tbody);
		tl_clear(&theader);
		tl_clear(&tbody);
		tl_clear(&tl);
		msg_body_clear(&body);
		continue;
	    }

	    /* Bounce, if host is down */
	    if(h->flags & HOST_DOWN)
	    {
		debug(1, "Registered node is down: %s",
		      znfp1(&msg.node_orig));
		logit("BOUNCE: mail from down %s",
		    s_rfcaddr_to_asc(&addr_from, TRUE));
		bounce_mail("down", &addr_from, &msg,
			    msgbody_rfc_to, &tbody);
		tl_clear(&theader);
		tl_clear(&tbody);
		tl_clear(&tl);
		msg_body_clear(&body);
		continue;
	    }
	}	    

	/*
	 * Check for address in mail_to
	 */
	if(area==NULL)
	{
	    if( strchr(mail_to, '@') || strchr(mail_to, '%') ||
	        strchr(mail_to, '!')                            )
	    {
		if(no_address_in_to_field)
		{
		    debug(1, "Message with address in mail_to: %s", mail_to);
		    logit("BOUNCE: mail from %s with address in to field: %s",
			s_rfcaddr_to_asc(&addr_from, TRUE), mail_to           );
		    bounce_mail("addrinto",
				&addr_from, &msg, msgbody_rfc_to, &tbody);
		    tl_clear(&theader);
		    tl_clear(&tbody);
		    tl_clear(&tl);
		    msg_body_clear(&body);
		    continue;
		}
		else
		    if( (p = strchr(msg.name_to, '@')) ||
			(p = strchr(msg.name_to, '%'))    )
			if(check_valid_domain(p+1))
			    /* Copy again to avoid "..." */
			    BUF_COPY(mail_to, msg.name_to);
	    }
	}
	
	/*
	 * Check for UUCP / GATEWAY, add address to mail without To line
	 */
	if(area==NULL && !msgbody_rfc_to &&
	   !strchr(mail_to, '@') && !strchr(mail_to, '%') &&
	   !strchr(mail_to, '!')			    )
	{
	    if(uucp_flag) 
	    {
		/* Addressed to `UUCP' or `GATEWAY', but no To: line */
		debug(1, "Message to `UUCP' or `GATEWAY' without To line");
		logit("BOUNCE: mail from %s without To line",
		    s_rfcaddr_to_asc(&addr_from, TRUE));
		bounce_mail("noto", &addr_from, &msg, msgbody_rfc_to, &tbody);
		tl_clear(&theader);
		tl_clear(&tbody);
		tl_clear(&tl);
		msg_body_clear(&body);
		continue;
	    }

	    BUF_APPEND(mail_to, "@");
	    if(use_ftn_to_address)
		/* Add @ftn-to-host */
		BUF_APPEND(mail_to, addr_to.addr);
	    else
		/* Add @host.domain to local address */
		BUF_APPEND(mail_to, cf_fqdn());
	}

	/* Construct string for From: header line */
	if(msgbody_rfc_from) {
	    RFCAddr rfc;
	    
	    rfc = rfcaddr_from_rfc(msgbody_rfc_from);
	    if(!rfc.real[0])
		BUF_COPY(rfc.real, addr_from.real);
	    
	    addr_from = rfc;
	}
	from_line = s_rfcaddr_to_asc(&addr_from, TRUE);

	/* Construct Reply-To line */
	reply_to_line = msgbody_rfc_reply_to;
	
	/* Construct string for To:/X-Comment-To: header line */
	if(msgbody_rfc_to) {
	    if(strchr(msgbody_rfc_to, '(') || strchr(msgbody_rfc_to, '<') ||
	       !*addr_to.real || uucp_flag                                   )
		to_line = s_printf("%s", msgbody_rfc_to);
	    else
		to_line = s_printf("%s (%s)", msgbody_rfc_to, addr_to.real);
	}
	else {
	    if(area)
	    {
		if(!strcmp(addr_to.user, "All")  ||
		   !strcmp(addr_to.user, "Alle") ||
		   !strcmp(addr_to.user, "*.*")  ||
		   !strcmp(addr_to.user, "*")      )
		    to_line = NULL;
		else
		    to_line = s_printf("(%s)", addr_to.real);
	    }
	    else
		to_line = s_printf("%s", mail_to);
	}

	/* Construct Cc/Bcc header lines */
	cc_line  = msgbody_rfc_to ? msgbody_rfc_cc  : NULL;
	bcc_line = msgbody_rfc_to ? msgbody_rfc_bcc : NULL;
	
	/* Construct Message-ID and References header lines */
	id_line  = NULL;
	ref_line = NULL;
	
	if( (p = kludge_get(&body.kludge, "ORIGID", NULL)) )
	    id_line = s_msgid_convert_origid(p, FALSE);
	else if( (p = kludge_get(&body.kludge, "Message-Id", NULL)) )
	    id_line = s_msgid_convert_origid(p, FALSE);
	else if( (p = kludge_get(&body.kludge, "RFC-Message-ID", NULL)) )
	    id_line = s_msgid_convert_origid(p, FALSE);
	if(!id_line)
	{
	    int id_zone;
	    
	    if( (p = kludge_get(&body.kludge, "MSGID", NULL)) )
	    {
		if(!strncmp(p, "<NOMSGID_", 9))
		{
		    logit("MSGID: %s, not gated", p);
		    tl_clear(&theader);
		    tl_clear(&tbody);
		    tl_clear(&tl);
		    msg_body_clear(&body);
		    continue;
		}
		id_line = s_msgid_fido_to_rfc(p, &id_zone);
		if(no_unknown_msgid_zones)
		    if(id_zone>=-1 && !cf_zones_check(id_zone))
		    {
			logit("MSGID %s: malformed or unknown zone, not gated", p);
			tl_clear(&theader);
			tl_clear(&tbody);
			tl_clear(&tl);
			msg_body_clear(&body);
			continue;
		    }
	    }
	    else
	    {
		if(no_messages_without_msgid)
		{
		    logit("MSGID: none, not gated");
		    tl_clear(&theader);
		    tl_clear(&tbody);
		    tl_clear(&tl);
		    msg_body_clear(&body);
		    continue;
		}
		id_line = s_msgid_default(&msg);
	    }
	}
	/* Can't happen, but who knows ... ;-) */
	if(!id_line)
	{
	    logit("ERROR: id_line==NULL, strange.");
	    tl_clear(&theader);
	    tl_clear(&tbody);
	    tl_clear(&tl);
	    msg_body_clear(&body);
	    continue;
	}
	
	if( (p = kludge_get(&body.kludge, "ORIGREF", NULL)) )
	    ref_line = s_msgid_convert_origid(p, FALSE);
	if(!ref_line)
	    if( (p = kludge_get(&body.kludge, "REPLY", NULL)) )
		ref_line = s_msgid_fido_to_rfc(p, NULL);

	/* ^AGATEWAY */
	gateway = kludge_get(&body.kludge, "GATEWAY", NULL);


	/*
	 * Output RFC mail/news header
	 */

	/* Different header for mail and news */
	if(area==NULL) {			/* Mail */
	    logit("MAIL: %s -> %s", from_line, to_line);
	    
	    tl_appendf(&theader,
		       "From %s %s\n", s_rfcaddr_to_asc(&addr_from, FALSE),
		       date(DATE_FROM, NULL) );
	    tl_appendf(&theader,
		"Received: by %s (FIDOGATE %s)\n",
		thisdomain, version_global()                            );
	    tl_appendf(&theader,
		"\tid AA%05d; %s\n",
		getpid(), date(NULL, NULL) );
	}
	else 					/* News */
	{
	    if(!strcmp(thisdomain, uplinkdomain))	/* this == uplink */
		tl_appendf(&theader,
			   "Path: %s!%s!%s\n",
			   thisdomain, origindomain, news_path_tail);
	    else
		tl_appendf(&theader,
			   "Path: %s!%s!%s!%s\n",
			   thisdomain, uplinkdomain, origindomain,
			   news_path_tail                         );
	}

	/* Common header */
	tl_appendf(&theader, "Date: %s\n", date(NULL, &msg.date));
	tl_appendf(&theader, "From: %s\n", from_line);
	if(reply_to_line)
	    tl_appendf(&theader, "Reply-To: %s\n", reply_to_line);
#ifndef AI_5
	msg_xlate_line(buffer, sizeof(buffer), msg.subject, 0);
#else
	msg_xlate_line(buffer, sizeof(buffer), msg.subject, cvt8);
#endif
	tl_appendf(&theader, "Subject: %s\n", buffer);
	tl_appendf(&theader, "Message-ID: %s\n", id_line);
	
	/* Different header for mail and news */
	if(area==NULL) {			/* Mail */
	    if(ref_line)
		tl_appendf(&theader, "In-Reply-To: %s\n", ref_line);
	    tl_appendf(&theader, "To: %s\n", to_line);
	    if(cc_line)
		tl_appendf(&theader, "Cc: %s\n", cc_line);
	    if(bcc_line)
		tl_appendf(&theader, "Bcc: %s\n", bcc_line);
	    if(*x_orig_to)
		tl_appendf(&theader, "X-Orig-To: %s\n", x_orig_to);
	    if(errors_to)
		tl_appendf(&theader, "Errors-To: %s\n", errors_to);
	    /* FTN ReturnReceiptRequest -> Return-Receipt-To */
	    if(msg.attr & MSG_RRREQ)
		tl_appendf(&theader, "Return-Receipt-To: %s\n",
				 s_rfcaddr_to_asc(&addr_from, FALSE)   );
	}
	else 					/* News */
	{
	    if(ref_line)
		tl_appendf(&theader, "References: %s\n", ref_line);
	    tl_appendf(&theader, "Newsgroups: %s\n",
			     area->group ? area->group : ftn_junk_group);
	    if(!area->group)
		tl_appendf(&theader, "X-FTN-Area: %s\n", area->area);
	    if(area->distribution)
		tl_appendf(&theader, "Distribution: %s\n",
				 area->distribution               );
	    if(to_line)
		tl_appendf(&theader, "X-Comment-To: %s\n", to_line);
	}
	
	/* Common header */
	p = NULL;
	if(use_origin_for_organization && body.origin)
	{
	    strip_crlf(body.origin);
#ifndef AI_5
	    msg_xlate_line(buffer, sizeof(buffer), body.origin, 0);
#else
	    msg_xlate_line(buffer, sizeof(buffer), body.origin, cvt8);
#endif
	    if((p = strrchr(buffer, '(')))
		*p = 0;
	    strip_space(buffer);
	    p = buffer + strlen(" * Origin: ");
	    while(is_blank(*p))
		p++;
	    if(!*p)
		p = NULL;
	}
	if(!p)
	    p = cf_p_organization();
	tl_appendf(&theader, "Organization: %s\n", p);

	tl_appendf(&theader, "Lines: %d\n", lines);
	if(gateway)
	    tl_appendf(&theader, "X-Gateway: FIDO %s [FIDOGATE %s], %s\n",
		       cf_fqdn(), version_global(), gateway               );
	else
	    tl_appendf(&theader, "X-Gateway: FIDO %s [FIDOGATE %s]\n",
		       cf_fqdn(), version_global()                       );

	if(area==NULL)
	{
	    if(x_ftn_f)
		tl_appendf(&theader, "X-FTN-From: %s @ %s\n",
			   addr_from.real, znfp1(&msg.node_orig));
	    if(x_ftn_t)
		tl_appendf(&theader, "X-FTN-To: %s @ %s\n",
			   addr_to.real, znfp1(&msg.node_to));
	}

	if(x_ftn_T  &&  body.tear && !strncmp(body.tear, "--- ", 4))
	{
	    strip_crlf(body.tear);
#ifndef AI_5
	    msg_xlate_line(buffer, sizeof(buffer), body.tear, 0);
#else
	    msg_xlate_line(buffer, sizeof(buffer), body.tear, cvt8);
#endif
	    tl_appendf(&theader, "X-FTN-Tearline: %s\n", buffer+4);
	}
	if(!use_origin_for_organization  &&  x_ftn_O  &&  body.origin)
	{
	    strip_crlf(body.origin);
#ifndef AI_5
	    msg_xlate_line(buffer, sizeof(buffer), body.origin, 0);
#else
	    msg_xlate_line(buffer, sizeof(buffer), body.origin, cvt8);
#endif
	    p = buffer + strlen(" * Origin: ");
	    while(is_blank(*p))
		p++;
	    tl_appendf(&theader, "X-FTN-Origin: %s\n", p);
	}
	if(x_ftn_V)
	    for(pl=body.via.first; pl; pl=pl->next)
	    {
		p = pl->line;
		strip_crlf(p);
		msg_xlate_line(buffer, sizeof(buffer), p+1, 0);
		tl_appendf(&theader, "X-FTN-Via: %s\n", buffer+4);
	    }
	if(x_ftn_D)
	    tl_appendf(&theader, "X-FTN-Domain: Z%d@%s\n",
		       cf_zone(), cf_zones_ftn_domain(cf_zone()));
	if(x_ftn_S)
	    for(pl=body.seenby.first; pl; pl=pl->next)
	    {
		p = pl->line;
		strip_crlf(p);
		tl_appendf(&theader, "X-FTN-Seen-By: %s\n", p + 9);
	    }
	if(x_ftn_P)
	    for(pl=body.path.first; pl; pl=pl->next)
	    {
		p = pl->line;
		strip_crlf(p);
		tl_appendf(&theader, "X-FTN-Path: %s\n", p + 7);
	    }

	if(split_line)
	    tl_appendf(&theader, "X-SPLIT: %s\n", split_line);

	/* MIME header */
	tl_appendf(&theader, "MIME-Version: 1.0\n");
	tl_appendf(&theader, "Content-Type: text/plain; charset=%s\n",
		   cvt8 ? cs_out : CHARSET_STD7BIT);
	tl_appendf(&theader, "Content-Transfer-Encoding: %s\n",
		   cvt8 ? ((cvt8 & AREA_QP) ? "quoted-printable" : "8bit")
		        : "7bit");

	if(cs_save)
	    xfree(cs_save);
	
	/* Add extra headers */
	if(area)
	    for(pl=area->x_hdr.first; pl; pl=pl->next)
		tl_appendf(&theader, "%s\n", pl->line);
	    
	tl_appendf(&theader, "\n");

	/* Write header and message body to output file */
	if(area) {
	    if(!mail_file('n'))
		if(mail_open('n') == ERROR)
		{
		    ret = ERROR;
		    break;
		}

	    if(!single_articles)
		/* News batch */
		fprintf(mail_file('n'), "#! rnews %ld\n",
			tl_size(&theader) + tl_size(&tbody) );
	    tl_print(&theader, mail_file('n'));
	    tl_print(&tbody,   mail_file('n'));

	    if(single_articles)
		mail_close('n');
	}
	else
	{
	    if(mail_open('m') == ERROR)
	    {
		ret = ERROR;
		break;
	    }

	    /* Mail message */
	    tl_print(&theader, mail_file('m'));
	    tl_print(&tbody,   mail_file('m'));
	    /* Close mail */
	    mail_close('m');
	}

	tl_clear(&theader);
	tl_clear(&tbody);
	tl_clear(&tl);
	msg_body_clear(&body);
	tmps_freeall();
    } /**while(type == MSG_TYPE)**/

    if(mail_file('n')) 
	mail_close('n');

    TMPS_RETURN(ret);
}



/*
 * Unpack one packet file
 */
int unpack_file(char *pkt_name)
{
    Packet pkt;
    FILE *pkt_file;

    /* Open packet and read header */
    pkt_file = fopen(pkt_name, R_MODE);
    if(!pkt_file) {
	logit("$ERROR: can't open packet %s", pkt_name);
	if(n_flag)
	    return ERROR;
	else
	{
	    rename_bad(pkt_name);
	    return OK;
	}
    }
    if(pkt_get_hdr(pkt_file, &pkt) == ERROR)
    {
	logit("ERROR: reading header from %s", pkt_name);
	if(n_flag)
	    return ERROR;
	else
	{
	    rename_bad(pkt_name);
	    return OK;
	}
    }
    
    /* * Unpack it */
    logit("packet %s (%ldb) from %s to %s", pkt_name, check_size(pkt_name),
	znfp1(&pkt.from), znfp2(&pkt.to) );
    
    if(unpack(pkt_file, &pkt) == ERROR) 
    {
	logit("ERROR: processing %s", pkt_name);
	if(n_flag)
	    return ERROR;
	else
	{
	    rename_bad(pkt_name);
	    return OK;
	}
    }
    
    fclose(pkt_file);
    
    if(!n_flag && unlink(pkt_name)==ERROR) {
	logit("$ERROR: can't unlink packet %s", pkt_name);
	rename_bad(pkt_name);
	return OK;
    }

    return OK;
}



/*
 * Usage messages
 */
void short_usage(void)
{
    fprintf(stderr, "usage: %s [-options] [packet ...]\n", PROGRAM);
    fprintf(stderr, "       %s --help  for more information\n", PROGRAM);
}


void usage(void)
{
    fprintf(stderr, "FIDOGATE %s  %s %s\n\n",
	    version_global(), PROGRAM, version_local(VERSION) );
    
    fprintf(stderr, "usage:   %s [-options] [packet ...]\n\n", PROGRAM);
    fprintf(stderr, "\
options: -1 --single-articles         write single news articles, not batch\n\
         -I --in-dir name             set input packet directory\n\
         -i --ignore-hosts            do not bounce unknown host\n\
         -l --lock-file               create lock file while processing\n\
         -n --no-remove               don't remove/rename input packet file\n\
	 -t --insecure                process insecure packets\n\
         -x --exec-program name       exec program after processing\n\
\n\
	 -v --verbose                 more verbose\n\
	 -h --help                    this help\n\
         -c --config name             read config file (\"\" = none)\n\
	 -a --addr Z:N/F.P            set FTN address\n\
	 -u --uplink-addr Z:N/F.P     set FTN uplink address\n");
    
    exit(0);
}



/***** main() ****************************************************************/

int main(int argc, char **argv)
{
    int c, ret;
    char *execprog = NULL;
    int l_flag=FALSE;
    char *I_flag=NULL;
    char *c_flag=NULL;
    char *a_flag=NULL, *u_flag=NULL;
    char *pkt_name;
    char *p;
    
    int option_index;
    static struct option long_options[] =
    {
	{ "single-articles", 0, 0, '1'},/* Write single article files */
	{ "in-dir",       1, 0, 'I'},	/* Set inbound packets directory */
	{ "ignore-hosts", 0, 0, 'i'},	/* Do not bounce unknown hosts */
	{ "lock-file",    0, 0, 'l'},	/* Create lock file while processing */
	{ "no-remove",    0, 0, 'n'},	/* Don't remove/rename packet file */
	{ "insecure",     0, 0, 't'},	/* Toss insecure packets */
	{ "exec-program", 1, 0, 'x'},	/* Exec program after processing */

	{ "verbose",      0, 0, 'v'},	/* More verbose */
	{ "help",         0, 0, 'h'},	/* Help */
	{ "config",       1, 0, 'c'},	/* Config file */
	{ "addr",         1, 0, 'a'},	/* Set FIDO address */
	{ "uplink-addr",  1, 0, 'u'},	/* Set FIDO uplink address */
	{ 0,              0, 0, 0  }
    };

    log_program(PROGRAM);
    
    /* Init configuration */
    cf_initialize();


    while ((c = getopt_long(argc, argv, "1itI:lnx:vhc:a:u:",
			    long_options, &option_index     )) != EOF)
	switch (c) {
	/***** ftn2rfc options *****/
	case '1':
	    /* Write single article files */
	    single_articles = TRUE;
	    break;
	case 'I':
	    /* Inbound packets directory */
	    I_flag = optarg;
	    break;
	case 'i':
	    /* Don't bounce unknown hosts */
	    ignore_hosts = TRUE;
	    break;
        case 'l':
            /* Lock file */
	    l_flag = TRUE;
            break;
	case 'n':
	    /* Don't remove/rename input packet file */
	    n_flag = TRUE;
	    break;
	case 't':
	    /* Insecure */
	    t_flag = TRUE;
	    break;
        case 'x':
            /* Exec program after unpack */
            execprog = optarg;
            break;

	/***** Common options *****/
	case 'v':
	    verbose++;
	    break;
	case 'h':
	    usage();
	    exit(0);
	    break;
	case 'c':
	    c_flag = optarg;
	    break;
	case 'a':
	    a_flag = optarg;
	    break;
	case 'u':
	    u_flag = optarg;
	    break;
	default:
	    short_usage();
	    exit(EX_USAGE);
	    break;
	}

    /* Read config file */
    cf_read_config_file(c_flag ? c_flag : CONFIG);

    /* Process config options */
    if(a_flag)
	cf_set_addr(a_flag);
    if(u_flag)
	cf_set_uplink(u_flag);

    cf_i_am_a_gateway_prog();
    cf_debug();
    
    /* Process local options */
    BUF_EXPAND(in_dir, I_flag ? I_flag : cf_p_pinbound());

    /* Initialize mail_dir[], news_dir[] output directories */
    BUF_EXPAND(mail_dir, DEFAULT_OUTRFC_MAIL);
    BUF_EXPAND(news_dir, DEFAULT_OUTRFC_NEWS);

    /* Process optional config statements */
    if(cf_get_string("DotNames", TRUE))
    {
	debug(8, "config: DotNames");
	rfcaddr_dot_names(TRUE);
    }
    if( (p = cf_get_string("X-FTN", TRUE)) )
    {
	debug(8, "config: X-FTN %s", p);
	while(*p)
	{
	    switch(*p)
	    {
	    case 'f':    x_ftn_f = TRUE;  break;
	    case 't':    x_ftn_t = TRUE;  break;
	    case 'T':    x_ftn_T = TRUE;  break;
	    case 'O':    x_ftn_O = TRUE;  break;
	    case 'V':    x_ftn_V = TRUE;  break;
	    case 'D':    x_ftn_D = TRUE;  break;
	    case 'S':    x_ftn_S = TRUE;  break;
	    case 'P':    x_ftn_P = TRUE;  break;
	    }
	    p++;
	}
    }
    if( (p = cf_get_string("BounceCCMail", TRUE)) )
    {
	debug(8, "config: BounceCCMail %s", p);
	bounce_set_cc(p);
    }
    if( (p = cf_get_string("TrackerMail", TRUE)) )
    {
	debug(8, "config: TrackerMail %s", p);
	tracker_mail_to = p;
    }
    if(cf_get_string("KillUnknownMSGIDZone", TRUE))
    {
	debug(8, "config: KillUnknownMSGIDZone");
	no_unknown_msgid_zones = TRUE;
    }
    if(cf_get_string("KillNoMSGID", TRUE))
    {
	debug(8, "config: KillNoMSGID");
	no_messages_without_msgid = TRUE;
    }
    if(cf_get_string("UseOriginForOrganization", TRUE))
    {
	debug(8, "config: UseOriginForOrganzation");
	use_origin_for_organization = TRUE;
    }
    if(cf_get_string("HostsRestricted", TRUE))
    {
	debug(8, "config: HostsRestricted");
	addr_restricted(TRUE);
    }
    if( (p = cf_get_string("FTNJunkGroup", TRUE)) )
    {
	debug(8, "config: FTNJunkGroup %s", p);
	ftn_junk_group = p;
    }
    if( (p = cf_get_string("ErrorsTo", TRUE)) )
    {
	debug(8, "config: ErrorsTo %s", p);
	ftn_junk_group = p;
    }
    if(cf_get_string("NoAddressInToField", TRUE))
    {
	debug(8, "config: NoAddressInToField");
	no_address_in_to_field = TRUE;
    }
    if(cf_get_string("NetMail8bit", TRUE))
    {
	debug(8, "config: NetMail8bit");
	netmail_8bit = TRUE;
    }
    if(cf_get_string("NetMailQuotedPrintable", TRUE) ||
       cf_get_string("NetMailQP", TRUE)                )
    {
	debug(8, "config: NetMailQP");
	netmail_qp = TRUE;
    }
    if(cf_get_string("UseFTNToAddress", TRUE))
    {
	debug(8, "config: UseFTNToAddress");
	use_ftn_to_address = TRUE;
    }
    if(cf_get_string("KillSplit", TRUE))
    {
	debug(8, "config: KillSplit");
	kill_split = TRUE;
    }
    if(cf_get_string("SingleArticles", TRUE))
    {
	debug(8, "config: SingleArticles");
	single_articles = TRUE;
    }
    if( (p = cf_get_string("RFCAddrMode", TRUE)) )
    {
	int m = 0;

	switch(*p) 
	{
	case '(': case 'p': case '0':
	    m = 0;				/* user@do.main (Real Name) */
	    break;
	case '<': case 'a': case '1':
	    m = 1;				/* Real Name <user@do.main> */
	    break;
	}
	rfcaddr_mode(m);
	debug(8, "config: RFCAddrMode %d", m);
    }
    if( (p = cf_get_string("DefaultCharset", TRUE)) )
    {
	debug(8, "config: DefaultCharset %s", p);
	default_charset_def = strtok(p, ":");
	strtok(NULL, ":");
	default_charset_out = strtok(NULL, ":");
    }
    if( (p = cf_get_string("NetMailCharset", TRUE)) )
    {
	debug(8, "config: NetMailCharset %s", p);
	netmail_charset_def = strtok(p, ":");
	strtok(NULL, ":");
	netmail_charset_out = strtok(NULL, ":");
    }
    if( (p = cf_get_string("NewsPathTail", TRUE)) )
    {
	/* <FIDOGATE_CONFIG>
	 * <CMD>     NewsPathTail
	 * <PARA>    STRING
	 * <DEFAULT> "not-for-mail"
	 * <DESC>    The STRING which FIDOGATE's ftn2rfc adds to the
	 *           Path header, normally "not-for-mail". If gated
	 *           messages are not generally exported to the Usenet,
	 *           setting it to "fidogate!not-for-mail" makes
	 *           the INN newsfeeds entry easier and less error-prone.
	 */
	debug(8, "config: NewsPathTail %s", p);
	news_path_tail = p;
    }
    if(cf_get_string("NetMailCharsetUse1st", TRUE))
    {
	debug(8, "config: NetMailCharsetUse1st");
	netmail_charset_use_1st = TRUE;
    }
    if(cf_get_string("DontIgnore0x8d", TRUE)   ||
       cf_get_string("DontIgnoreSoftCR", TRUE)   )
    {
	debug(8, "config: DontIgnore0x8d");
	msg_ignore_0x8d = FALSE;
    }
    
    /* Init various modules */
    areas_init();
    hosts_init();
    alias_init();
    charset_init();

    /* If called with -l lock option, try to create lock FILE */
    if(l_flag)
	if(lock_program(PROGRAM, NOWAIT) == ERROR)
	    exit(EXIT_BUSY);

    ret = EXIT_OK;

    if(optind >= argc)
    {
	/* process packet files in input directory */
	dir_sortmode(DIR_SORTMTIME);
	if(dir_open(in_dir, "*.pkt", TRUE) == ERROR)
	{
	    logit("$ERROR: can't open directory %s", in_dir);
	    if(l_flag)
		unlock_program(PROGRAM);
	    exit(EX_OSERR);
	}

	for(pkt_name=dir_get(TRUE); pkt_name; pkt_name=dir_get(FALSE))
	{
	    if(unpack_file(pkt_name) != OK)
		ret = EXIT_ERROR;
	    tmps_freeall();
	}

	dir_close();
    }
    else
    {
	/* Process packet files on command line */
	for(; optind<argc; optind++)
	{
	    if(unpack_file(argv[optind]) != OK)
		ret = EXIT_ERROR;
	    tmps_freeall();
	}
    }
    

    /* Execute given command, if option -x set.  */
    if(execprog)
    {
	int retx;

	BUF_EXPAND(buffer, execprog);
	debug(4, "Command: %s", buffer);
	retx = run_system(buffer);
	debug(4, "Exit code=%d", retx);
	if(retx != EXIT_OK)
	    ret = EXIT_ERROR;
    }
    
    if(l_flag)
	unlock_program(PROGRAM);
    
    exit(ret);
}
