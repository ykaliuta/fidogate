#
# Makefile FIDOGATE TOPDIR
#

include $(top_builddir)/build/config.make
include $(top_builddir)/build/rules.make

TOPDIR		= .

SUBDIRS		= src tests

FDESTDIR	= $(DEFAULT_V_CONFIGDIR) \
		  $(DEFAULT_V_LOGDIR) \
		  $(DEFAULT_V_VARDIR) \
		  $(DEFAULT_V_VARDIR)/seq \
		  $(DEFAULT_V_LOCKDIR) \
		  $(DEFAULT_V_SPOOLDIR) \
		  $(DEFAULT_V_SPOOLDIR)/outrfc \
		  $(DEFAULT_V_SPOOLDIR)/outrfc/mail \
		  $(DEFAULT_V_SPOOLDIR)/outrfc/news \
		  $(DEFAULT_V_SPOOLDIR)/outpkt \
		  $(DEFAULT_V_SPOOLDIR)/outpkt/mail \
		  $(DEFAULT_V_SPOOLDIR)/outpkt/news \
		  $(DEFAULT_V_SPOOLDIR)/toss \
		  $(DEFAULT_V_SPOOLDIR)/toss/toss \
		  $(DEFAULT_V_SPOOLDIR)/toss/route \
		  $(DEFAULT_V_SPOOLDIR)/toss/pack \
		  $(DEFAULT_V_SPOOLDIR)/toss/bad \
		  $(DEFAULT_V_SPOOLDIR)/ftpin \
		  $(DEFAULT_V_SPOOLDIR)/uuin \
		  $(DEFAULT_V_BTBASEDIR) \
		  $(DEFAULT_V_BTBASEDIR)/tick \
		  $(DEFAULT_V_INBOUND) \
		  $(DEFAULT_V_INBOUND)/bad \
		  $(DEFAULT_V_INBOUND)/tmpunpack \
		  $(DEFAULT_V_PINBOUND) \
		  $(DEFAULT_V_PINBOUND)/bad \
		  $(DEFAULT_V_PINBOUND)/tmpunpack

OWNER = @OWNER@
GROUP = @GROUP@

install-exec-hook: installfdistdir

installfdistdir:
	@for d in $(FDESTDIR); do \
	if ! test -d $(DESTDIR)$$d; then \
	$(mkinstalldirs) $(DESTDIR)$$d || exit 1; \
	echo "create directory $(DESTDIR)$$d"; \
	fi; \
	done;
	@if test `id -u` -eq 0 && \
	   test -n "${OWNER}" && \
           test -n "${GROUP}" && \
           id "${OWNER}" >/dev/null 2>/dev/null ; then \
	for d in $(FDESTDIR); do \
	chown ${OWNER}:${GROUP} $(DESTDIR)$$d || exit 1; \
	echo "chown ${OWNER}:${GROUP} $(DESTDIR)$$d"; \
	done; \
	else echo "Cannot change owner to ${OWNER}:${GROUP}"; \
	fi

ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST = build/config.rpath m4/ChangeLog
EXTRA_DIST += build/rules.make build/subst.pl config.pl
EXTRA_DIST += autogen.sh
EXTRA_DIST += packages/rpm/fidogate.spec

#debian
EXTRA_DIST += debian/copyright \
            debian/fidogate.dirs \
            debian/fidogate-doc.docs \
            debian/fidogate-postfix.install \
            debian/fidogate-postfix.dirs \
            debian/fidogate.postrm \
            debian/patches/series \
            debian/source/format \
            debian/fidogate.postinst \
            debian/fidogate.install \
            debian/fidogate.cron.d \
            debian/rules \
            debian/compat \
            debian/changelog \
            debian/fidogate-sendmail.install \
            debian/control \
            debian/fidogate.logrotate \
            debian/fidogate-sendmail.dirs

noinst_HEADERS = version.h

########## scripts

binscripts = scripts/areas/areassucksync \
	  scripts/areas/areasbbssync \
	  scripts/nodelist/nl-del \
	  scripts/nodelist/nl-diff \
	  scripts/nodelist/nl-autoupd \
	  scripts/maintenance/logstat \
	  scripts/maintenance/logdaily \
	  scripts/maintenance/runchklock \
	  scripts/maintenance/logsendmail \
	  scripts/maintenance/logcheck \
	  scripts/maintenance/logreport \
	  scripts/misc/senduumail \
	  scripts/misc/hosts2dns \
	  scripts/misc/senduu \
	  scripts/misc/recvuu \
	  scripts/outbound/out-ls \
	  scripts/outbound/outb \
	  scripts/outbound/out-freq \
	  scripts/outbound/out-attach \
	  scripts/outbound/out-rm0 \
	  scripts/outbound/out-manip \
	  scripts/outbound/outb-kill \
	  scripts/outbound/out-rmbsy

noinstscripts = scripts/www/gate.cgi \
	  scripts/filebase/fb-filelist \
	  scripts/filebase/fb-chkfiles \
	  scripts/maintenance/fidogate.logrotate \
	  scripts/maintenance/rc.fidogate \
	  scripts/nodelist/html-tmpl \
	  scripts/nodelist/nl-2hosts \
	  scripts/nodelist/nl-print \
	  scripts/nodelist/nl-2routing \
	  scripts/nodelist/nl-html \
	  scripts/nodelist/nl-isdn

scripts = $(binscripts) $(noinstscripts)

bin_SCRIPTS		= $(binscripts)
noinst_SCRIPTS		= $(noinstscripts)

subst_scripts		= $(scripts)
CLEANFILES		= $(scripts)

EXTRA_DIST += $(scripts:=.in)

########## doc

docs  =  doc/mailer/postfix/master.cf \
	    doc/mailer/sendmail/mailer/ftn.m4 \
	    doc/news/leafnode/config.h \
	    doc/news/leafnode/Makefile \
	    doc/examples/dyff/fidogate.conf.sample \
	    doc/examples/point/fidogate.conf \
	    doc/examples/dyff/packing.sample

subst_scripts        += $(docs)
CLEANFILES           += $(docs)

EXTRA_DIST += $(docs:=.in)
EXTRA_DIST += TODO.rus doc/README doc/README.ru doc/FAQ.ru

all-am: $(docs)

########## configs

dist_sysconf_DATA = configs/bounce.acl \
		    configs/bounce.acl_netmail \
		    configs/bounce.addrinto \
		    configs/bounce.down \
		    configs/bounce.insecure \
		    configs/bounce.noto \
		    configs/bounce.restricted \
		    configs/acl.sample \
		    configs/aliases.sample \
		    configs/areas.sample \
		    configs/areafix.help.sample \
		    configs/areafix.help.rus \
		    configs/fidokill.sample \
		    configs/ftnacl.sample \
		    configs/hosts.sample \
		    configs/passwd.sample \
		    configs/routing.sample \
		    configs/spyes.sample \
		    configs/uplinks.sample

sysconf_DATA = $(generated)

generated = configs/fidogate.conf.sample \
	    configs/packing.sample

subst_scripts        += $(generated)
CLEANFILES           += $(generated)

all-am: $(generated)

dist_var_DATA = configs/areas.bbs.sample \
	            configs/fareas.bbs.sample

EXTRA_DIST += $(generated:=.in)
