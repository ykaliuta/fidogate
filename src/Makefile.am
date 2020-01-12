#:ts=8
#
#    Makefile for FIDOGATE src/
#

#
# Normally it should not be necessary to modify this file, see
# ../config.make and ../rules.make
#

include $(top_builddir)/build/rules.make

AM_CPPFLAGS		= -Iinclude

LDADD			= common/libfidogate.a
LDADD			+= @LIBICONV@

AM_CFLAGS		= -Wall

CLEANFILES=
DISTCLEANFILES=
subst_scripts =
install_hooks =
EXTRA_DIST=

########## common, libfidogate

noinst_LIBRARIES = common/libfidogate.a
common_libfidogate_a_SOURCES = $(LIBFIDOGATE_SRC)

LIBFIDOGATE_SRC	   = common/acl.c \
	    common/acl_ftn.c \
	    common/active.c \
	    common/address.c \
	    common/aliases.c \
	    common/areas.c \
	    common/areasbbs.c \
	    common/binkley.c \
	    common/bounce.c \
	    common/cf_funcs.c \
	    common/charset.c \
	    common/config.c \
	    common/crc16.c \
	    common/crc32.c \
	    common/date.c \
	    common/dbz.c \
	    common/dir.c \
	    common/file.c \
	    common/flo.c \
	    common/fopen.c \
	    common/ftnaddr.c \
	    common/getopt.c \
	    common/getopt1.c \
	    common/gettime.c \
	    common/hosts.c \
	    common/histdb.c \
	    common/kludge.c \
	    common/lock.c \
	    common/log.c \
	    common/mail.c \
	    common/message.c \
	    common/mime.c \
	    common/misc.c \
	    common/msgid.c \
	    common/node.c \
	    common/outpkt.c \
	    common/packet.c \
	    common/parsedate.y \
	    common/parsenode.c \
	    common/passwd.c \
	    common/read.c \
	    common/rematch.c \
	    common/rfcaddr.c \
	    common/rfcheader.c \
	    common/routing.c \
	    common/sequencer.c \
	    common/spyes.c \
	    common/strtok_r.c \
	    common/textlist.c \
	    common/tick.c \
	    common/tmps.c \
	    common/uplinks.c \
	    common/version.c \
	    common/wildmat.c \
	    common/xalloc.c \
	    common/xstrnlen.c

DISTCLEANFILES	+= common/cf_* common/parsedate.c common/parsedate.h

########## areafix

bin_PROGRAMS			= areafix/ftnaf areafix/ftnafutil areafix/pkttmpl
libexec_PROGRAMS		= areafix/ftnafpkt areafix/ftnafmail

areafix_ftnaf_SOURCES		= areafix/ftnaf.c areafix/areafix.c areafix/mailer.c
areafix_ftnafutil_SOURCES	= areafix/ftnafutil.c areafix/areafix.c
areafix_ftnafmail_SOURCES	= areafix/ftnafmail.c areafix/areafix.c
areafix_ftnafpkt_SOURCES	= areafix/ftnafpkt.c areafix/areafix.c

install_hooks 			+= areafix_install
areafix-install:
	chmod 4755 $(DESTDIR)$(libexecdir)/ftnafmail$(EXEEXT)

########## gate

gate_scripts		= gate/ftninrecomb gate/send-fidogate
EXTRA_DIST		+= $(gate_scripts:=.in)
libexec_SCRIPTS		= $(gate_scripts)

if ftnnews
FTNNEWS			= gate/ftnnews
endif

libexec_PROGRAMS	+= gate/ftn2rfc gate/ftnin gate/ftninpost \
			   gate/rfc2ftn gate/ftnmail $(FTNNEWS)

subst_scripts		+= $(gate_scripts)
CLEANFILES		+= $(gate_scripts)

install_hooks		+= gate_install
gate_install:
	chmod 4755 $(DESTDIR)$(libexecdir)/ftnmail$(EXEEXT)
	@if test -e $(srcdir)/ftnnews$(EXEEXT); then \
	chmod 4750 $(DESTDIR)$(libexecdir)/ftnnews$(EXEEXT); \
	echo " chmod 4750 $(DESTDIR)$(libexecdir)/ftnnews$(EXEEXT)"; \
	if test `id -u` -eq 0; then \
	chown news $(DESTDIR)$(libexecdir)/send-fidogate; \
	echo " chown news $(DESTDIR)$(libexecdir)/send-fidogate"; \
	fi; \
	fi

########## include

noinst_HEADERS = include/constants.h \
		 include/dbz.h \
		 include/declare.h \
		 include/fidogate.h \
		 include/getopt.h \
		 include/node.h \
		 include/packet.h \
		 include/prototypes.h \
		 include/structs.h

INCLUDE_GENERATED_H = include/paths.h include/cf_funcs.h
INCLUDE_GENERATED_C = common/cf_funcs.c common/cf_abbrev.c common/cf_env.c

SUFFIXES = .h .c .mak

.mak.h:
	$(AM_V_GEN)cp $< $@

.mak.c:
	$(AM_V_GEN)cp $< $@

BUILT_SOURCES		= $(INCLUDE_GENERATED_C) $(INCLUDE_GENERATED_H)

DISTCLEANFILES		+= include/cf_* include/paths.*

########## tick

tick_scripts		= tick/ftntickpost
EXTRA_DIST		+= $(tick_scripts:=.in)

bin_PROGRAMS		+= tick/ftnhatch
libexec_PROGRAMS	+= tick/ftntick
libexec_SCRIPTS		+= $(tick_scripts)

tick_ftntick_SOURCES	= tick/ftntick.c areafix/areafix.c

subst_scripts		+= $(tick_scripts)
CLEANFILES		+= $(tick_scripts)


########### toss

toss_scripts		= toss/report_traffic
EXTRA_DIST		+= $(toss_scripts:=.in)

bin_PROGRAMS		+= toss/ftnexpire toss/runinc toss/ngoper
libexec_PROGRAMS	+= toss/ftn2ftn toss/ftnpack toss/ftnroute toss/ftntoss
libexec_SCRIPTS		+= $(toss_scripts)

toss_ftntoss_SOURCES	= toss/ftntoss.c \
			  toss/history.c \
			  areafix/areafix.c \
                          areafix/mailer.c

subst_scripts		+= $(toss_scripts)
CLEANFILES		+= $(toss_scripts)

install_hooks		+= toss_install
toss_install:
	if test `id -u` -eq 0; then \
	chown news $(DESTDIR)$(bindir)/ngoper; \
	fi
	chmod 4550 $(DESTDIR)$(bindir)/ngoper

########### util

util_scripts		= util/pktmore
EXTRA_DIST		+= $(util_scripts:=.in)

bin_PROGRAMS		+= util/ftnfattach util/sumcrc util/ftnoutpkt util/pktdebug
bin_PROGRAMS		+= util/ftngenpkt
libexec_PROGRAMS	+= util/confval util/ftnflo
bin_SCRIPTS		= $(util_scripts)

subst_scripts		+= $(util_scripts)
CLEANFILES		+= $(util_scripts)
