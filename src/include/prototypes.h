/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FTN NetMail/EchoMail
 *
 * $Id: prototypes.h,v 4.71 2004/08/22 20:19:12 n0ll Exp $
 *
 * Prototypes for functions in libfidogate.a
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

/* define __attribute__ to nothing for non GNU */
#ifndef __GNUC__
# define __attribute__(x)	/*nothing*/
#endif



#ifdef AI_8
/* acl.c */
void	acl_init		(void);
void	acl_ngrp		(RFCAddr);
int	acl_ngrp_lookup		(char *);
int	pna_notify		(char *);
#endif
#if defined(AI_6) || defined(AI_8)
int	list_init		(char ***, char *);
int	list_match		(register int, char **, char **);
#endif

/* address.c */
extern int i_flag;

void	addr_restricted		(int);
int	addr_is_restricted	(void);
void	addr_ignore		(int);
char   *str_ftn_to_inet		(char *, size_t, Node *, int);
char   *s_ftn_to_inet		(Node *, int);
char   *s_ftn_to_inet_pfnz	(Node *);
Node   *inet_to_ftn		(char *);
int	addr_is_local		(char *);
int	addr_is_domain		(char *);
#ifdef AI_6
int 	addr_is_local_xpost	(char *);
void	addr_is_local_xpost_init(char *);
#endif
#ifdef AI_1
int	verify_host_flag	(Node *, int);
#endif

/* aliases.c */
void	alias_init		(void);
Alias  *alias_lookup		(Node *, char *, char *);
Alias  *alias_lookup_strict	(Node *, char *, char *);
Alias  *alias_lookup_userdom	(Node *, RFCAddr *, char *);

/* areas.c */
void	areas_maxmsgsize	(long);
long	areas_get_maxmsgsize	(void);
void	areas_limitmsgsize	(long);
long	areas_get_limitmsgsize	(void);
void    areas_init		(void);
Area   *areas_lookup		(char *, char *);

/* areasbbs.c */
void	areasbbs_remove		(AreasBBS *, AreasBBS *);
AreasBBS *areasbbs_new		(void);
int	areasbbs_init		(char *);
int	areasbbs_print		(FILE *);
AreasBBS *areasbbs_first	(void);
AreasBBS *areasbbs_lookup	(char *);
void	areasbbs_add		(AreasBBS *p);

/* binkley.c */
#define NOWAIT		0
#define WAIT		1

#define CHECK_FILE	0
#define CHECK_DIR	1

char   *flav_to_asc		(int);
int	asc_to_flav		(char *);
char   *bink_out_name		(Node *);
char   *bink_bsy_name		(Node *);
int	bink_bsy_test		(Node *);
int	bink_bsy_create		(Node *, int);
int	bink_bsy_delete		(Node *);
char   *bink_find_flo		(Node *, char *);
char   *bink_find_out		(Node *, char *);
int	bink_attach		(Node *, int, char *, char *, int);
int	check_access		(char *, int);
int	bink_mkdir		(Node *);
long	check_size		(char *);
int	check_old		(char *, time_t dt);

/* bounce.c */
void	bounce_set_cc		(char *);
int	print_file_subst	(FILE *, FILE*, Message*, char*, Textlist*);
int	bounce_header		(char *);
int	bounce_mail		(char *, RFCAddr *, Message *,
				 char *, Textlist *);

/* charset.c */
CharsetTable *charset_table_new	(void);
CharsetAlias *charset_alias_new	(void);
int	charset_write_bin	(char *);
int	charset_read_bin	(char *);
char   *charset_qpen		(int, int);
char   *charset_map_c		(int, int);
char   *charset_alias_fsc	(char *);
char   *charset_alias_rfc	(char *);
int	charset_set_in_out	(char *, char *);
void	charset_init		(void);
char   *charset_chrs_name	(char *);

