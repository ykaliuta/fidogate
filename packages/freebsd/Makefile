# New ports collection makefile for:    fidogate
# Date created:         27 Sep 2002
# Whom:                 dyff
#
# $FreeBSD: ports/news/inn/Makefile,v 1.52 2002/08/01 07:35:52 ijliao Exp $
#

PORTNAME=	fidogate
PORTVERSION=	5.0.0b7
CATEGORIES=	russian news mail
MASTER_SITES=	http://www.fidogate.spb.ru/download/ \
		${MASTER_SITE_SUNSITE}

MAINTAINER=	dyff@fidogate.spb.ru

USE_BZIP2=	yes
GNU_CONFIGURE=	yes
USE_GMAKE=	yes
USE_AUTOCONF=	yes
USE_PERL5=	yes

INSTALL_SHLIB=	yes

CONFIGURE_ARGS=	\
		--enable-amiga-out \
		--disable-desc-dir \
		--prefix=/

DOCLIST=	\
		README

do-install:
	(cd ${WRKSRC}; ${GMAKE} -f ${MAKEFILE} ${MAKE_ARGS} install)
	(cd ${WRKSRC}; ${GMAKE} -f ${MAKEFILE} ${MAKE_ARGS} install-config)

pre-install:
	(cd ${WRKSRC}; ${GMAKE} -f ${MAKEFILE} ${MAKE_ARGS} install-dirs)

post-install:
	strip ${PREFIX}/bin/ftnafutil
	strip ${PREFIX}/bin/pkttmpl
	strip ${PREFIX}/bin/runinc
	strip ${PREFIX}/bin/ngoper
	strip ${PREFIX}/bin/ftnfattach
	strip ${PREFIX}/bin/sumcrc
	strip ${PREFIX}/bin/ftnoutpkt
	strip ${PREFIX}/bin/pktdebug
	strip ${PREFIX}/libexec/ftnaf
	strip ${PREFIX}/libexec/ftnafpkt
	strip ${PREFIX}/libexec/ftnafmail
	strip ${PREFIX}/libexec/ftnmail
	strip ${PREFIX}/libexec/rfc2ftn
	strip ${PREFIX}/libexec/ftn2rfc
	strip ${PREFIX}/libexec/ftnin
	strip ${PREFIX}/libexec/ftntick
	strip ${PREFIX}/libexec/ftnhatch
	strip ${PREFIX}/libexec/ftn2ftn
	strip ${PREFIX}/libexec/ftntoss
	strip ${PREFIX}/libexec/ftnroute
	strip ${PREFIX}/libexec/ftnpack
	strip ${PREFIX}/libexec/ftnexpire
	strip ${PREFIX}/libexec/ftnflo

.include <bsd.port.mk>
