/*
*/


#include "prototypes.h"
#include "node.h"
#include "config.h"
#include <cgreen/mocks.h>
#include <stddef.h>
#include <stdio.h>

char buffer[BUFFERSIZE];
char mail_dir[MAXPATH];
char news_dir[MAXPATH];
char address_error[ADDRESS_ERROR_SIZE];

char *charset_alias_rfc(char *name)
{
    return NULL;
}

char *charset_alias_fsc(char *name)
{
    return NULL;
}

void charset_set_in_out(char *in, char *out)
{
}

char *mime_dequote(char *d, size_t n, char *s, int flags)
{
    return NULL;
}

int str_last(char *s, size_t len)
{
    return 0;
}

char *xlat_s(char *s1, char *s2)
{
    return NULL;
}

int pkt_put_msg_hdr(FILE *pkt, Message *msg, int kludge_flag)
    /* kludge_flag --- TRUE: write AREA/^AINTL,^AFMPT,^ATOPT */
{
    return 0;
}

#ifdef FIDO_STYLE_MSGID
char *s_msgid_rfc_to_fido(int *origid_flag, char *message_id,
			  int part, int split, char *area, short int dont_flush,
			  int for_reply)
#else
char *s_msgid_rfc_to_fido(int *origid_flag, char *message_id,
			  int part, int split, char *area, short int x_flags_m,
			  int for_reply)
#endif
{
    return NULL;
}

void header_ca_rfc(FILE *out, int rfc_level)
{
}

char *header_geth(char *name, int first)
{
    return NULL;
}

char *s_header_getcomplete(char *name)
{
    mock(name);
}

char *header_get(char *name)
{
    return NULL;
}

char *header_getnext(void)
{
    return NULL;
}

short header_hops(void)
{
    return 0;
}

void str_printf(char *buf, size_t len, const char *fmt, ...)
{
}

char *date(char *fmt, time_t *t)
{
    return NULL;
}

int pkt_isopen(void)
{
    return 0;
}

FILE *pkt_open(char *name, Node *node, char *flav, int bsy)
{
    mock(name, node, flav, bsy);
}

void pkt_put_line(FILE *fp, char *s)
{
}

int pkt_close(void)
{
    return 0;
}
char *s_rfcaddr_to_asc(RFCAddr *rfc, int real_flag)
    /* TRUE=with real name, FALSE=without*/
{
    return NULL;
}

RFCAddr rfcaddr_from_rfc(char *addr)
{
    RFCAddr a = { 0 };
    return a;
}

char *znf1(Node *node)
{
    return NULL;
}

char *xstrtok(char *s, const char *delim)
{
    return NULL;
}

int bink_attach(Node *node, int mode, char *name, char *flav, int bsy)
{
    return 0;
}

char *version_global(void)
{
    return NULL;
}

char *version_local(char *rev)
{
    return NULL;
}

long sequencer(char *seqname)
{
    return 0;
}

long areas_get_maxmsgsize(void)
{
    return 0;
}

void node_clear(Node *n)
{
}

int mime_debody(Textlist *body)
{
    return 0;
}

char *mime_deheader(char *d, size_t n, char *s)
{
    return NULL;
}

int acl_ngrp_lookup( char *list )
{
    return 0;
}

void acl_ngrp( RFCAddr rfc_from, int type )
{
}

void bounce_mail(char *reason, RFCAddr *addr_from, Message *msg, char *rfc_to, Textlist *body)
{
}

void tmps_freeall(void)
{
}

int addr_is_local_xpost( char *addr )
{
    return 0;
}

long areas_get_limitmsgsize(void)
{
    return 0;
}

int addr_is_local( char *addr )
{
    return 0;
}

char *addr_token(char *line)
{
    return NULL;
}

char *znfp1(Node *node)
{
    return NULL;
}

char *znfp2(Node *node)
{
    return NULL;
}

int pna_notify( char *email )
{
    return 0;
}

int is_blank_line(char *s)
{
    return 0;
}

char *strip_space(char *line)
{
    return NULL;
}

int is_space(int c)
{
    return 0;
}

int regex_match(const char *s)
{
    return 0;
}

char *str_regex_match_sub(char *buf, size_t len, int idx, const char *s)
{
    return NULL;
}

FILE *fopen_expand_name( char *name, char *mode, int err_abort )
{
    return NULL;
}

void exit_free(void)
{
}

void areas_init(void)
{
}

void pkt_outdir(char *dir1, char *dir2)
{
}

void header_read(FILE *file)
{
}

