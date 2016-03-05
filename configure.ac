dnl $Id: configure.in,v 5.5 2008/06/29 07:20:39 anray Exp $
dnl
dnl Copyright (c) 2003-2008 Andrey Slusar <anrays@gmail.com>
dnl Copyright (c) 2003-2004 Alexandr Dobroslavskiy <dsas@users.sourceforge.net>
dnl Copyright (c) 2015-206 Yauheni Kaliuta <y.kaliuta@gmail.com>
dnl
AC_REVISION([$Revision: 5.6 $])
AC_PREREQ(2.53)
AC_INIT([fidogate],[5.2.5],[y.kaliuta@gmail.com])
AC_DISABLE_OPTION_CHECKING
AM_CONFIG_HEADER([config.h])
AC_CONFIG_SRCDIR([src/common/config.c])
AC_CONFIG_AUX_DIR([build])
AC_PREFIX_DEFAULT(/usr/local/fido)
AM_INIT_AUTOMAKE
AM_MAINTAINER_MODE
AC_CANONICAL_HOST

dnl Check for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB
AC_PROG_AWK
AC_PROG_YACC

dnl Checks for header files.
AC_FUNC_ALLOCA
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS(fcntl.h limits.h stddef.h sys/mount.h sys/param.h sys/statfs.h sys/stat.h)
AC_CHECK_HEADERS(sys/statvfs.h sys/time.h sys/vfs.h syslog.h utime.h time.h)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_MODE_T
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM
AC_STRUCT_TIMEZONE

dnl Checks for library functions.
AC_FUNC_CLOSEDIR_VOID
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_FUNC_MKTIME
dnl for future AC_FUNC_REALLOC
AC_FUNC_SETVBUF_REVERSED
AC_TYPE_SIGNAL
AC_FUNC_STAT
AC_FUNC_STRFTIME
AC_FUNC_UTIME_NULL
AC_FUNC_VPRINTF
AC_CHECK_FUNCS(gettimeofday memchr memset mkdir regcomp strcasecmp strchr)
AC_CHECK_FUNCS(strerror strncasecmp strpbrk strrchr strspn strstr utime)

dnl Redefine install scripts
AM_PROG_LIBTOOL
AC_SUBST(LIBTOOL_DEPS)
INSTALL_PROGRAM='${INSTALL} -g ${GROUP} -o ${OWNER} -m 755'
INSTALL_SCRIPT='${INSTALL} -g ${GROUP} -o ${OWNER} -m 755'
INSTALL_DATA='${INSTALL} -g ${GROUP} -o ${OWNER} -m 640'

dnl Paths to major progs
AC_HEADER_MAJOR
AC_PATH_PROG(PERL, perl, no)
AC_PATH_PROG(PERL5, perl5, $PERL)
AC_PATH_PROG(ZIP, zip, no)
AC_PATH_PROG(RAR, rar, no)
AC_PATH_PROG(UNZIP, unzip, no)
AC_PATH_PROG(ARC, arc, no)
AC_PATH_PROG(LHA, lzh, no)
AC_PATH_PROG(UUDECODE, uudecode, no)

dnl took from libgd
dnl if we're configuring on a system w/out gettext, don't fall over
m4_ifndef([AM_ICONV],[m4_define([AM_ICONV],[:])])
AM_ICONV

config_vars=
# Check for a --with-logdir argument
AC_ARG_WITH(logdir, dnl
  --with-logdir=DIR		set fidogate log DIR,
	    [dnl
case "$with_logdir" in
yes) AC_MSG_ERROR([--with-logdir requires an argument; use --with-logdir=DIR]) ;;
''|no) ;;
*) logdir="$withval" ;;
esac
])

# Check for a --with-vardir argument
AC_ARG_WITH(vardir, dnl
  --with-vardir=DIR		set fidogate var DIR,
	    [dnl
case "$with_vardir" in
yes) AC_MSG_ERROR([--with-vardir requires an argument; use --with-vardir=DIR]) ;;
''|no) ;;
*) vardir="$withval" ;;
esac
])

# Check for a --with-lockdir argument
AC_ARG_WITH(lockdir, dnl
  --with-lockdir=DIR		set fidogate lock DIR,
	    [dnl
case "$with_lockdir" in
yes) AC_MSG_ERROR([--with-lockdir requires an argument; use --with-lockdir=DIR]) ;;
''|no) ;;
*) LOCKDIR="$withval" ;;
esac
])

