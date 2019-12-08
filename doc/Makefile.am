#
# Makefile.am FIDOGATE doc/
#

include $(top_builddir)/build/rules.make

configs  =  mailer/postfix/master.cf \
	    mailer/sendmail/mailer/ftn.m4 \
	    news/leafnode/config.h \
	    news/leafnode/Makefile \
	    examples/dyff/fidogate.conf.sample \
	    examples/point/fidogate.conf \
	    examples/dyff/packing.sample

subst_scripts        = $(configs)
CLEANFILES           = $(configs)

all-am: $(configs)