void header_delete(void)
{
}

long read_rnews_size(FILE *stream)
{
    return 0;
}

void regex_init(void)
{
}

void charset_init(void)
{
}

void strip_crlf(char *line)
{
}

int check_access(char *name, int check)
{
    return 0;
}

void acl_do_file( char *name )
{
}

int areasbbs_init(char *name)
{
    return 0;
}

void passwd_init(void)
{
}

void alias_do_file( char *name )
{
}

void hosts_init(void)
{
}

void addr_is_local_xpost_init( char *addr )
{
}

void rfcaddr_mode(int m)
{
}

void addr_restricted(int f)
{
}

void areas_limitmsgsize(long int sz)
{
}

void areas_maxmsgsize(long int sz)
{
}

char *str_expand_name(char *d, size_t n, char *s)
{
    return NULL;
}

char *cf_fqdn(void)
{
    return NULL;
}

char *cf_p_origin(void)
{
    return NULL;
}

Node cf_n_uplink(void)
{
    Node node = { 0 };
    return node;
}

Node *cf_uplink(void)
{
    return NULL;
}

int cf_zone(void)
{
    return 0;
}

Node cf_gateway(void)
{
    Node node = { 0 };
    return node;
}

Node *cf_addr(void)
{
    return NULL;
}

char *cf_p_seq_split(void)
{
    return NULL;
}

void cf_set_best(int zone, int net, int node)
{
}

void cf_set_curr(Node *node)
{
}

char *cf_get_string(char *name, int first)
{
    return NULL;
}

char *cf_p_outpkt_mail(void)
{
    return NULL;
}

char *cf_p_newsspooldir(void)
{
    return NULL;
}

char *cf_p_acl(void)
{
    return NULL;
}

char *cf_p_outpkt_news(void)
{
    return NULL;
}

char *cf_p_aliases(void)
{
    return NULL;
}

char *cf_p_outrfc_mail(void)
{
    return NULL;
}

char *cf_p_outrfc_news(void)
{
    return NULL;
}

char *cf_p_seq_msgid(void)
{
    return NULL;
}

void cf_debug(void)
{
}

void cf_i_am_a_gateway_prog(void)
{
}

void cf_set_uplink(char *addr)
{
}

void cf_set_addr(char *addr)
{
}

char *cf_s_btbasedir(char *s)
{
    return NULL;
}

void cf_check_gate(void)
{
}

void cf_read_config_file(char *name)
{
}

void cf_initialize(void)
{
}

Node cf_n_addr(void)
{
    Node node;
    return node;
}

void cf_set_zone(int zone)
{
}

AreasBBS *areasbbs_lookup(char *area)
{
    return NULL;
}

Area *areas_lookup(char *area, char *group, Node *aka)
{
    return NULL;
}

int node_eq(Node *a, Node *b)
{
    return 0;
}

Alias *alias_lookup_userdom( RFCAddr *rfc )
{
    return NULL;
}

Alias *alias_lookup( Node *node, char *username )
{
    return NULL;
}

time_t
parsedate(char *p, TIMEINFO *now)
{
    return 0;
}

int addr_is_restricted(void)
{
    return 0;
}

int asc_to_node(char *asc, Node *node, int point_flag)
{
    return 0;
}

int addr_is_domain( char *addr )
{
    return 0;
}

int cf_zones_check(int zone)
{
    return 0;
}

Host *hosts_lookup(Node *node, char *name)
{
    return NULL;
}

Node *inet_to_ftn( char *addr )
{
    return NULL;
}

/* Pure functions or with localaized side effects */

/*
char *str_copy( char *d, size_t n, char *s )
{
    return NULL;
}

char *str_copy2(char *d, size_t n, char *s1, char *s2)
{
    return NULL;
}

char *str_copy3(char *d, size_t n, char *s1, char *s2, char *s3)
{
    return NULL;
}

char *str_append( char *d, size_t n, char *s )
{
    return NULL;
}

char *str_append2(char *d, size_t n, char *s1, char *s2)
{
    return NULL;
}

char *str_upper(char *s)
{
    return NULL;
}

char *str_lower(char *s)
{
    return NULL;
}

void tl_init(Textlist *list)
{
}

void tl_append(Textlist *list, char *s)
{
}

void tl_clear(Textlist *list)
{
}

char *read_line(char *buf, int n, FILE *stream)
{
    return NULL;
}


char *s_copy(char *s)
{
    return NULL;
}

MIMEInfo *get_mime(char *ver, char *type, char *enc)
{
    return NULL;
}

 */
