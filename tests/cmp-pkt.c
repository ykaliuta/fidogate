/* This file is a part of fidogate
 * (c) 2019 Yauheni Kaliuta <y.kaliuta@gmail.com>
 */

#include "fidogate.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

struct message {
    Message hdr;
    MsgBody body;
};

struct packet {
    Packet hdr;
    size_t n_msgs;
    struct message *msgs;
};

static char *skip_kludges[] = {
    "SPLIT:", /* contains date */
    NULL,
};

static struct packet *packet_new(void)
{
    struct packet *pkt;

    pkt = malloc(sizeof(*pkt));
    memset(pkt, 0, sizeof(*pkt));
    if (pkt == NULL)
	abort();

    return pkt;
}

static struct message *packet_msg_new(struct packet *pkt)
{
    pkt->n_msgs++;

    pkt->msgs = realloc(pkt->msgs, sizeof(*pkt->msgs) * pkt->n_msgs);
    if (pkt->msgs == NULL)
	abort();

    memset(pkt->msgs + (pkt->n_msgs - 1), 0, sizeof(*pkt->msgs));
    return pkt->msgs + (pkt->n_msgs - 1);

}

static void packet_free(struct packet *pkt)
{
    int i;

    if (pkt == NULL)
	return;

    for (i = 0; i < pkt->n_msgs; i++)
	msg_body_clear(&pkt->msgs[i].body);

    free(pkt->msgs);
    free(pkt);
}

#define OUT(field) do {					\
	fprintf(stderr, "Field " #field " differ\n");	\
	ret = 1;					\
    } while (0);

#define VAL_CMP(val_name) do {			\
	if (h1->val_name != h2->val_name)	\
	OUT(val_name);				\
    } while(0);

#define NODE_CMP(val_name) do {				\
	if (!node_eq(&h1->val_name, &h2->val_name))	\
	    OUT(val_name);				\
    } while(0);

#define STRING_CMP(val_name) do {				\
    if ((h1->val_name == NULL) != (h2->val_name == NULL)) {	\
	ret = 1;						\
	break;							\
    }								\
    if (h1->val_name == NULL)					\
	break;							\
    if (strcmp(h1->val_name, h2->val_name) != 0)		\
	OUT(val_name);						\
    } while(0);

#define TL_CMP(val_name) do {						\
	if (tl_cmp(# val_name,						\
		   &h1->val_name, &h2->val_name, NULL) != 0)		\
	    OUT(val_name);						\
    } while(0);

#define KLUDGES_CMP(val_name) do {					\
	if (tl_cmp(# val_name,						\
		   &h1->val_name, &h2->val_name, skip_kludges) != 0)	\
	    OUT(val_name);						\
    } while(0);

static bool same_prefix(char **prefixes, char *l1, char *l2)
{
    size_t len;

    if (prefixes == NULL)
	return false;

    for (; *prefixes != NULL; prefixes++) {
	len = strlen(*prefixes);
	/* Skip first ^A */
	if ((strncmp(*prefixes, l1 + 1, len) == 0) &&
	    (strncmp(*prefixes, l2 + 1, len) == 0))
	    return true;
    }
    return false;
}

static int tl_cmp(const char *field, Textlist *tl1, Textlist *tl2,
		  char **skip_prefixes)
{
    Textline *l1, *l2;
    int ret = 0;

    for (l1 = tl1->first, l2 = tl2->first;
	 (l1 != NULL) && (l2 != NULL);
	 l1 = l1->next, l2 = l2->next) {

	if (strcmp(l1->line, l2->line) != 0) {

	    if (same_prefix(skip_prefixes, l1->line, l2->line))
		continue;

	    fprintf(stderr, "- %s line1: %s\n", field, l1->line);
	    fprintf(stderr, "+ %s line2: %s\n", field, l2->line);
	    ret = 1;
	}
    }

    if ((l1 == NULL) != (l2 == NULL)) {
	fprintf(stderr, "%s line number different\n", field);
	ret = 1;
    }

    return ret;
}