# Check for a --with-spooldir argument
AC_ARG_WITH(spooldir, dnl
  --with-spooldir=DIR		set fidogate spool DIR,
	    [dnl
case "$with_spooldir" in
yes) AC_MSG_ERROR([--with-spooldir requires an argument; use --with-spooldir=DIR]) ;;
''|no) ;;
*) spooldir="$withval" ;;
esac
])

# Check for a --with-sysconfdir argument
AC_ARG_WITH(sysconfdir, dnl
  --with-sysconfdir=DIR		set fidogate config DIR,
            [dnl
case "$with_sysconfdir" in
yes) AC_MSG_ERROR([--with-sysconfdir requires an argument; use --with-sysconfdir=DIR]) ;;
''|no) ;;
*) sysconfdir="$withval" ;;
esac 
])

# Check for a --with-btbasedir argument
AC_ARG_WITH(btbasedir, dnl
  --with-btbasedir=DIR		set fidogate BTBASE DIR,
	    [dnl
case "$with_btbasedir" in
yes) AC_MSG_ERROR([--with-btbasedir requires an argument; use --with-btbasedir=DIR]) ;;
''|no);;
*) btbasedir="$withval"
;;
esac
])
if ! test "${btbasedir+set}" = set; then
    btbasedir='/var/spool/fido/bt'
fi
AC_SUBST(btbasedir)

# Check for a --with-netmaildir argument
AC_ARG_WITH(netmaildir, dnl
  --with-netmaildir=DIR		set fidogate NETMAIL DIR,
	    [dnl
case "$with_netmaildir" in
yes) AC_MSG_ERROR([--with-netmaildir requires an argument; use --with-netmaildir=DIR]) ;;
''|no);;
*) netmaildir="$withval"
;;
esac
])
if ! test "${netmaildir+set}" = set; then
    netmaildir="${btbasedir}/netmail"
fi
AC_SUBST(netmaildir)

dnl# Check for a --with-newsbindir argument
AC_ARG_WITH(newsbindir, dnl
  --with-newsbindir=DIR		set INN bin directory,
	    [dnl
case "$with_newsbindir" in
yes) AC_MSG_ERROR([--with-newsbindir requires an argument; use --with-newsbindir=DIR]) ;;
''|no) ;;
*) newsbindir="$withval" ;;
esac
])

dnl# Check for a --with-inews argument
AC_ARG_WITH(INEWS_S, dnl
  --with-inews=PATH		set directory containt INEWS,
	    [dnl
case "$with_inews" in
yes) AC_MSG_ERROR([--with-inews   requires an argument; use --with-inews=PATH]) ;;
''|no) ;;
*) INEWS_S="$withval" ;;
esac
])

dnl# Check for a --with-rnews argument
AC_ARG_WITH(RNEWS_S, dnl
  --with-rnews=PATH		set directory containt RNEWS,
	    [dnl
case "$with_rnews" in
yes) AC_MSG_ERROR([--with-rnews   requires an argument; use --with-rnews=PATH]) ;;
''|no) ;;
*) RNEWS_S="$withval" ;;
esac
])

AC_ARG_WITH(, dnl
  --without-news          build fidogate without news server,,)
AC_ARG_WITH(, dnl
  --without-rsf           build fidogate without runinc send_fidogate function,,)
  

AC_ARG_WITH(sendmail, dnl
  --with-sendmail=PATH    Specify path to sendmail,
            [dnl
case "$with_sendmail" in
yes|''|no) AC_MSG_ERROR([--with-sendmail   requires an argument; use --with-sendmail=PATH]) ;;
*) SENDMAIL="$withval" ;;
esac
])


OWNER="news"
SRFC2FTN="rfc2ftn"

AC_ARG_WITH(owner, dnl
  --with-owner=USER   	  set possibility to run FIDOGATE from a USER,
            [dnl
case "$with_owner" in
''|news|no)
    AM_CONDITIONAL(ftnnews,false);;
*) 
    OWNER="$withval" 
    AM_CONDITIONAL(ftnnews,true)
    SRFC2FTN="ftnnews";;
esac
])
AC_SUBST(OWNER)

AC_SUBST(SRFC2FTN)