/* config.c */
void	cf_i_am_a_gateway_prog	(void);
void	cf_check_gate		(void);
void	cf_debug		(void);
Node   *cf_main_addr		(void);
Node    cf_n_main_addr		(void);
Node   *cf_addr			(void);
Node   *cf_uplink		(void);
Node	cf_n_addr		(void);
Node	cf_n_uplink		(void);
void	cf_set_curr		(Node *);
void	cf_set_zone		(int);
int	cf_zone			();
int	cf_defzone		();
long	cf_lineno_get		(void);
long	cf_lineno_set		(long n);
char   *cf_getline		(char *, int, FILE *);
void	cf_do_line		(char *);
void	cf_read_config_file	(char *);
void	cf_initialize		(void);
void	cf_set_addr		(char *);
void	cf_set_uplink		(char *);
char   *cf_hostname		(void);
char   *cf_domainname		(void);
char   *cf_hostsdomain		(void);
char   *cf_fqdn			(void);
char   *cf_zones_inet_domain	(int);
int	cf_zones_check		(int);
char   *cf_zones_trav		(int);
char   *cf_zones_out		(int);
char   *cf_zones_ftn_domain	(int);
Node   *cf_addr_trav		(int);
int	cf_dos			(void);
char   *cf_dos_xlate		(char *);
char   *cf_unix_xlate		(char *);
Node    cf_gateway		(void);
char   *cf_get_string		(char *, int);
char   *cf_p_organization	(void);
char   *cf_p_origin		(void);

/* crc16.c */
void	crc16_init		(void);
void	crc16_update		(int);
void	crc16_update_ccitt	(int);
unsigned int crc16_value	(void);
unsigned int crc16_value_ccitt	(void);

/* crc32.c */
unsigned long compute_crc32	(unsigned char *, int);
void	crc32_init		(void);
void	crc32_compute		(unsigned char *, int);
void	crc32_update		(int);
unsigned long crc32_value	(void);
unsigned long crc32_file	(char *);

/* date.c */
char   *date			(char *, time_t *);
char   *date_buf		(char *, size_t, char *, time_t *);

/* dir.c */
#define DIR_SORTNAME	'n'
#define DIR_SORTNAMEI	'i'
#define DIR_SORTSIZE	's'
#define DIR_SORTMTIME	'm'
#define DIR_SORTNONE	'-'

int	dir_open		(char *, char *, int);
void	dir_close		(void);
void	dir_sortmode		(int);
char   *dir_get			(int);
char   *dir_search		(char *, char *);

/* file.c */
int	rename_bad		(char *);

/* flo.c */
FILE   *flo_file		(void);
int	flo_open		(Node *, int);
int	flo_openx		(Node *, int, char *, int);
char   *flo_gets		(char *, size_t);
int	flo_close		(Node *, int, int);
int	flo_mark		(void);

/* fopen.c */
FILE   *fopen_expand_name	(char *, char *, int);
FILE   *xfopen			(char *, char *);

/* ftnaddr.c */
void	ftnaddr_init		(FTNAddr *);
void	ftnaddr_invalid		(FTNAddr *);
FTNAddr	ftnaddr_parse		(char *);
char   *s_ftnaddr_print		(FTNAddr *);

/* getdate.y */
time_t	get_date		(char *, void *);

/* gettime.c / parsedate.y */
extern time_t	parsedate	(char *, TIMEINFO *);
extern int	GetTimeInfo	(TIMEINFO *);

/* history.c */
int 	hi_init			(void);
int	hi_close		(void);
int	hi_write		(time_t, char *);
int	hi_test			(char *);

/* hosts.c */
void	hosts_init		(void);
Host   *hosts_lookup		(Node *, char *);

/* kludge.c */
void    kludge_pt_intl		(MsgBody *, Message *, int);
char   *kludge_getn		(Textlist *, char *, Textline **, int);
char   *kludge_get		(Textlist *, char *, Textline **);

/* lock.c */
int	lock_fd			(int);
int	unlock_fd		(int);
int	lock_file		(FILE *);
int	unlock_file		(FILE *);
#ifdef NFS_SAFE_LOCK_FILES
int	lock_lockfile_nfs	(char *, int, char *);
int	unlock_lockfile_nfs	(char *);
#else
int	lock_lockfile_id	(char *, int, char *);
int	unlock_lockfile		(char *);
#endif
int	lock_program_id		(char *, int, char *);
int	lock_program		(char *, int);
int	unlock_program		(char *);

/* log.c */
extern int verbose;
extern int no_debug;

char   *strerror		(int);
void	logit			(const char *, ...)
    __attribute__ ((format (printf, 1, 2)));
void	debug			(int, const char *, ...)
    __attribute__ ((format (printf, 2, 3)));
void	log_file		(char *);
void	log_program		(char *);
void	logx			(char *, char *, char *, ...)
    __attribute__ ((format (printf, 3, 4)));

