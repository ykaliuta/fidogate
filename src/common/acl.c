/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FIDO NetMail/EchoMail
 *
 * $Id: acl.c,v 4.3 2000/10/18 21:53:57 mj Exp $
 *
 *****************************************************************************/

#include "fidogate.h"



#ifdef AI_8


/*
 * Local prototypes
 */
static Acl  *acl_parse_line	(char *);
static int   acl_do_file	(char *);
static char *acl_lookup		(char *);
static void  pna_notify_init	(char *);


/*
 * ACL list
 */
static Acl *acl_list = NULL;
static Acl *acl_last = NULL;

static char **pna_notify_list = NULL;
static char **ngrp_pat_list = NULL;



static Acl *acl_parse_line(char *buf)
{
    Acl *p;

    char *f, *n;
	
    f = strtok(buf,  " \t");	/* E-Mail address pattern */
    n = strtok(NULL, " \t");	/* Newsgroup pattern */
    if(f==NULL || n==NULL)
	return NULL;
    
    if(strieq(f, "include"))
    {
	acl_do_file(n);
	return NULL;
    }

    if(strieq(f, "PostingNotAllowedNotify"))
    {
	pna_notify_init(n);
	return NULL;
    }
    
    p = (Acl *)xmalloc(sizeof(Acl));
    p->next      = NULL;
    p->email_pat = strsave(f);
    p->ngrp_pat  = strsave(n);
	
    debug(15, "acl: %s       %s", p->email_pat, p->ngrp_pat);

    return p;
}



static int acl_do_file(char *name)
{
    FILE *fp;
    Acl *p;

    debug(14, "Reading ACL file %s", name);
    
    fp = fopen_expand_name(name, R_MODE_T, FALSE);
    if(!fp)
	return ERROR;
    
    while(cf_getline(buffer, BUFFERSIZE, fp))
    {
	p = acl_parse_line(buffer);
	if(!p)
	    continue;
	
	/* Put into linked list */
	if(acl_list)
	    acl_last->next = p;
	else
	    acl_list       = p;
	acl_last       = p;
    }
    
    fclose(fp);

    return OK;
}


void acl_init(void)
{
    acl_do_file( cf_p_acl() );
}


char *acl_lookup(char *email)
{
    Acl  *p;
    char *ngrp = NULL;
    
    for(p=acl_list; p; p=p->next)
    {
	if(wildmat(email, p->email_pat))
	    ngrp = p->ngrp_pat;
    }

    debug(7, "acl_lookup(): From=%s, ngrp=%s", email, ngrp);
    
    return ngrp;
}


void acl_ngrp(RFCAddr rfc_from)
{
    char  email[MAXINETADDR];

    BUF_COPY(email, s_rfcaddr_to_asc(&rfc_from, FALSE));
    list_init(&ngrp_pat_list, acl_lookup(email));
}


int acl_ngrp_lookup(char *list)
{
    static char **ngrp_list = NULL;

    list_init(&ngrp_list, list);
    return list_match(FALSE, ngrp_pat_list, ngrp_list);
}



void pna_notify_init(char *list)
{
    list_init(&pna_notify_list, list);
}


int pna_notify(char *email)
{
    static char **email_list = NULL;

    list_init(&email_list, email);
    return list_match(FALSE, pna_notify_list, email_list);
}



/*=================================================================*/
/* This is part of INN 1.7.2                                       */
/* Modify by Andy Igoshin                                          */

/* =()<#define DISPOSE(p)		free((@<POINTER>@ *)p)>()= */
#define DISPOSE(p)		free((void *)p)

#define NEW(T, c)			\
	((T *)xmalloc((unsigned int)(sizeof (T) * (c))))

/*
**  <ctype.h> usually includes \n, which is not what we want.
*/
#define ISWHITE(c)			((c) == ' ' || (c) == '\t')



/*
**  Parse a string into a NULL-terminated array of words; return number
**  of words.  If argvp isn't NULL, it and what it points to will be
**  DISPOSE'd.
*/
int
Argify(line, argvp)
    char		*line;
    char		***argvp;
{
    register char	**argv;
    register char	*p;
    register int	i;

    if (*argvp != NULL) {
	DISPOSE(*argvp[0]);
	DISPOSE(*argvp);
    }

    /*  Copy the line, which we will split up. */
    while (ISWHITE(*line))
	line++;
    i = strlen(line);
    p = strsave(line);

    /* Allocate worst-case amount of space. */
    for (*argvp = argv = NEW(char*, i + 2); *p; ) {
	/* Mark start of this word, find its end. */
	for (*argv++ = p; *p && !ISWHITE(*p); )
	    p++;
	if (*p == '\0')
	    break;

	/* Nip off word, skip whitespace. */
	for (*p++ = '\0'; ISWHITE(*p); )
	    p++;
    }
    *argv = NULL;
    return argv - *argvp;
}



/*
**  Parse a newsgroups line, return TRUE if there were any.
*/
int list_init(char ***argvp, char *list)
{
    register char	*p;

    if(!list)
	return FALSE;

    for (p = list; *p; p++)
	if (*p == ',')
	    *p = ' ';

    return Argify(list, argvp) != 0;
}



/*
**  Match a list of newsgroup specifiers against a list of newsgroups.
**  func is called to see if there is a match.
*/
int list_match(register int match, char **Pats, char **list)
{
    register int	i;
    register char	*p;

    if (!Pats)
    	return FALSE;
    if (!list)
    	return FALSE;
    if (Pats[0] == NULL)
	return FALSE;

    for ( ; *list; list++) {
	for (i = 0; (p = Pats[i]) != NULL; i++) {
	    if (p[0] == '!') {
		if (wildmat(*list, ++p))
		    match = FALSE;
	    }
	    else if (wildmat(*list, p))
		match = TRUE;
	}
    }

    if (match)
	return TRUE;

    return FALSE;
}


#endif /**AI_8**/
