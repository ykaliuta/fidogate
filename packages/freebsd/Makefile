# New ports collection makefile for: fidogate5
# Whom:                 Andrey Slusar <rusfidogate@users.sourceforge.net>
# Date created:         29 Aug 2003
# $FreeBSD$
#

PORTNAME=		fidogate
PORTVERSION=		5.1.0
PORTREVISION=		ds
CATEGORIES=		news mail
MASTER_SITES=		http://node126.narod.ru/files/
DISTNAME=		fidogate5.1.0ds-rc1

MAINTAINER=		rusfidogate@users.sourceforge.net
COMMENT=		Fido-Internet Gateway and Tosser

USE_BZIP2=      	yes
GNU_CONFIGURE=  	yes
USE_PERL5=      	yes

INSTALL_SHLIB=  	yes

SPOOLDIR?=		/var/spool/fido/gate
BTBASEDIR?=		/var/spool/fido/bt
LOCKDIR?=		/var/run/fidogate
LOGDIR?=		/var/log/fido/gate
VARDIR?=		${PREFIX}/var/fidogate

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
	${MKDIR} ${VARDIR}/seq
	${CHOWN} news:news ${VARDIR}
	${CHOWN} news:news ${VARDIR}/seq
	${CHOWN} news:news ${PREFIX}/etc/fido/gate
	${CHOWN} news:news ${PREFIX}/lib/fidogate
	${CHOWN} news:news ${PREFIX}/libexec/fidogate

.if !defined(NOPORTDOCS)
	${MKDIR} ${PREFIX}/share/doc/fidogate5
	${INSTALL_DATA} ${WRKSRC}/doc/README.ru ${PREFIX}/share/doc/fidogate5/README.ru
	${INSTALL_DATA} ${WRKSRC}/doc/README ${PREFIX}/share/doc/fidogate5/README
	${INSTALL_DATA} ${WRKSRC}/ChangeLog ${PREFIX}/share/doc/fidogate5/ChangeLog
	${MKDIR} ${PREFIX}/share/doc/fidogate5/examples
	${MKDIR} ${PREFIX}/share/examples/fidogate5/dyff
	${INSTALL_DATA} ${WRKSRC}/doc/examples/dyff/acl.sample ${PREFIX}/share/examples/fidogate5/dyff/acl.sample
	${INSTALL_DATA} ${WRKSRC}/doc/examples/dyff/aliases.sample ${PREFIX}/share/examples/fidogate5/dyff/aliases.sample
	${INSTALL_DATA} ${WRKSRC}/doc/examples/dyff/areafix.help.rus ${PREFIX}/share/examples/fidogate5/dyff/areafix.help.rus
	${INSTALL_DATA} ${WRKSRC}/doc/examples/dyff/areafix.help.sample ${PREFIX}/share/examples/fidogate5/dyff/areafix.help.sample
	${INSTALL_DATA} ${WRKSRC}/doc/examples/dyff/areas.bbs.sample ${PREFIX}/share/examples/fidogate5/dyff/areas.bbs.sample
	${INSTALL_DATA} ${WRKSRC}/doc/examples/dyff/areas.sample ${PREFIX}/share/examples/fidogate5/dyff/areas.sample
	${INSTALL_DATA} ${WRKSRC}/doc/examples/dyff/fareas.bbs.sample ${PREFIX}/share/examples/fidogate5/dyff/fareas.bbs.sample
	${INSTALL_DATA} ${WRKSRC}/doc/examples/dyff/fidogate.conf.sample ${PREFIX}/share/examples/fidogate5/dyff/fidogate.conf.sample
	${INSTALL_DATA} ${WRKSRC}/doc/examples/dyff/fidokill.sample ${PREFIX}/share/examples/fidogate5/dyff/fidokill.sample
	${INSTALL_DATA} ${WRKSRC}/doc/examples/dyff/ftnacl.sample ${PREFIX}/share/examples/fidogate5/dyff/ftnacl.sample
	${INSTALL_DATA} ${WRKSRC}/doc/examples/dyff/hosts.sample ${PREFIX}/share/examples/fidogate5/dyff/hosts.sample
	${INSTALL_DATA} ${WRKSRC}/doc/examples/dyff/packing.sample ${PREFIX}/share/examples/fidogate5/dyff/packing.sample
	${INSTALL_DATA} ${WRKSRC}/doc/examples/dyff/passwd.sample ${PREFIX}/share/examples/fidogate5/dyff/passwd.sample
	${INSTALL_DATA} ${WRKSRC}/doc/examples/dyff/routing.sample ${PREFIX}/share/examples/fidogate5/dyff/routing.sample
	${INSTALL_DATA} ${WRKSRC}/doc/examples/dyff/spyes.sample ${PREFIX}/share/examples/fidogate5/dyff/spyes.sample
	${INSTALL_DATA} ${WRKSRC}/doc/examples/dyff/uplinks.sample ${PREFIX}/share/examples/fidogate5/dyff/uplinks.sample
	${MKDIR} ${PREFIX}/share/examples/fidogate5/point
	${INSTALL_DATA} ${WRKSRC}/doc/examples/point/areas ${PREFIX}/share/examples/fidogate5/point/areas
	${INSTALL_DATA} ${WRKSRC}/doc/examples/point/areas.bbs ${PREFIX}/share/examples/fidogate5/point/areas.bbs
	${INSTALL_DATA} ${WRKSRC}/doc/examples/point/fidogate.conf ${PREFIX}/share/examples/fidogate5/point/fidogate.conf
	${INSTALL_DATA} ${WRKSRC}/doc/examples/point/newsfeeds ${PREFIX}/share/examples/fidogate5/point/newsfeeds
	${INSTALL_DATA} ${WRKSRC}/doc/examples/point/packing ${PREFIX}/share/examples/fidogate5/point/packing
	${INSTALL_DATA} ${WRKSRC}/doc/examples/point/routing ${PREFIX}/share/examples/fidogate5/point/routing
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