/* mail.c */
extern char mail_dir[MAXPATH];
extern char news_dir[MAXPATH];

int	mail_open		(int);
char   *mail_name		(int);
FILE   *mail_file		(int);
int	mail_close		(int);

/* maus.c */
void	maus_init		(void);
Maus   *maus_default		(void);
Maus   *maus_lookup		(char *);

/* message.c */
extern int msg_ignore_0x8d;

int	pkt_get_line		(FILE *, char *, int);
int	pkt_get_body		(FILE *, Textlist *);
void	msg_body_init		(MsgBody *);
void	msg_body_clear		(MsgBody *);
int	is_blank_line		(char *);
int	msg_body_parse		(Textlist *, MsgBody *);
void	msg_body_debug		(FILE *, MsgBody *, int);
int	msg_put_msgbody		(FILE *, MsgBody *, int);
int	msg_put_line		(FILE *, char *);
char   *msg_xlate_line		(char *, int, char *, int);
int	msg_format_buffer	(char *, Textlist *);
int	msg_parse_origin	(char *, Node *);

/* mime.c */
#define MIME_QP 1		/* quoted printable */
#define MIME_US 2		/* underscore `_' */

char   *mime_dequote		(char *, size_t, char *, int);
char   *mime_deheader		(char *, size_t, char *, int);

/* misc.c */
char   *str_change_ext		(char *, size_t, char *, char *);
int	str_printf		(char *, size_t, const char *, ...)
    __attribute__ ((format (printf, 3, 4)));
int	str_last		(char *, size_t);
char   *str_lastp		(char *, size_t);
char   *str_lower		(char *);
char   *str_upper		(char *);
char   *str_copy		(char *, size_t, char *);
char   *str_append		(char *, size_t, char *);
char   *str_copy2		(char *, size_t, char *, char *);
char   *str_copy3		(char *, size_t, char *, char *, char *);
char   *str_copy4		(char *, size_t, char *, char *, char *,
				 char *);
char   *str_copy5		(char *, size_t, char *, char *, char *,
				 char *, char *);

#define BUF_LAST(d)			str_last  (d,sizeof(d))
#define BUF_LASTP(d)			str_lastp (d,sizeof(d))
#define BUF_COPY(d,s)			str_copy  (d,sizeof(d),s)
#define BUF_APPEND(d,s)			str_append(d,sizeof(d),s)
#define BUF_COPY2(d,s1,s2)		str_copy2 (d,sizeof(d),s1,s2)
#define BUF_COPY3(d,s1,s2,s3)		str_copy3 (d,sizeof(d),s1,s2,s3)
#define BUF_COPY4(d,s1,s2,s3,s4)	str_copy4 (d,sizeof(d),s1,s2,s3,s4)
#define BUF_COPY5(d,s1,s2,s3,s4,s5)	str_copy5 (d,sizeof(d),s1,s2,s3,s4,s5)

char   *str_copy_range		(char *, size_t, char *, char *);
char   *strncpy0		(char *, char *, int);
char   *strncat0		(char *, char *, int);
#ifdef HAS_STRCASECMP
# define stricmp  strcasecmp
# define strnicmp strncasecmp
#endif
#if !defined(HAS_STRCASECMP) && !defined(HAS_STRICMP)
int	strnicmp		(char *, char *, int);
int	stricmp			(char *, char *);
#endif

#define streq(a,b)		(strcmp  ((a),(b)) == 0)
#define strieq(a,b)		(stricmp ((a),(b)) == 0)
#define strneq(a,b,n)		(strncmp ((a),(b),(n)) == 0)
#define strnieq(a,b,n)		(strnicmp((a),(b),(n)) == 0)

long	xtol			(char *);
void	strip_crlf		(char *);
char   *strip_space		(char *);
int	is_space		(int);
int	is_blank		(int);
int     is_digit		(int);
int     is_xdigit		(int);
int     is_odigit		(int);
char   *str_expand_name		(char *, size_t, char *);

#define BUF_EXPAND(d,s)			str_expand_name(d,sizeof(d),s)

char   *str_dosify		(char *);
#ifdef DO_DOSIFY /* MSDOS, OS2 */
# define DOSIFY_IF_NEEDED(s)	str_dosify(s);
#else            /* UNIX */
# define DOSIFY_IF_NEEDED(s)
#endif
int    run_system		(char *);

