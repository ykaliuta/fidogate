# New ports collection makefile for: fidogate5
# Whom:                 Andrey Slusar <rusfidogate@users.sourceforge.net>
# Date created:         29 Aug 2003
# $FreeBSD$
#

PORTNAME=		fidogate
PORTVERSION=		5.2.0
CATEGORIES=		news mail
MASTER_SITES=		${MASTER_SITE_SOURCEFORGE}
DISTNAME=		fidogate5.2.1ds

MAINTAINER=		rusfidogate@users.sourceforge.net
COMMENT=		Fido-Internet Gateway and Tosser

USE_BZIP2=      	yes
GNU_CONFIGURE=  	yes
USE_PERL5=      	yes

INSTALL_SHLIB=  	yes

SPOOLDIR?=/var/spool/fido/gate
BTBASEDIR?=/var/spool/fido/bt
LOCKDIR?=/var/run/fidogate
LOGDIR?=/var/log/fido/gate
VARDIR?=${PREFIX}/var/fidogate

PLIST_SUB+=		SPOOLDIR=${SPOOLDIR} \
			BTBASEDIR=${BTBASEDIR} \
			LOGDIR=${LOGDIR} \
			LOCKDIR=${LOCKDIR} \
			PREFIX=${PREFIX}

HAS_CONFIGURE=		yes
.if !defined(WITH_BSO)
CONFIGURE_ARGS+=	--enable-amiga-out
.endif
.if !defined(WITH_DESCDIR)
CONFIGURE_ARGS+=	--disable-desc-dir
.endif
.if !defined(WITHOUT_ALIASES)
CONFIGURE_ARGS+=	--enable-aliases-local
.endif
.if defined(WITH_DBC)
CONFIGURE_ARGS+=	--enable-dbc-history
.endif
.if defined(DNT_NETMAIL)
CONFIGURE_ARGS+=	--enable-dnt-netmail
.endif
.if defined(WITHOUT_FMSGID)
CONFIGURE_ARGS+=	--disable-fs-msgid
.endif
.if !defined(WITH_AVAIL)
CONFIGURE_ARGS+=	--disable-af-avail
.endif
.if defined(WITHOUT_BEST_AKA)
CONFIGURE_ARGS+=	--disable-best-aka
.endif
CONFIGURE_ARGS+=	--libexec=${PREFIX}/libexec/fidogate \
			--sysconfdir=${PREFIX}/etc/fido/gate \
			--libdir=${PREFIX}/lib/fidogate \
			--with-lockdir=${LOCKDIR} \
			--with-logdir=${LOGDIR} \
			--with-vardir=${VARDIR} \
			--with-spooldir=${SPOOLDIR} \
			--with-btbasedir=${BTBASEDIR}

do-install:
	(cd ${WRKSRC}; ${MAKE} -f ${MAKEFILE} ${MAKE_ARGS} install)