GROUP="news"
AC_ARG_WITH(group, dnl
  --with-group=GROUP   	  set possibility to run FIDOGATE from a GROUP,
            [dnl
case "$with_group" in
''|news) ;;
*) GROUP="$withval" ;;
esac
])
AC_SUBST(GROUP)

AC_ARG_WITH(group, dnl
  --with-libname=GROUP 	  library file name,
            [dnl
case "$with_group" in
''|news) ;;
*) group="$withval" ;;
esac
])

AC_ARG_ENABLE(local-ftn-addr,
[  --enable-local-ftn-addr use local FTN addres],
	      AC_DEFINE(LOCAL_FTN_ADDRESSES),)

AC_ARG_ENABLE(dobsy-files,
[  --disable-dobsy-files   create Binkley-style BSY],
    ,AC_DEFINE(DO_BSY_FILES))

AC_ARG_ENABLE(nfs-safe-lock,
[  --enable-nfs-safe-lock  create lock files/BSY files in an NFS-safe way],
    AC_DEFINE(NFS_SAFE_LOCK_FILES),)

AC_ARG_WITH(maxmsg-size,
[  --with-maxmsg-size=SIZE set max. message size for FIDO [14000]],
    AC_DEFINE_UNQUOTED(MAXMSGSIZE,$withval),AC_DEFINE_UNQUOTED(MAXMSGSIZE, 14000))

AC_ARG_WITH(ftn-invalid-domain,
[  --with-ftn-invalid-domain=DOM  set domain for invalid FTN addresses [INVALID_FTN_ADDRESS]],
    AC_DEFINE_UNQUOTED(FTN_INVALID_DOMAIN,"$withval"),AC_DEFINE_UNQUOTED(FTN_INVALID_DOMAIN, "INVALID_FTN_ADDRESS"))

AC_ARG_WITH(syslog-facility,
[  --with-syslog-facility=FAC	set syslog facility [LOG_LOCAL0]],
    AC_DEFINE_UNQUOTED(FACILITY,$withval),AC_DEFINE_UNQUOTED(FACILITY, LOG_LOCAL0))

AC_ARG_WITH(ftn-charset,
[  --with-ftn-charset=CHS  set default assumed charset for Fido messages [cp866] ],
    AC_DEFINE_UNQUOTED(CHARSET_STDFTN,"$withval"),AC_DEFINE_UNQUOTED(CHARSET_STDFTN, "cp866"))

AC_ARG_WITH(rfc-charset,
[  --with-rfc-charset=CHS  set default assumed charset for RFC messages if without MIME headers [koi8-r]],
    AC_DEFINE_UNQUOTED(CHARSET_STDRFC,"$withval"),AC_DEFINE_UNQUOTED(CHARSET_STDRFC, "koi8-r"))

AC_ARG_WITH(7bit-charset,
[  --with-7bit-charset=CHS set default charset for RFC messages with forced 7bit encoding [us-ascii]],
    AC_DEFINE_UNQUOTED(CHARSET_STD7BIT,"$withval"),AC_DEFINE_UNQUOTED(CHARSET_STD7BIT, "us-ascii"))

AC_ARG_WITH(afix-maxstr,
[  --with-afix-maxstr=AFS set maximum strings in areafix reports [150] ],
    AC_DEFINE_UNQUOTED(AREAFIXMAXSTR,$withval),AC_DEFINE_UNQUOTED(AREAFIXMAXSTR, 150))

