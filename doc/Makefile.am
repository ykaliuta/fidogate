#
# $Id$
#
# Makefile.am FIDOGATE doc/
#


dist_sysconf_DATA = examples/bounce/bounce.acl \
		    examples/bounce/bounce.acl_netmail \
		    examples/bounce/bounce.addrinto \
		    examples/bounce/bounce.down \
		    examples/bounce/bounce.insecure \
		    examples/bounce/bounce.noto \
		    examples/bounce/bounce.restricted \
		    examples/dyff/acl.sample \
		    examples/dyff/aliases.sample \
		    examples/dyff/areafix.help.sample \
		    examples/dyff/areafix.help.rus \
		    examples/dyff/fidogate.conf.sample \
		    examples/dyff/fidokill.sample \
		    examples/dyff/ftnacl.sample \
		    examples/dyff/hosts.sample \
		    examples/dyff/packing.sample \
		    examples/dyff/passwd.sample \
		    examples/dyff/routing.sample \
		    examples/dyff/spyes.sample \
		    examples/dyff/uplinks.sample

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

dist_var_DATA=   examples/dyff/areas.bbs.sample \
		    examples/dyff/fareas.bbs.sample

clean-am:
	rm -f $(configs)