static int packet_hdr_cmp(struct packet *p1, struct packet *p2)
{
    Packet *h1 = &p1->hdr;
    Packet *h2 = &p2->hdr;
    int ret = 0;


    NODE_CMP(from);
    NODE_CMP(to);

    /* skip time since it is current time */
    /* VAL_CMP(time); */
    VAL_CMP(baud);
    VAL_CMP(version);
    VAL_CMP(product_l);
    VAL_CMP(product_h);
    /* skip fidogate minor version for smooth updates */
    /* VAL_CMP(rev_min); */
    VAL_CMP(rev_maj);
    STRING_CMP(passwd);
    VAL_CMP(capword);

    return ret;
}

static int message_body_cmp(struct message *m1, struct message *m2)
{
    MsgBody *h1 = &m1->body;
    MsgBody *h2 = &m2->body;
    int ret = 0;

    STRING_CMP(area);
    KLUDGES_CMP(kludge);
    TL_CMP(rfc);
    TL_CMP(body);
    STRING_CMP(tear);
    STRING_CMP(origin);
    TL_CMP(seenby);
    TL_CMP(path);
    /* skip via, contains time */
    /* TL_CMP(via); */

    return ret;
}

static int message_cmp(struct message *m1, struct message *m2)
{
    Message *h1 = &m1->hdr;
    Message *h2 = &m2->hdr;
    int ret = 0;
    int rc;

    NODE_CMP(node_from);
    NODE_CMP(node_to);
    NODE_CMP(node_orig);
    VAL_CMP(attr);
    VAL_CMP(cost);
    VAL_CMP(date);
    STRING_CMP(name_to);
    STRING_CMP(name_from);
    STRING_CMP(subject);
    STRING_CMP(area);

    rc = message_body_cmp(m1, m2);
    return ret | rc;
}

static int packet_cmp(struct packet *p1, struct packet *p2)
{
    int rc;
    int i;

    if (p1->n_msgs != p2->n_msgs)
	return -1;

    rc = packet_hdr_cmp(p1, p2);
    if (rc != 0)
	return rc;

    for (i = 0; i < p1->n_msgs; i++) {
	rc = message_cmp(&p1->msgs[i], &p2->msgs[i]);
	if (rc != 0)
	    return rc;
    }
    return 0;
}

static struct packet *packet_read_fp(FILE *fp)
{
	Node dummy_to = { 0 };
	Node dummy_from = { 0 };
	struct packet *pkt;
	struct message *msg;
	int rc;

	pkt = packet_new();

	rc = pkt_get_hdr(fp, &pkt->hdr);
	if (rc == ERROR)
	    goto err;

	rc = pkt_get_int16(fp);
	if (rc != MSG_TYPE)
	    goto err;

	/* xfeof() is not a feof() wrapper */
	while (!xfeof(fp)) {

	    msg = packet_msg_new(pkt);

	    rc = pkt_get_msg_hdr(fp, &msg->hdr, false);
	    if (rc == ERROR)
		goto err;

	    rc = pkt_get_body_parse(fp, &msg->body, &dummy_from, &dummy_to);
	    if (rc == ERROR)
		goto err;
	}

	return pkt;
err:
	packet_free(pkt);
	return NULL;
}

static struct packet *packet_read(const char *fn)
{
    FILE *fp;
    struct packet *pkt;

    fp = fopen(fn, "r");
    if (fp == NULL)
	return NULL;

    pkt = packet_read_fp(fp);
    fclose(fp);

    return pkt;
}

static void usage(void)
{
    fprintf(stderr, "Usage: <command> pkt1 pkt2\n");
}

int main(int argc, char **argv)
{
    struct packet *pkt1, *pkt2;
    int ret = 0;

    if (argc != 3) {
	usage();
	return 1;
    }

    pkt1 = packet_read(argv[1]);
    pkt2 = packet_read(argv[2]);

    if ((pkt1 == NULL) || (pkt2 == NULL)) {
	fprintf(stderr, "Could not open one file\n");
	ret = 2;
	goto out;
    }

    if (packet_cmp(pkt1, pkt2) != 0)
	ret = 3;
out:
    packet_free(pkt2);
    packet_free(pkt1);

    return ret;
}