.if !defined(NOPORTDOCS)
	${MKDIR} ${PREFIX}/share/doc/fidogateds/old
	${INSTALL_DATA} ${WRKSRC}/doc/README.ru ${PREFIX}/share/doc/fidogateds/README.ru
	${INSTALL_DATA} ${WRKSRC}/doc/README ${PREFIX}/share/doc/fidogateds/README
	${INSTALL_DATA} ${WRKSRC}/doc/FAQ.ru ${PREFIX}/share/doc/fidogateds/FAQ.ru
	${INSTALL_DATA} ${WRKSRC}/ChangeLog ${PREFIX}/share/doc/fidogateds/ChangeLog
	${MKDIR} ${PREFIX}/share/examples/fidogateds/mailer/postfix
	${INSTALL_DATA} ${WRKSRC}/doc/mailer/postfix/master.cf ${PREFIX}/share/examples/fidogateds/mailer/postfix/master.cf
	${INSTALL_DATA} ${WRKSRC}/doc/mailer/postfix/transport ${PREFIX}/share/examples/fidogateds/mailer/postfix/transport
	${MKDIR} ${PREFIX}/share/examples/fidogateds/mailer/sendmail
	${INSTALL_DATA} ${WRKSRC}/doc/mailer/sendmail/README.FIDOGATE ${PREFIX}/share/examples/fidogateds/mailer/sendmail/README.FIDOGATE
	${INSTALL_DATA} ${WRKSRC}/doc/mailer/sendmail/mkclean ${PREFIX}/share/examples/fidogateds/mailer/sendmail/mkclean
	${INSTALL_DATA} ${WRKSRC}/doc/mailer/sendmail/mklinks ${PREFIX}/share/examples/fidogateds/mailer/sendmail/mklinks
	${MKDIR} ${PREFIX}/share/examples/fidogateds/mailer/sendmail/mailer
	${INSTALL_DATA} ${WRKSRC}/doc/mailer/sendmail/mailer/ftn.m4  ${PREFIX}/share/examples/fidogateds/mailer/sendmail/mailer/ftn.m4
	${INSTALL_DATA} ${WRKSRC}/doc/old/FAQ_Fidogate ${PREFIX}/share/doc/fidogateds/old/FAQ_Fidogate
	${INSTALL_DATA} ${WRKSRC}/doc/old/FIDOGATE_ftrack ${PREFIX}/share/doc/fidogateds/old/FIDOGATE_ftrack
	${MKDIR} ${PREFIX}/share/examples/fidogateds/dyff
	${INSTALL_DATA} ${WRKSRC}/doc/examples/dyff/acl.sample ${PREFIX}/share/examples/fidogateds/dyff/acl.sample
	${INSTALL_DATA} ${WRKSRC}/doc/examples/dyff/aliases.sample ${PREFIX}/share/examples/fidogateds/dyff/aliases.sample
	${INSTALL_DATA} ${WRKSRC}/doc/examples/dyff/areafix.help.rus ${PREFIX}/share/examples/fidogateds/dyff/areafix.help.rus
	${INSTALL_DATA} ${WRKSRC}/doc/examples/dyff/areafix.help.sample ${PREFIX}/share/examples/fidogateds/dyff/areafix.help.sample
	${INSTALL_DATA} ${WRKSRC}/doc/examples/dyff/areas.bbs.sample ${PREFIX}/share/examples/fidogateds/dyff/areas.bbs.sample
	${INSTALL_DATA} ${WRKSRC}/doc/examples/dyff/areas.sample ${PREFIX}/share/examples/fidogateds/dyff/areas.sample
	${INSTALL_DATA} ${WRKSRC}/doc/examples/dyff/fareas.bbs.sample ${PREFIX}/share/examples/fidogateds/dyff/fareas.bbs.sample
	${INSTALL_DATA} ${WRKSRC}/doc/examples/dyff/fidogate.conf.sample ${PREFIX}/share/examples/fidogateds/dyff/fidogate.conf.sample
	${INSTALL_DATA} ${WRKSRC}/doc/examples/dyff/fidokill.sample ${PREFIX}/share/examples/fidogateds/dyff/fidokill.sample
	${INSTALL_DATA} ${WRKSRC}/doc/examples/dyff/ftnacl.sample ${PREFIX}/share/examples/fidogateds/dyff/ftnacl.sample
	${INSTALL_DATA} ${WRKSRC}/doc/examples/dyff/hosts.sample ${PREFIX}/share/examples/fidogateds/dyff/hosts.sample
	${INSTALL_DATA} ${WRKSRC}/doc/examples/dyff/packing.sample ${PREFIX}/share/examples/fidogateds/dyff/packing.sample
	${INSTALL_DATA} ${WRKSRC}/doc/examples/dyff/passwd.sample ${PREFIX}/share/examples/fidogateds/dyff/passwd.sample
	${INSTALL_DATA} ${WRKSRC}/doc/examples/dyff/routing.sample ${PREFIX}/share/examples/fidogateds/dyff/routing.sample
	${INSTALL_DATA} ${WRKSRC}/doc/examples/dyff/spyes.sample ${PREFIX}/share/examples/fidogateds/dyff/spyes.sample
	${INSTALL_DATA} ${WRKSRC}/doc/examples/dyff/uplinks.sample ${PREFIX}/share/examples/fidogateds/dyff/uplinks.sample
	${MKDIR} ${PREFIX}/share/examples/fidogateds/point
	${INSTALL_DATA} ${WRKSRC}/doc/examples/point/areas ${PREFIX}/share/examples/fidogateds/point/areas
	${INSTALL_DATA} ${WRKSRC}/doc/examples/point/areas.bbs ${PREFIX}/share/examples/fidogateds/point/areas.bbs
	${INSTALL_DATA} ${WRKSRC}/doc/examples/point/fidogate.conf ${PREFIX}/share/examples/fidogateds/point/fidogate.conf
	${INSTALL_DATA} ${WRKSRC}/doc/examples/point/newsfeeds ${PREFIX}/share/examples/fidogateds/point/newsfeeds
	${INSTALL_DATA} ${WRKSRC}/doc/examples/point/packing ${PREFIX}/share/examples/fidogateds/point/packing
	${INSTALL_DATA} ${WRKSRC}/doc/examples/point/routing ${PREFIX}/share/examples/fidogateds/point/routing
.endif

post-install:
	${STRIP_CMD} ${PREFIX}/bin/ftnaf
	${STRIP_CMD} ${PREFIX}/bin/ftnafutil
	${STRIP_CMD} ${PREFIX}/bin/ftnexpire
	${STRIP_CMD} ${PREFIX}/bin/ftnfattach
	${STRIP_CMD} ${PREFIX}/bin/ftnhatch
	${STRIP_CMD} ${PREFIX}/bin/ftnoutpkt
	${STRIP_CMD} ${PREFIX}/bin/ngoper
	${STRIP_CMD} ${PREFIX}/bin/pktdebug
	${STRIP_CMD} ${PREFIX}/bin/pkttmpl
	${STRIP_CMD} ${PREFIX}/bin/runinc
	${STRIP_CMD} ${PREFIX}/bin/sumcrc
	${STRIP_CMD} ${PREFIX}/libexec/fidogate/confval
	${STRIP_CMD} ${PREFIX}/libexec/fidogate/ftn2ftn
	${STRIP_CMD} ${PREFIX}/libexec/fidogate/ftn2rfc
	${STRIP_CMD} ${PREFIX}/libexec/fidogate/ftnafmail
	${STRIP_CMD} ${PREFIX}/libexec/fidogate/ftnafpkt
	${STRIP_CMD} ${PREFIX}/libexec/fidogate/ftnflo
	${STRIP_CMD} ${PREFIX}/libexec/fidogate/ftnin
	${STRIP_CMD} ${PREFIX}/libexec/fidogate/ftninpost
	${STRIP_CMD} ${PREFIX}/libexec/fidogate/ftnmail
	${STRIP_CMD} ${PREFIX}/libexec/fidogate/ftnpack
	${STRIP_CMD} ${PREFIX}/libexec/fidogate/ftnroute
	${STRIP_CMD} ${PREFIX}/libexec/fidogate/ftntick
	${STRIP_CMD} ${PREFIX}/libexec/fidogate/ftntoss
	${STRIP_CMD} ${PREFIX}/libexec/fidogate/rfc2ftn

.include <bsd.port.mk>