AC_ARG_ENABLE(aliases-local,
[  --enable-aliases-local  rewrite addresses found in ALIASES so that the sender's address is the gateway address],
    AC_DEFINE(ALIASES_ARE_LOCAL),)dnl'

AC_ARG_ENABLE(ai1,
[  --enable-ai1		Add -a option to HOSTS entries, useful only with PASSTHRU NET/ECHOMAIL],
    AC_DEFINE(AI_1),)

AC_ARG_ENABLE(best-aka,
[  --disable-best-aka	disable best AKA select],
    ,AC_DEFINE(BEST_AKA))

AC_ARG_ENABLE(ftnacl,
[  --disable-ftnacl        do'nt use ACL for FTN links ],
    ,AC_DEFINE(FTN_ACL))

AC_ARG_WITH(desc-dir,
[  --with-desc-dir=DIR     put file descriptions into ".desc/%s.desc" file instead of files.bbs],
    DESC_DIR="$withval",DESC_DIR=".desc")

AC_ARG_ENABLE(desc-dir,
[  --disable-desc-dir      put file descriptions into files.bbs],
    ,AC_DEFINE_UNQUOTED(DESC_DIR,"$DESC_DIR"))

AC_ARG_ENABLE(recode-file-desc,
[  --disable-recode-file-desc disable recode file descriptions],
    ,AC_DEFINE(RECODE_FILE_DESC))

AC_ARG_ENABLE(af-avail,
[  --disable-af-avail      change behaviour of areafix command "%list" and "%avail" ],
    ,AC_DEFINE(AF_AVAIL))

AC_ARG_ENABLE(xff-echomail,
[  --disable-xff-echomail  add X-FTN-From header to gated echomail],
    ,AC_DEFINE(X_FTN_FROM_ECHOMAIL))

AC_ARG_ENABLE(spyes,
[  --disable-spyes         use SPYES config],
    ,AC_DEFINE(SPYES))

AC_ARG_ENABLE(aflr,
[  --disable-aflr          restricted access to areafix command %listall],
    ,AC_DEFINE(AF_LISTALL_RESTRICTED))

AC_ARG_ENABLE(fecho-pass,
[  --disable-fecho-pass    use passthrought filearea],
    ,AC_DEFINE(FECHO_PASSTHROUGHT))

AC_ARG_ENABLE(use-filebox,
[  --disable-use-filebox   use fileboxes for fileecho],
    ,AC_DEFINE(USE_FILEBOX))

AC_ARG_ENABLE(tick-crc,
[  --disable-tick-crc      ftntick CRC control],
    AC_DEFINE(FTNTICK_NOCRC),)

AC_ARG_ENABLE(no-organization,
[  --enable-no-organization enable if no insert Origin line if it absence],
    ,AC_DEFINE(NOINSERT_ORGANIZATION),)

AC_ARG_ENABLE(insecure-not-db,
[  --disable-insecure-not-db put insecure mail into dupe database],
    ,AC_DEFINE(INSECURE_DONT_PUT_INTO_DUPE_DB))

AC_ARG_ENABLE(af-ansok,
[  --enable-af-ansok       change areafix answering to "ok" if subscribing],
    AC_DEFINE(ANSWER_OK), )

AC_ARG_ENABLE(fs-msgid,
[  --disable-fs-msgid      set gate msgid into outgoing messages],
    ,AC_DEFINE(FIDO_STYLE_MSGID))

AC_ARG_ENABLE(dbc-history,
[  --enable-dbc-history    use msgid correspond data base],
    AC_DEFINE(DBC_HISTORY), )

AC_ARG_ENABLE(afses,
[  --enable-afses          areafix send echo status],
    AC_DEFINE(AFSEND_ECHO_STATUS),)

AC_ARG_ENABLE(sub_zonegate,
[  --enable-sub-zonegate   subscribe only zonegate],
    AC_DEFINE(SUBSCRIBE_ZONEGATE),)

AC_ARG_ENABLE(pid2rd-tid2gtv,
[  --enable-pid2rd-tid2gtv paste ^PID & ^TID kludges to message],
    AC_DEFINE(PID_READER_TID_GTV),)

AC_ARG_ENABLE(tick-history,
[  --disable-tick-history  tick dupe check],
    ,AC_DEFINE(TIC_HISTORY))

AC_ARG_ENABLE(dnt-netmail,
[  --enable-dnt-netmail    do not toss netmail],
    AC_DEFINE(DO_NOT_TOSS_NETMAIL), )

AC_ARG_ENABLE(sep-pkt,
[  --enable-sep-pkt        separate message to individual pkt (rfc->ftn)],
    AC_DEFINE(SEP_RFC2FTN_PKT), )

AC_ARG_ENABLE(owfl,
[  --disable-owfl          overwrite fileecho link],
    ,AC_DEFINE(OVERWRITEN_FECHO_FILE_TO_LINK))

AC_ARG_ENABLE(active-lookup,
[  --disable-active-lookup active file lookup],
    ,AC_DEFINE(ACTIVE_LOOKUP))

AC_ARG_ENABLE(recode-allrfc,
[  --disable-recode-allrfc recoding some RFC header],
    ,AC_DEFINE(RECODE_ALL_RFC))

AC_SEARCH_LIBS(regcomp, rx regex, [
    AC_DEFINE(HAS_POSIX_REGEX,[],[Do we have a decent regex library?])
    AC_CHECK_HEADER(rxposix.h, AC_DEFINE(HAVE_RX,[],[Do we have rxposix.h?]))])

if test "$with_news" != no ;then
{
    AC_MSG_CHECKING(for news-library path)
    if ! test -x "${newsbindir}"; then
	if test -x /usr/local/news/bin/innconfval ; then
	    newsbindir=/usr/local/news/bin
	elif test -x /usr/lib/innconfval ; then
	    newsbindir=/usr/lib
	elif test -x /usr/lib/news/innconfval ; then
	    newsbindir=/usr/lib/news
	elif test -x /usr/lib/news/bin/innconfval ; then
	    newsbindir=/usr/lib/news/bin
	elif test -x /usr/local/lib/news/bin/innconfval ; then
	    newsbindir=/usr/local/lib/news/bin
	elif test -x /usr/local/news/innconfval ; then
	    newsbindir=/usr/local/news
	elif test -x /usr/bin/innconfval ; then
	    newsbindir=/usr/bin
	else
AC_MSG_ERROR([PLEASE INSTALL INN OR SET PATH TO innconfval DIR; use --with-newsbindir=DIR or --without-news])
	    exit 1;
	fi
    fi
    if $newsbindir/innconfval -v 2>&1 </dev/null | egrep 2\.[3\|4\|5\|6] > /dev/null; then
      eval `$newsbindir/innconfval -s|grep -e 'PATHNEWS\|PATHBIN\|PATHOUTGOING\|PATHDB\|PATHARTICLES\|PATHETC'`
      NEWSLIBDIR=${PATHNEWS}
    else
      NEWSLIBDIR="$newsbindir"
    fi
    AC_MSG_RESULT($NEWSLIBDIR)

    if test "x${PATHOUTGOING}" = "x"; then
	AC_MSG_ERROR([use --with-newsbindir=DIR or --without-news option])
    fi

    XPATH="$PATHBIN:/usr/local/news/bin:/usr/lib/news:/usr/lib/news/bin:/usr/local/lib/news/bin:usr/bin"

    AC_MSG_CHECKING(for rnews)
    if test "${with_rnews+set}" = set ;then
	RNEWS="$with_rnews"
    else
	AC_PATH_PROG(RNEWS, rnews,, $XPATH)
	if test "x$RNEWS" = "x"; then
	    AC_MSG_ERROR([rnews not found, re-run with valid --with-rnews=DIR])
	fi
    fi

    AC_MSG_CHECKING(for inews)
    if test "${with_inews+set}" = set ;then
	INEWS="$with_inews"
    else
	AC_PATH_PROG(INEWS, inews,, $XPATH)
	if test "x$INEWS" = "x"; then
	    AC_MSG_ERROR([inews not found, re-run with valid --with-inews=DIR])
	fi
    fi

    if $NEWSLIBDIR/bin/innconfval -v 2>&1 </dev/null | egrep 2\.\[3\|4\|5\|6] > /dev/null; then
	AC_DEFINE(OLD_BATCHER)
	BATCHER='time $PATHBIN/batcher -N $QUEUEJOBS -b500000 -p"$RFC2FTN -b -n" $SITE $BATCHFILE'
    else
	BATCHER='time $RFC2FTN -f $BATCHFILE -m 500'
    fi

    if test "$with_news" != no ;then
	if test "$with_owner" = news; then
	    AC_DEFINE(USE_RUNINC_SFGT)
	fi
    fi
}
else
    BATCHER='time $RFC2FTN -f $BATCHFILE -m 500'
    RNEWS='/none/rnews'
    INEWS='/none/inews'
    NEWSLIBDIR='/none/news-libdir'
    PATHOUTGOING='/none/news-pathoutgoing'
    PATHARTICLES='/none/news-patharticles'
    PATHNEWS='/none/news-pathnews'
    PATHBIN='/none/news-pathbin'
    PATHETC='/none/news-pathetc'
    PATHDB='/none/news-pathdb'
fi

AC_SUBST(BATCHER)
AC_SUBST(RNEWS)
AC_SUBST(NEWSLIBDIR)
AC_SUBST(PATHOUTGOING)
AC_SUBST(PATHARTICLES)
AC_SUBST(PATHNEWS)
AC_SUBST(PATHBIN)
AC_SUBST(PATHETC)
AC_SUBST(PATHDB)
AC_SUBST(INEWS)

# Checking sendmail
AC_MSG_CHECKING(for sendmail)
if test "${with_sendmail+set}" = set ;then
	SENDMAIL="$with_sendmail"
else
    XPATH="/usr/sbin:/usr/lib:/usr/ucblib:/usr/share"
    AC_PATH_PROG(SENDMAIL, sendmail,, $XPATH)
    if test "x$SENDMAIL" = "x"; then
	AC_MSG_ERROR([sendmail not found, re-run with valid --with-sendmail=DIR])
    fi
fi
AC_SUBST(SENDMAIL)

AC_MSG_CHECKING(for lock directory)
if test "${with_lockdir+set}" = set; then
	LOCKDIR="$with_lockdir"
elif test -d /var/run; then
	LOCKDIR=/var/run/fidogate
elif test -d /var/spool/lock; then
	LOCKDIR=/var/spool/lock/fidogate
else
	LOCKDIR=/var/lock/fidogate
fi
AC_MSG_RESULT($LOCKDIR)
AC_SUBST(LOCKDIR)

AC_MSG_CHECKING(for var directory)
if test "${with_vardir+set}" = set; then
	vardir="$with_vardir"
elif test -d /var/lib; then
	vardir=/var/lib/fidogate
elif test -d /var/data; then
	vardir=/var/data/fidogate
elif test -d /var/db; then
	vardir=/var/db/fidogate
else
	vardir=/var/lib/fidogate
fi
AC_MSG_RESULT(${vardir})
AC_SUBST(vardir)

AC_MSG_CHECKING(for log directory)
if test "${with_logdir+set}" = set; then
	LOGDIR="$with_logdir"
elif test -d /var/log; then
	LOGDIR=/var/log/fido/gate
elif test -d /var/spool/log; then
	LOGDIR=/var/spool/log/fidogate
else
	VARDIR=/var/log/fido/gate
fi
AC_MSG_RESULT($LOGDIR)
AC_SUBST(LOGDIR)

AC_MSG_CHECKING(for spool directory)
if test "${with_spooldir+set}" = set; then
	SPOOLDIR="$with_spooldir"
elif test -d /var/spool; then
	SPOOLDIR=/var/spool/fido/gate
elif test -d /var/data; then
	SPOOLDIR=/var/data/fido/gate
elif test -d /var/db; then
	SPOOLDIR=/var/db/fido/gate
else
	SPOOLDIR=/var/data/fido/gate
fi
AC_MSG_RESULT($SPOOLDIR)
AC_SUBST(SPOOLDIR)

SUBSTCMD='$(PERL) $(top_srcdir)/build/subst.pl -c$(top_srcdir)/build/config.make -t$(top_srcdir)'
M4MAKE='$(M4) $(M4OPTIONS)'

AC_SUBST(SUBSTCMD)
AC_SUBST(M4MAKE)
AC_DEFINE_UNQUOTED(GIT_HASH_STR,
	["`cd $srcdir && git log -1 --format=format:%h`"],
	[A string containing the git hash])

HOSTNAME=`hostname -s`
DOMAINNAME=`hostname|sed -e "s/$HOSTNAME//"`
AC_SUBST(HOSTNAME)
AC_SUBST(DOMAINNAME)

AC_CONFIG_COMMANDS([build_headers],[build/subst.pl -h])

AC_CONFIG_FILES([build/config.make \
                 Makefile \
		 src/Makefile \
		 src/areafix/Makefile \
		 src/charset/Makefile \
		 src/common/Makefile \
		 src/gate/Makefile \
		 src/include/Makefile \
		 src/tick/Makefile \
		 src/toss/Makefile \
		 src/util/Makefile \
		 scripts/Makefile \
		 scripts/areas/Makefile \
		 scripts/maintenance/Makefile \
		 scripts/misc/Makefile \
		 scripts/nodelist/Makefile \
		 scripts/outbound/Makefile \
		 doc/Makefile \
		 tests/Makefile \
		 tests/common/Makefile \
		 tests/areafix/Makefile \
		 tests/charset/Makefile \
		 tests/gate/Makefile \
		 tests/tick/Makefile \
		 tests/toss/Makefile \
		 tests/utils/Makefile ])
AC_OUTPUT