/* msgid.c */
char   *s_msgid_fido_to_rfc	(char *, int *);
char   *s_msgid_default		(Message *);
char   *s_msgid_rfc_to_fido	(int *, char *, int, int, char *);
char   *s_msgid_convert_origid	(char *, int);

/* node.c */
int	pfnz_to_node	        (char *, Node *, int);
int	asc_to_node	        (char *, Node *, int);
char   *node_to_pfnz	        (Node *, int);
int	node_eq			(Node *, Node *);
int	node_np_eq		(Node *, Node *);
void	node_clear		(Node *);
void	node_invalid		(Node *);
int	asc_to_node_diff	(char *, Node *, Node *);
char   *node_to_asc_diff	(Node *, Node *);
int	node_cmp		(Node *, Node *);

void	lon_init		(LON *);
void	lon_delete		(LON *);
void	lon_add			(LON *, Node *);
int	lon_remove		(LON *, Node *);
int	lon_search		(LON *, Node *);
int	lon_add_string		(LON *, char *);
int	lon_print		(LON *, FILE *, int);
void	lon_sort		(LON *, int);
int	lon_print_sorted	(LON *, FILE *, int);
void	lon_debug		(int, char *, LON *, int);
void	lon_join		(LON *, LON *);

/* outpkt.c */
char   *outpkt_outputname	(char *, char *, int, int, int, long, char *);
long    outpkt_sequencer	(void);
void	outpkt_set_maxopen	(int);
FILE   *outpkt_open		(Node *, Node *, int, int, int, int);
int	outpkt_close		(void);
int	outpkt_netmail		(Message *, Textlist *, char *);

/* packet.c */
void	pkt_outdir		(char *, char *);
char   *pkt_get_outdir		(void);
void	pkt_baddir		(char *, char *);
char   *pkt_get_baddir		(void);
FILE   *pkt_open		(char *, Node *, char *, int);
int	pkt_close		();
char   *pkt_name		(void);
char   *pkt_tmpname		(void);
int	pkt_isopen		(void);

int	pkt_get_string		(FILE *, char *, int);
time_t	pkt_get_date		(FILE *);
int	pkt_get_msg_hdr		(FILE *, Message *);
void	pkt_debug_msg_hdr	(FILE *, Message *, char *);
int	pkt_put_string		(FILE *, char *);
int	pkt_put_line		(FILE *, char *);
int	pkt_put_int16		(FILE *, int);
int	pkt_put_date		(FILE *, time_t);
void	pkt_set_forcefmpt0	(int);
void	pkt_set_forceintl	(int);
int	pkt_put_msg_hdr		(FILE *, Message *, int);
long	pkt_get_int16		(FILE *);
int	pkt_get_nbytes		(FILE *, char *, int);
int 	pkt_get_hdr		(FILE *, Packet *);
void	pkt_debug_hdr		(FILE *, Packet *, char *);
int	pkt_put_string_padded	(FILE *, char *, int);
int	pkt_put_hdr		(FILE *, Packet *);

/* parsenode.c */
int	znfp_get_number		(char **);
int	znfp_parse_partial	(char *, Node *);
int	znfp_parse_diff		(char *, Node *, Node *);
char   *znfp_put_number		(int, int);
char   *s_znfp_print		(Node *, int, int);
char   *str_znfp_print		(char *, size_t, Node *, int, int);
char   *s_znfp			(Node *);
char   *znfp1			(Node *);
char   *znfp2			(Node *);
char   *znfp3			(Node *);
char   *znf1			(Node *);
char   *znf2			(Node *);

/* passwd.c */
void	passwd_init		(void);
Passwd *passwd_lookup		(char *, Node *);

/* read.c */
char *read_line			(char *, int, FILE *);
long  read_rnews_size		(FILE *);

/* rematch.c */
#ifdef HAS_POSIX_REGEX
int	regex_match		(const char *);
char   *str_regex_match_sub	(char *, size_t, int, const char *);
void	regex_init		(void);
#endif

/* rfcaddr.c */
void	rfcaddr_dot_names	(int);
void	rfcaddr_mode		(int);
RFCAddr	rfcaddr_from_ftn	(char *, Node *);
RFCAddr	rfcaddr_from_rfc	(char *);
char   *s_rfcaddr_to_asc	(RFCAddr *, int);

