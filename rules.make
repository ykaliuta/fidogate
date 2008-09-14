# -*- Makefile -*-
#
# $Id: rules.make,v 4.6 1998/11/15 10:59:01 mj Exp $
#
# Common rules for all FIDOGATE Makefiles
#

.SUFFIXES: .pl .sh .mc .cf

%.o:		%.c
	$(CC) $(CFLAGS) $(LOCAL_CFLAGS) -c $<

%:		%.o
	$(CC) $(LFLAGS) $(LOCAL_LFLAGS) -o $* $*.o $(LIBS)

%:		%.pl
	$(PERL) $(TOPDIR)/subst.pl -t$(TOPDIR) -p $< >$*
	chmod +x $*

%.cgi:		%.cgi.pl
	$(PERL) $(TOPDIR)/subst.pl -t$(TOPDIR) -p -o '-Tw' $< >$*.cgi
	chmod +x $*.cgi

%:		%.sh
	$(PERL) $(TOPDIR)/subst.pl -t$(TOPDIR) $< >$*
	chmod +x $*

%.cf:		%.mc
	$(M4) $(M4OPTIONS) $< >$*.cf

#$(LIB)(%.o):	%.o
#	$(AR) r $(LIB) $<
