#
# $Id: Makefile.am,v 5.2 2004/11/23 00:50:36 anray Exp $
#
# Makefile.am FIDOGATE doc/
#

configs  =  mailer/postfix/master.cf \
	    mailer/sendmail/mailer/ftn.m4 \
	    news/leafnode/config.h \
	    news/leafnode/Makefile \
	    examples/dyff/fidogate.conf.sample \
	    examples/point/fidogate.conf \
	    examples/dyff/packing.sample

$(configs):
	$(SUBSTCMD) $@.in > $@

all-am: $(configs)

clean-am:
	rm -f $(configs)
