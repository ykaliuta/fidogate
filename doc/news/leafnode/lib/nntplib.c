/*
 * (C) Maint Laboratory 2003
 * Author: Elohin Igor
 * e-mail: maint@unona.ru
 * fido  : 2:5070/222.52
 * URL   : http://maint.unona.ru
 */
#include <stdio.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/socket.h>
#include <errno.h>
#include "../common.h"

#define NNTP_BAD_COMMAND_VAL 500

FILE *inp_nntp;
FILE *out_nntp;

int nntp_connect(char *host, int port, char *line)
{
    struct hostent *hp;
    char **ap;
    char *p;
    char *dest;
    struct sockaddr_in server;
    int i, j;
    char buf[BUFSIZE + 2];

    if ((hp = gethostbyname(host)) == NULL) {
	error("Not a host name");
	return -1;
    }
    ap = hp->h_addr_list;
    /* Set up the socket address. */
    memset((void *) &server, 0, sizeof(server));
    server.sin_family = hp->h_addrtype;
    server.sin_port = htons(port);
    /* Loop through the address list, trying to connect. */
    for (; ap && *ap; ap++) {
	/* Make a socket and try to connect. */
	if ((i = socket(hp->h_addrtype, SOCK_STREAM, 0)) < 0)
	    break;
	p = (char *) *ap;
	for (dest = (char *) &server.sin_addr, j = hp->h_length; --j >= 0;)
	    *dest++ = *p++;
	if (connect(i, (struct sockaddr *) &server, sizeof server) < 0) {
	    close(i);
	    continue;
	}

	/* Connected -- now make sure we can post. */
	if ((inp_nntp = fdopen(i, "r")) == NULL) {
	    close(i);
	    continue;
	}
	if (fgets(buf, sizeof(buf), inp_nntp) == NULL) {
	    fclose(inp_nntp);
	    close(i);
	    continue;
	}
	j = atoi(buf);
	if (j != 200 && j != 201) {
	    fclose(inp_nntp);
	    close(i);
	    break;
	}

	if ((out_nntp = fdopen(dup(i), "w")) == NULL) {
	    fclose(inp_nntp);
	    close(i);
	    continue;
	}
	return 0;
    }
    return -1;
}
int connect_server(char *host, int port)
{
    char line1[BUFSIZE + 2];
    char line2[BUFSIZE];

    if (nntp_connect(host, port, line1) < 0) {
	if (line1[0] == '\0') {
	    error("I/O problem");
	    return -1;
	}
	error("Server rejected connection; return it's reply code.");
	return atoi(line1);
    }

    /* Send the INN command; if understood, use that reply. */
    put_server("MODE READER");
    if (get_server(line2) < 0)
	return -1;
    if (atoi(line2) != NNTP_BAD_COMMAND_VAL)
	strcpy(line1, line2);
    /* Connected; return server's reply code. */
    return atoi(line1);
}
void disconnect_server()
{
    char buf[BUFSIZE];

    if (out_nntp != NULL && inp_nntp != NULL) {
	put_server("QUIT");
	fclose(out_nntp);
	out_nntp = NULL;

	get_server(buf);
	fclose(inp_nntp);
	inp_nntp = NULL;
    }
}
int get_server(char *buf)
{
    char *p;

    if (fgets(buf, BUFSIZE, inp_nntp) == NULL)
	return -1;
    p = &buf[strlen(buf)];
    if (p >= &buf[2] && p[-2] == '\r' && p[-1] == '\n')
	p[-2] = '\0';
    return 0;
}
char *get_line(char *buf)
{
    char *p;

    if (fgets(buf, BUFSIZE, inp_nntp) == NULL)
	return NULL;
    p = &buf[strlen(buf)];
    if (p >= &buf[2] && p[-2] == '\r' && p[-1] == '\n')
	p[-2] = '\0';
    return buf;
}
void put_server(char *buf)
{
    fprintf(out_nntp, "%s\r\n", buf);
    fflush(out_nntp);
}