/* rfcheader.c */
int	header_ca_rfc		(FILE *, int);
void	header_delete		(void);
void	header_read		(FILE *);
int	header_hops		(void);
char   *rfcheader_get		(Textlist *, char *);
char   *header_get		(char *);
char   *rfcheader_geth		(Textlist *, char *, int);
char   *header_geth		(char *, int);
char   *header_getnext		(void);
char   *s_header_getcomplete	(char *);
char   *addr_token		(char *);

/* routing.c */
extern Routing *routing_first;
extern Routing *routing_last;
extern Remap   *remap_first;
extern Remap   *remap_last;
extern Rewrite *rewrite_first;
extern Rewrite *rewrite_last;

int	parse_cmd		(char *);
int	parse_flav		(char *);
void	routing_init		(char *);
int	node_match		(Node *, Node *);
PktDesc*parse_pkt_name		(char *, Node *, Node *);

/* sequencer.c */
long	sequencer		(char *);
long	sequencer_nx		(char *, int);

/* strtok_r.c */
#define DELIM_WS	" \t\r\n"
#define DELIM_EOL	"\r\n"
#define NONE		0
#define QUOTE		'\"'
#define DQUOTE		'\"'
#define SQUOTE		'\''

#undef strtok
#undef strtok_r

char   *strtok			(char *, const char *);
char   *xstrtok                 (char *, const char *);
char   *strtok_r		(char *, const char *, char **);
char   *strtok_r_ext		(char *, const char *, char **, int);

#define LOCAL_STRTOK		char *local_strtok_lasts;
#define STRTOK(s,d)		strtok_r_ext(s,d,&local_strtok_lasts,NONE)
#define STRTOK_Q(s,d)		strtok_r_ext(s,d,&local_strtok_lasts,DQUOTE)

/* textlist.c */
int	tl_fput			(FILE *, Textlist *);
void	tl_add			(Textlist *, Textline *);
void	tl_remove		(Textlist *, Textline *);
void	tl_delete		(Textlist *, Textline *);
void	tl_init			(Textlist *);
void	tl_append		(Textlist *, char *);
void	tl_appendf		(Textlist *, char *, ...)
    __attribute__ ((format (printf, 2, 3)));
int	tl_print		(Textlist *, FILE *);
int	tl_print_x		(Textlist *, FILE *, char *);
void	tl_clear		(Textlist *);
long	tl_size			(Textlist *);
void	tl_addtl		(Textlist *, Textlist *);

/* tick.c */
void	tick_init		(Tick *);
void	tick_delete		(Tick *);
int	tick_put		(Tick *, char *);
int	tick_get		(Tick *, char *);
void	tick_debug		(Tick *, int);
int	tick_send		(Tick *, Node *, char *);
void	tick_add_path		(Tick *);

/* tmps.c */
void    fatal			(char *, int);
TmpS   *tmps_alloc		(size_t);
TmpS   *tmps_realloc		(TmpS *, size_t);
TmpS   *tmps_find		(char *);
void    tmps_free		(TmpS *);
void    tmps_freeall		(void);
TmpS   *tmps_printf		(const char *, ...)
    __attribute__ ((format (printf, 1, 2)));
TmpS   *tmps_copy		(char *);
TmpS   *tmps_stripsize		(TmpS *);
char   *s_alloc			(size_t);
char   *s_realloc		(char *, size_t);
void    s_free			(char *s);
void    s_freeall		(void);
char   *s_printf		(const char *, ...)
    __attribute__ ((format (printf, 1, 2)));
char   *s_copy			(char *);
char   *s_stripsize		(char *);

#define TMPS_RETURN(x)		do { tmps_freeall(); return x; } while(0)

/* version.c */
char   *version_global		(void);
char   *version_local		(char *);
int	version_major		(void);
int	version_minor		(void);

/* wildmat */
int	wildmat			(char *, char *);
int	wildmatch		(char *, char *, int);

/* xalloc.c */
#define BUFFERSIZE		(32*1024)	/* Global buffer */
extern char buffer[BUFFERSIZE];

void   *xmalloc			(int);
void   *xrealloc		(void *, int);
void	xfree			(void *);
char   *strsave			(char *);
char   *strsave2		(char *, char *);
char   *strsaveline		(char *);
