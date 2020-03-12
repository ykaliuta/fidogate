dnl Copyright (c) 2003-2008 Andrey Slusar <anrays@gmail.com>
dnl Copyright (c) 2003-2004 Alexandr Dobroslavskiy <dsas@users.sourceforge.net>
dnl Copyright (c) 2015-2020 Yauheni Kaliuta <y.kaliuta@gmail.com>
dnl
AC_REVISION([Revision: 1033])
AC_PREREQ(2.53)
AC_INIT([fidogate],[5.8],[y.kaliuta@gmail.com])
AC_DISABLE_OPTION_CHECKING
AM_CONFIG_HEADER([config.h])
AC_CONFIG_SRCDIR([src/common/config.c])
AC_CONFIG_AUX_DIR([build])
AC_PREFIX_DEFAULT(/usr/local/fido)
AM_INIT_AUTOMAKE([1.11 foreign subdir-objects])
AM_SILENT_RULES([yes])
AM_MAINTAINER_MODE
AC_CANONICAL_HOST

dnl Check for programs.
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_AWK
AC_PROG_YACC

if test "x$YACC" = "x"; then
   AC_MSG_ERROR([yacc/bison not found, please install])
fi

# for libtool:
# LT_INIT
# AC_SUBST(LIBTOOL_DEPS)
AC_PROG_RANLIB
AM_PROG_AR

dnl Checks for header files.
AC_FUNC_ALLOCA
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS(fcntl.h limits.h stddef.h sys/mount.h sys/param.h sys/statfs.h sys/stat.h)
AC_CHECK_HEADERS(sys/statvfs.h sys/time.h sys/vfs.h syslog.h utime.h time.h)
AC_CHECK_HEADERS(sysexits.h malloc.h)

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
AC_CHECK_FUNCS(strerror strncasecmp strpbrk strrchr strspn strstr stricmp utime)
AC_CHECK_FUNCS(link mktime syslog sprintf snprintf)

if ! test "x$ac_cv_func_snprintf" = xyes && \
   ! test "x$ac_cv_func_sprintf" = xyes; then
   AC_MSG_ERROR(sprintf() or snprintf() required)
fi

dnl Redefine install scripts
INSTALL_DATA='${INSTALL} -m 640'

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

AM_ICONV

AS_IF([test x$am_func_iconv != xyes],
       AC_MSG_ERROR([working iconv not found]))

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
    btbasedir="${localstatedir}/spool/fido/bt"
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
  --with-newsbindir=DIR		set INN bin directory; PATHBIN,
	    [dnl
case "$with_newsbindir" in
yes) AC_MSG_ERROR([--with-newsbindir requires an argument; use --with-newsbindir=DIR]) ;;
''|no) ;;
*) newsbindir="$withval" ;;
esac
])

AC_ARG_WITH([news],
    [AS_HELP_STRING([--without-news], [build fidogate without news server])])

AC_ARG_WITH([inndir],
    [AS_HELP_STRING([--with-inndir], [main INN directory; PATHNEWS])],
    [],
    [with_inndir=undefined])

AC_ARG_WITH([innetcdir],
    [AS_HELP_STRING([--with-innetcdir], [INN etc directory; PATHETC])],
    [],
    [with_innetcdir=undefined])

AC_ARG_WITH([inndbdir],
    [AS_HELP_STRING([--with-inndbdir], [INN db directory; PATHDB])],
    [],
    [with_inndbdir=undefined])

AC_ARG_WITH([innspooldir],
    [AS_HELP_STRING([--with-innspooldir], [INN spool directory])],
    [],
    [with_innspooldir=undefined])

AC_ARG_ENABLE([oldbatcher],
    [AS_HELP_STRING([--inn-disable-old-batcher], [disable old INN batcher])])

AC_ARG_WITH([rsf],
    [AS_HELP_STRING([--without-rsf],
                   [build fidogate without runinc send_fidogate function])])

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
    withftnnews=false;;
*) 
    OWNER="$withval"
    withftnnews=true
    SRFC2FTN="ftnnews";;
esac
])

AM_CONDITIONAL(ftnnews, [test x$withftnnews = xtrue])


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

AC_ARG_WITH(default-mailer,dnl
  [--with-default-mailer=COMMAND default mailer comand],dnl
  [ if test -n "$withval"; then
    AC_DEFINE_UNQUOTED(DEFAULT_MAILER, "$withval",
      [default mailer command])
    fi
  ])

AC_ARG_ENABLE(local-ftn-addr,
[  --enable-local-ftn-addr use local FTN addres],dnl
   [case "${enableval}" in
    yes) enable_local_ftn_addr=yes ;;
     no) enable_local_ftn_addr=no ;;
      *) AC_MSG_ERROR(bad value ${enableval} for --enable-local-ftn-addr) ;;
    esac],dnl
    enable_local_ftn_addr=no)

if test $enable_local_ftn_addr = yes; then
    AC_DEFINE(LOCAL_FTN_ADDRESSES,
              1,
	      [
 Generate local FTN addresses, e.g.
      user_name%p.f.n.z@host.domain
 instead of
      user_name@p.f.n.z.domain
])
fi

AC_ARG_ENABLE(dobsy-files,
[  --disable-dobsy-files   create Binkley-style BSY],dnl
   [case "${enableval}" in
    yes | no) enable_dobsy_files="${enableval}" ;;
      *) AC_MSG_ERROR(bad value ${enableval} for --disable-dobsy-files) ;;
    esac],dnl
    enable_dobsy_files=yes)

if test $enable_dobsy_files = yes; then
    AC_DEFINE(DO_BSY_FILES,
              1,
	      [Create Binkley-style BSY files for all outbound operations])
fi

AC_ARG_ENABLE(nfs-safe-lock,
[  --enable-nfs-safe-lock  create lock files/BSY files in an NFS-safe way],dnl
   [case "${enableval}" in
    yes | no) enable_nfs_safe_lock="${enableval}" ;;
      *) AC_MSG_ERROR(bad value ${enableval} for --enable-nfs-safe-lock) ;;
    esac],dnl
    enable_nfs_safe_lock=no)

if test $enable_nfs_safe_lock = yes; then
    AC_DEFINE(NFS_SAFE_LOCK_FILES,
              1,
	      [Create lock files / BSY files in an NFS-safe way (see man 2 open)])
fi

MAXMSGSIZE=14000
AC_ARG_WITH(maxmsg-size,
[  --with-maxmsg-size=SIZE set max. message size for FIDO [14000]],
    MAXMSGSIZE=$withval,)
AC_DEFINE_UNQUOTED(MAXMSGSIZE,
	$MAXMSGSIZE,
	[
Default max. message size for FIDO. Due to some more brain damage
in FIDONET programs we have to split larger messages into several
smaller ones. May be set with the -M option in AREAS or MaxMsgSize
in DEFAULT_CONFIG_GATE.

< 16 K -- 14000
< 32 K -- 30000
])

FTN_INVALID_DOMAIN="INVALID_FTN_ADDRESS"
AC_ARG_WITH(ftn-invalid-domain,
[  --with-ftn-invalid-domain=DOM  set domain for invalid FTN addresses [INVALID_FTN_ADDRESS]],
   FTN_INVALID_DOMAIN="$withval",)
AC_DEFINE_UNQUOTED(FTN_INVALID_DOMAIN,"$FTN_INVALID_DOMAIN",
		 [Domain for invalid FTN addresses])

SYSLOG_FACILITY=LOG_LOCAL0
AC_ARG_WITH(syslog-facility,
[  --with-syslog-facility=FAC	set syslog facility [LOG_LOCAL0]],
    SYSLOG_FACILITY=$withval,)
AC_DEFINE_UNQUOTED(FACILITY, $SYSLOG_FACILITY,
[
 syslog facility used for logging if logfile == "syslog"
 (only for HAS_SYSLOG defined)
])

CHARSET_STDFTN="cp866"
CHARSET_STDRFC="utf-8"
CHARSET_STD7BIT="us-ascii"
AC_ARG_WITH(ftn-charset,
[  --with-ftn-charset=CHS  set default assumed charset for Fido messages [cp866] ],
   CHARSET_STDFTN="$withval",)
AC_DEFINE_UNQUOTED(CHARSET_STDFTN, "$CHARSET_STDFTN",
[Default assumed charset for Fido messages (see also DefaultCharset)])

AC_ARG_WITH(rfc-charset,
[  --with-rfc-charset=CHS  set default assumed charset for RFC messages if without MIME headers [koi8-r]],
   CHARSET_STDRFC="$withval",)
AC_DEFINE_UNQUOTED(CHARSET_STDRFC, "$CHARSET_STDRFC",
[Default assumed charset for RFC messages if without MIME headers])

AC_ARG_WITH(7bit-charset,
[  --with-7bit-charset=CHS set default charset for RFC messages with forced 7bit encoding [us-ascii]],
   CHARSET_STD7BIT="$withval",)
AC_DEFINE_UNQUOTED(CHARSET_STD7BIT, "$CHARSET_STD7BIT",
[Default charset for RFC messages with forced 7bit encoding])

AREAFIXMAXSTR=150
AC_ARG_WITH(afix-maxstr,
[  --with-afix-maxstr=AFS set maximum strings in areafix reports [150] ],AREAFIXMAXSTR=$withval,)
AC_DEFINE_UNQUOTED(AREAFIXMAXSTR,$AREAFIXMAXSTR,
[AFS set maximum strings in areafix reports])

AC_ARG_ENABLE(aliases-local,
[  --enable-aliases-local  rewrite addresses found in ALIASES so that the sender's address is the gateway address],dnl
   [case "${enableval}" in
    yes | no) enable_aliases_local="${enableval}" ;;
      *) AC_MSG_ERROR(bad value ${enableval} for --enable-aliases-local) ;;
    esac],dnl
    enable_aliases_local=no)

if test $enable_aliases_local = yes; then
    AC_DEFINE(ALIASES_ARE_LOCAL,
              1,
	      [
 Rewrite addresses found in ALIASES so that the sender's address is the
 gateway address. The reverse direction requires suitable MTA aliases.
])
fi

AC_ARG_ENABLE(ai1,
[  --enable-ai1		Add -a option to HOSTS entries, useful only with PASSTHRU NET/ECHOMAIL],dnl
   [case "${enableval}" in
    yes | no) enable_ai1="${enableval}" ;;
      *) AC_MSG_ERROR(bad value ${enableval} for --enable-ai1) ;;
    esac],dnl
    enable_ai1=no)

if test $enable_ai1 = yes; then
    AC_DEFINE(AI_1,
              1,
	      [Add -a option to HOSTS entries, useful only with PASSTHRU_NET/ECHOMAIL])
fi

AC_ARG_ENABLE(best-aka,
[  --disable-best-aka	disable best AKA select],dnl
   [case "${enableval}" in
    yes | no) enable_best_aka="${enableval}" ;;
      *) AC_MSG_ERROR(bad value ${enableval} for --disable-best-aka) ;;
    esac],dnl
    enable_best_aka=no)

if test $enable_best_aka = yes; then
    AC_DEFINE(BEST_AKA,
              1,
	      [Enable best AKA select])
fi

AC_ARG_ENABLE(ftnacl,
[  --disable-ftnacl        do'nt use ACL for FTN links ],dnl
   [case "${enableval}" in
    yes | no) enable_ftnacl="${enableval}" ;;
      *) AC_MSG_ERROR(bad value ${enableval} for --disable-ftnacl) ;;
    esac],dnl
    enable_ftnacl=no)

if test $enable_ftnacl = yes; then
    AC_DEFINE(FTN_ACL,
              1,
	      [Enable ACL for FTN links])
fi

AC_ARG_WITH(desc-dir,
[  --with-desc-dir=DIR     put file descriptions into ".desc/%s.desc" file instead of files.bbs],
    DESC_DIR="$withval",DESC_DIR=".desc")

AC_ARG_ENABLE(desc-dir,
[  --disable-desc-dir      put file descriptions into files.bbs],dnl
   [case "${enableval}" in
    yes | no) enable_desc_dir="${enableval}" ;;
      *) AC_MSG_ERROR(bad value ${enableval} for --disable-desc-dir) ;;
    esac],dnl
    enable_desc_dir=yes)

if test $enable_desc_dir = yes; then
    AC_DEFINE_UNQUOTED(DESC_DIR,
    "$DESC_DIR",[
Put file descriptions into `.desc/%s.desc' file instead of files.bbs
.../fileecho/.desc/file.zip.desc -> ".desc",
.../fileecho/file.zip.desc ""
])
fi

AC_ARG_ENABLE(recode-file-desc,
[  --disable-recode-file-desc disable recode file descriptions],dnl
   [case "${enableval}" in
    yes | no) enable_recode_file_desc="${enableval}" ;;
      *) AC_MSG_ERROR(bad value ${enableval} for --disable-recode-file-desc) ;;
    esac],dnl
    enable_recode_file_desc=yes)

if test $enable_recode_file_desc = yes; then
    AC_DEFINE(RECODE_FILE_DESC,
              1,
	      [Recode file descriptions])
fi

AC_ARG_ENABLE(af-avail,
[  --disable-af-avail      change behaviour of areafix command "%list" and "%avail" ],dnl
   [case "${enableval}" in
    yes | no) enable_af_afail="${enableval}" ;;
      *) AC_MSG_ERROR(bad value ${enableval} for --disable-af-afail) ;;
    esac],dnl
    enable_af_afail=yes)

if test $enable_af_afail = yes; then
    AC_DEFINE(AF_AVAIL,
              1,[
  Enable areafix command `%avail', change behaviour of areafix
  command `%list'
])
fi

AC_ARG_ENABLE(xff-echomail,
[  --disable-xff-echomail  add X-FTN-From header to gated echomail],dnl
   [case "${enableval}" in
    yes | no) enable_xff_echomail="${enableval}" ;;
      *) AC_MSG_ERROR(bad value ${enableval} for --disable-xff-echomail) ;;
    esac],dnl
    enable_xff_echomail=yes)

if test $enable_xff_echomail = yes; then
    AC_DEFINE(X_FTN_FROM_ECHOMAIL,
              1,
	      [Add `X-FTN-From' header in gated echomail.])
fi

AC_ARG_ENABLE(spyes,
[  --disable-spyes         use SPYES config],dnl
   [case "${enableval}" in
    yes | no) enable_spyes="${enableval}" ;;
      *) AC_MSG_ERROR(bad value ${enableval} for --disable-spyes) ;;
    esac],dnl
    enable_spyes=yes)

if test $enable_spyes = yes; then
    AC_DEFINE(SPYES,
              1,
	      [Added the feature "spy forwards".])
fi

AC_ARG_ENABLE(aflr,
[  --disable-aflr          restricted access to areafix command %listall],dnl
   [case "${enableval}" in
    yes | no) enable_aflr="${enableval}" ;;
      *) AC_MSG_ERROR(bad value ${enableval} for --disable-aflr) ;;
    esac],dnl
    enable_aflr=yes)

if test $enable_aflr = yes; then
    AC_DEFINE(AF_LISTALL_RESTRICTED,
              1,
	      [Restricted access to areafix command `%listall'])
fi

AC_ARG_ENABLE(fecho-pass,
[  --disable-fecho-pass    use passthrought filearea],dnl
   [case "${enableval}" in
    yes | no) enable_fecho_pass="${enableval}" ;;
      *) AC_MSG_ERROR(bad value ${enableval} for --disable-fecho-pass) ;;
    esac],dnl
    enable_fecho_pass=yes)

if test $enable_fecho_pass = yes; then
    AC_DEFINE(FECHO_PASSTHROUGHT,
              1,
	      [Enable passthrought filearea])
fi

AC_ARG_ENABLE(use-filebox,
[  --disable-use-filebox   use fileboxes for fileecho],dnl
   [case "${enableval}" in
    yes | no) enable_use_filebox="${enableval}" ;;
      *) AC_MSG_ERROR(bad value ${enableval} for --disable-use-filebox) ;;
    esac],dnl
    enable_use_filebox=yes)

if test $enable_use_filebox = yes; then
    AC_DEFINE(USE_FILEBOX,
              1,
	      [Enable if use fileboxes for fileecho])
fi

AC_ARG_ENABLE(tick-crc,
[  --disable-tick-crc      ftntick CRC control],dnl
   [case "${enableval}" in
    yes | no) enable_tick_crc="${enableval}" ;;
      *) AC_MSG_ERROR(bad value ${enableval} for --disable-tick-crc) ;;
    esac],dnl
    enable_tick_crc=yes)

if test $enable_tick_crc = yes; then
    AC_DEFINE(FTNTICK_NOCRC,
              1,
	      [Enable if no ftntick CRC control])
fi

AC_ARG_ENABLE(no-organization,
[  --enable-no-organization enable if no insert Origin line if it absence],dnl
   [case "${enableval}" in
    yes | no) enable_no_organization="${enableval}" ;;
      *) AC_MSG_ERROR(bad value ${enableval} for --enable-no-organization) ;;
    esac],dnl
    enable_no_organization=no)

if test $enable_no_organization = yes; then
    AC_DEFINE(NOINSERT_ORGANIZATION,
              1,
	      [Enable if no insert Origin line if it absence])
fi

AC_ARG_ENABLE(insecure-not-db,
[  --disable-insecure-not-db put insecure mail into dupe database],
   [case "${enableval}" in
    yes | no) enable_insecure_not_db="${enableval}" ;;
      *) AC_MSG_ERROR(bad value ${enableval} for --disable-insecure-not-db) ;;
    esac],dnl
    enable_insecure_not_db=yes)

if test $enable_insecure_not_db = yes; then
    AC_DEFINE(INSECURE_DONT_PUT_INTO_DUPE_DB,
              1,
	      [Don't put insecure mail into dupe database])
fi

AC_ARG_ENABLE(af-ansok,
[  --enable-af-ansok       change areafix answering to "ok" if subscribing],dnl
   [case "${enableval}" in
    yes | no) enable_af_ansok="${enableval}" ;;
      *) AC_MSG_ERROR(bad value ${enableval} for --enable-af-ansok) ;;
    esac],dnl
    enable_af_ansok=no)

if test $enable_af_ansok = yes; then
    AC_DEFINE(ANSWER_OK,
              1,
	      [Change areafix answer when subscibing to area from `subscribe' to `ok'.])
fi

AC_ARG_ENABLE(fs-msgid,
[  --disable-fs-msgid      set gate msgid into outgoing messages],
   [case "${enableval}" in
    yes | no) enable_fs_msgid="${enableval}" ;;
      *) AC_MSG_ERROR(bad value ${enableval} for --disable-fs-msgid) ;;
    esac],dnl
    enable_fs_msgid=yes)

if test $enable_fs_msgid = yes; then
    AC_DEFINE(FIDO_STYLE_MSGID,
              1,
	      [Set fido-style msgid])
fi

AC_ARG_ENABLE(dbc-history,
[  --enable-dbc-history    use msgid correspond data base],dnl
   [case "${enableval}" in
    yes | no) enable_dbc_history="${enableval}" ;;
      *) AC_MSG_ERROR(bad value ${enableval} for --enable-dbc-history) ;;
    esac],dnl
    enable_dbc_history=no)

if test $enable_dbc_history = yes; then
    AC_DEFINE(DBC_HISTORY,
              1,
	      [Set msgid correspond data base])
fi

AC_ARG_ENABLE(afses,
[  --enable-afses          areafix send echo status],dnl
   [case "${enableval}" in
    yes | no) enable_afses="${enableval}" ;;
      *) AC_MSG_ERROR(bad value ${enableval} for --enable-afses) ;;
    esac],dnl
    enable_afses=no)

if test $enable_afses = yes; then
    AC_DEFINE(AFSEND_ECHO_STATUS,
              1,
	      [Enable areafix send echo status])
fi

AC_ARG_ENABLE(sub-zonegate,
[  --enable-sub-zonegate   subscribe only zonegate],dnl
   [case "${enableval}" in
    yes) enable_sub_zonegate=yes ;;
     no) enable_sub_zonegate=no ;;
      *) AC_MSG_ERROR(bad value ${enableval} for --enable-sub-zonegate) ;;
    esac],dnl
    enable_sub_zonegate=no)

if test $enable_sub_zonegate = yes; then
    AC_DEFINE(SUBSCRIBE_ZONEGATE,
              1,
	      [Enable subscribe autorization only if node listed in zonegate])
fi

AC_ARG_ENABLE(pid2rd-tid2gtv,
[  --enable-pid2rd-tid2gtv paste ^PID & ^TID kludges to message],dnl
   [case "${enableval}" in
    yes) enable_pid2rd_tid2gtv=yes ;;
     no) enable_pid2rd_tid2gtv=no ;;
      *) AC_MSG_ERROR(bad value ${enableval} for --enable-pid2rd-tid2gtv) ;;
    esac],dnl
    enable_pid2rd_tid2gtv=no)

if test $enable_pid2rd_tid2gtv = yes; then
    AC_DEFINE(PID_READER_TID_GTV,
              1,
	      [Enable paste ^PID (mail or news reader name)
	      and ^TID (fidogate version) kludge])
fi

AC_ARG_ENABLE(tick-history,
[  --disable-tick-history  tick dupe check],dnl
   [case "${enableval}" in
    yes) enable_tick_history=yes ;;
     no) enable_tick_history=no ;;
      *) AC_MSG_ERROR(bad value ${enableval} for --disable-tick-history) ;;
    esac],dnl
    enable_tick_history=yes)

if test $enable_tick_history = yes; then
    AC_DEFINE(TIC_HISTORY,
              1,
	      [Enable tic dupe check])
fi

AC_ARG_ENABLE(dnt-netmail,
[  --enable-dnt-netmail    do not toss netmail],dnl
   [case "${enableval}" in
    yes) enable_dnt_netmail=yes ;;
     no) enable_dnt_netmail=no ;;
      *) AC_MSG_ERROR(bad value ${enableval} for --enable-dnt-netmail) ;;
    esac],dnl
    enable_dnt_netmail=no)

if test $enable_dnt_netmail = yes; then
    AC_DEFINE(DO_NOT_TOSS_NETMAIL,
              1,
	      [Disable tosting netmail and set directory for incoming and outgoing pkt's])
fi

AC_ARG_ENABLE(owfl,
[  --disable-owfl          overwrite fileecho link],dnl
   [case "${enableval}" in
    yes) enable_owfl=yes ;;
     no) enable_owfl=no ;;
      *) AC_MSG_ERROR(bad value ${enableval} for --disable-owfl) ;;
    esac],dnl
    enable_owfl=yes)

if test $enable_owfl = yes; then
    AC_DEFINE(OVERWRITEN_FECHO_FILE_TO_LINK,
              1,
	      [Enable overwrite hardlink if file already exist])
fi

AC_ARG_ENABLE(active-lookup,
[  --disable-active-lookup active file lookup],dnl
   [case "${enableval}" in
    yes) enable_active_lookup=yes ;;
     no) enable_active_lookup=no ;;
      *) AC_MSG_ERROR(bad value ${enableval} for --disable-active-lookup) ;;
    esac],dnl
    enable_active_lookup=yes)

if test $enable_active_lookup = yes; then
    AC_DEFINE(ACTIVE_LOOKUP,1,[Enable lookup to INN active file])
fi

AC_ARG_ENABLE(recode-allrfc,
[  --disable-recode-allrfc recoding some RFC header],
   [case "${enableval}" in
    yes) enable_recode_allrfc=yes ;;
     no) enable_recode_allrfc=no ;;
      *) AC_MSG_ERROR(bad value ${enableval} for --disable-recode-allrfc) ;;
    esac],
    enable_recode_allrfc=yes)
if test $enable_recode_allrfc = yes; then
    AC_DEFINE(RECODE_ALL_RFC,1,[Recode all RFC headers])
fi

AC_ARG_ENABLE(security,
[  --disable-security          disable additional address checking],dnl
   [case "${enableval}" in
    yes | no) enable_security="${enableval}" ;;
      *) AC_MSG_ERROR(bad value ${enableval} for --disable-security) ;;
    esac],dnl
    enable_security=yes)

if test $enable_security = yes; then
    AC_DEFINE(SECURITY,
              1,
	      [
For the security reason enabled additional addresses checking in the
.pkt header, message header, origin and echomail ^aPATH.
If the last address in PATH is not equal to address in the pkt header,
then the address from the pkt header is inserted into PATH.
])
fi

AC_ARG_ENABLE(fts-via,
[  --disable-fts-via          disable FTS format of VIA],dnl
   [case "${enableval}" in
    yes | no) enable_fts_via="${enableval}" ;;
      *) AC_MSG_ERROR(bad value ${enableval} for --disable-fts-via) ;;
    esac],dnl
    enable_fts_via=yes)

if test $enable_fts_via = yes; then
    AC_DEFINE(FTS_VIA,
              1,
	      [
Change ^AVia format from:
   ^AVia FIDOGATE/ftntoss 2:5030/1229.0, 20010405.164323.MSK
to:
   ^AVia 2:5030/1229.0 @20010405.164323.MSK FIDOGATE/ftntoss
])
fi

AC_ARG_ENABLE(create-log-forwreq,
[  --enable-create-log-forwreq    use msgid correspond data base],dnl
   [case "${enableval}" in
    yes | no) enable_create_log_forwreq="${enableval}" ;;
      *) AC_MSG_ERROR(bad value ${enableval} for --enable-create-log-forwreq) ;;
    esac],dnl
    enable_create_log_forwreq=no)

if test $enable_create_log_forwreq = yes; then
    AC_DEFINE(CREATE_LOG_FORWREQ,
              1,
	      [Don't put info about create to log
              if echo created by forward request])
fi

AC_ARG_ENABLE(del-mime-if-rfc2,
[  --enable-del-mime-if-rfc2    delete MIME if rfc status set to 2],dnl
   [case "${enableval}" in
    yes | no) enable_del_mime_if_rfc2="${enableval}" ;;
      *) AC_MSG_ERROR(bad value ${enableval} for --enable-del-mime-if-rfc2) ;;
    esac],dnl
    enable_del_mime_if_rfc2=no)

if test $enable_del_mime_if_rfc2 = yes; then
    AC_DEFINE(DEL_MIME_IF_RFC2,
              1,
	      [Delete MIME if rfc status set to 2])
fi

AC_ARG_ENABLE(domain-to-origin,
[  --enable-domain-to-origin    paste domain name to Origin message line],dnl
   [case "${enableval}" in
    yes | no) enable_domain_to_origin="${enableval}" ;;
      *) AC_MSG_ERROR(bad value ${enableval} for --enable-domain-to-origin) ;;
    esac],dnl
    enable_domain_to_origin=no)

if test $enable_domain_to_origin = yes; then
    AC_DEFINE(DOMAIN_TO_ORIGIN,
              1,
	      [Paste domain name to Origin message line])
fi

AC_ARG_ENABLE(do-not-toss-netmail,
[  --enable-do-not-toss-netmail    disable tosting netmail and set directory for incoming and outgoing pkt's],dnl
   [case "${enableval}" in
    yes | no) enable_do_not_toss_netmail="${enableval}" ;;
      *) AC_MSG_ERROR(bad value ${enableval} for --enable-do-not-toss-netmail) ;;
    esac],dnl
    enable_do_not_toss_netmail=no)

if test $enable_do_not_toss_netmail = yes; then
    AC_DEFINE(DO_NOT_TOSS_NETMAIL,
              1,
	      [Disable tosting netmail and set directory
	      for incoming and outgoing pkt's])
fi

AC_ARG_ENABLE(sub-limit,
[  --disable-sub-limit		disable set limit for subscribe areas],dnl
   [case "${enableval}" in
    yes | no) enable_sub_limit="${enableval}" ;;
      *) AC_MSG_ERROR(bad value ${enableval} for --disable-sub-limit) ;;
    esac],dnl
    enable_sub_limit=yes)

if test $enable_sub_limit = yes; then
    AC_DEFINE(SUB_LIMIT,
              1,
	      [Enable set limit for subscribe areas])
fi

AC_ARG_ENABLE(fix-bad-pkt-year,
[  --enable-fix-bad-pkt-year    fix bad year in pkt's],dnl
   [case "${enableval}" in
    yes | no) enable_fix_bad_pkt_year="${enableval}" ;;
      *) AC_MSG_ERROR(bad value ${enableval} for --enable-fix-bad-pkt-year) ;;
    esac],dnl
    enable_fix_bad_pkt_year=no)

if test $enable_fix_bad_pkt_year = yes; then
    AC_DEFINE(FIX_BAD_PKT_YEAR,
              1,
	      [Fix bad year in pkt's])
fi

AC_ARG_ENABLE(ignore-from-if-reply,
[  --enable-ignore-from-if-reply    ignore From: field if REPLYTO or REPLYADDR was found],dnl
   [case "${enableval}" in
    yes | no) enable_ignore_from_if_reply="${enableval}" ;;
      *) AC_MSG_ERROR(bad value ${enableval} for --enable-ignore-from-if-reply) ;;
    esac],dnl
    enable_ignore_from_if_reply=no)

if test $enable_ignore_from_if_reply = yes; then
    AC_DEFINE(IGNORE_FROM_IF_REPLY,
              1,
	      [Ignore From: field if REPLYTO or REPLYADDR was found])
fi

AC_ARG_ENABLE(insert-origin,
[  --enable-insert-origin    if Origin line is not present, include link address],dnl
   [case "${enableval}" in
    yes | no) enable_insert_origin="${enableval}" ;;
      *) AC_MSG_ERROR(bad value ${enableval} for --enable-insert-origin) ;;
    esac],dnl
    enable_insert_origin=no)

if test $enable_insert_origin = yes; then
    AC_DEFINE(INSERT_ORIGIN,
              1,
	      [If Origin line is not present, include link address])
fi

AC_ARG_ENABLE(old-toss,
[  --enable-old-toss    enable old style tosting],dnl
   [case "${enableval}" in
    yes | no) enable_old_toss="${enableval}" ;;
      *) AC_MSG_ERROR(bad value ${enableval} for --enable-old-toss) ;;
    esac],dnl
    enable_old_toss=no)

if test $enable_old_toss = yes; then
    AC_DEFINE(OLD_TOSS,
              1,
	      [Enable old style tosting])
fi

AC_ARG_ENABLE(passthru-netmail,
[  --enable-passthru-netmail    passthru operation for NetMail: FIDO->Internet->FIDO],dnl
   [case "${enableval}" in
    yes | no) enable_passthru_netmail="${enableval}" ;;
      *) AC_MSG_ERROR(bad value ${enableval} for --enable-passthru-netmail) ;;
    esac],dnl
    enable_passthru_netmail=no)

if test $enable_passthru_netmail = yes; then
    AC_DEFINE(PASSTHRU_NETMAIL,
              1,
	      [Passthru operation for NetMail: FIDO->Internet->FIDO])
fi

AC_ARG_ENABLE(passthru-echomail,
[  --enable-passthru-echomail    passthru operation for EchoMail: FIDO->Internet->FIDO],dnl
   [case "${enableval}" in
    yes | no) enable_passthru_echomail="${enableval}" ;;
      *) AC_MSG_ERROR(bad value ${enableval} for --enable-passthru-echomail) ;;
    esac],dnl
    enable_passthru_echomail=no)

if test $enable_passthru_echomail = yes; then
    AC_DEFINE(PASSTHRU_ECHOMAIL,
              1,
	      [Passthru operation for EchoMail: FIDO->Internet->FIDO])
fi

AC_ARG_ENABLE(sn,
[  --enable-sn    enable SN-style active_lookup],dnl
   [case "${enableval}" in
    yes | no) enable_sn="${enableval}" ;;
      *) AC_MSG_ERROR(bad value ${enableval} for --enable-sn) ;;
    esac],dnl
    enable_sn=no)

if test $enable_sn = yes; then
    AC_DEFINE(SN,
              1,
	      [Enable SN-style active_lookup])
fi

AC_ARG_ENABLE(syslog,
[  --enable-syslog    enable if you want use syslog for log messages],
   [case "${enableval}" in
    yes | no) enable_syslog="${enableval}" ;;
      *) AC_MSG_ERROR(bad value ${enableval} for --enable-syslog) ;;
    esac],dnl
    enable_syslog=no)

if test $enable_syslog = yes; then
    AC_DEFINE(USE_SYSLOG,
              1,
	      [Enable if you want use syslog for log messages])
fi

dnl Libraries
AC_SEARCH_LIBS(regcomp, rx regex, [
    AC_DEFINE(HAS_POSIX_REGEX,[],[Do we have a decent regex library?])
    AC_CHECK_HEADER(rxposix.h, AC_DEFINE(HAVE_RX,[],[Do we have rxposix.h?]))])

if test "$with_news" != no ;then

    AS_IF([test "x$enable_oldbatcher" != "xno"], [
        BATCHER='time $PATHBIN/batcher -N $QUEUEJOBS -b500000 -p"$RFC2FTN -b -n" $SITE $BATCHFILE' ],
	[BATCHER='time $RFC2FTN -f $BATCHFILE -m 500'])

# MY CHECK IF

    AS_CASE([$with_inndir],
            [yes|no], [
                 AC_MSG_ERROR([--with-inndir requires an argument; use --with-inndir=PATH])],
            [undefined], [PATHNEWS=""],
            [PATHNEWS="$with_inndir"
	     AS_IF([test "$with_newsbindir" != "no"],
	       [ PATHBIN=$with_newsbindir ],
	       [ PATHBIN="$PATHNEWS/bin" ])
	     NEWSLIBDIR=$PATHNEWS
             RNEWS="$PATHBIN/rnews"
             INEWS="$PATHBIN/inews"
            ])
#MY CHECK CASE
    AS_CASE([$with_innetcdir],
            [yes|no], [
                 AC_MSG_ERROR([--with-innetcdir requires an argument; use --with-innetcdir=PATH])],
            [undefined], [PATHETC=""],
            [PATHETC="$with_innetcdir"])

    AS_CASE([$with_inndbdir],
            [yes|no], [
                 AC_MSG_ERROR([--with-inndbdir requires an argument; use --with-inndbdir=PATH])],
            [undefined], [PATHDB=""],
            [PATHDB="$with_inndbdir"])

    AS_CASE([$with_innspooldir],
            [yes|no], [
                 AC_MSG_ERROR([--with-innspooldir requires an argument; use --with-innspooldir=PATH])],
            [undefined], [PATHARTICLES=""],
            [PATHARTICLES="$with_innspooldir/articles"
             PATHOUTGOING="$with_innspooldir/outgoing"
            ])

# if something is not provided to configure, do autodetect

    AS_IF([test -z "$PATHNEWS" -o -z "$PATHETC" \
                -o -z "$PATHDB" -o -z "$PATHARTICLES"],
    [
    AC_MSG_NOTICE([Something from --with-inndir, --with-innetcdir, --with-inndbdir, --with-innspooldir is missing. Detecting])
    AC_MSG_CHECKING(for news-library path)
    if ! test -x "${newsbindir}/innconfval"; then
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
	    AC_MSG_ERROR([rnews not found])
	fi
    fi

    AC_MSG_CHECKING(for inews)
    if test "${with_inews+set}" = set ;then
	INEWS="$with_inews"
    else
	AC_PATH_PROG(INEWS, inews,, $XPATH)
	if test "x$INEWS" = "x"; then
	    AC_MSG_ERROR([inews not found])
	fi
    fi

    if $NEWSLIBDIR/bin/innconfval -v 2>&1 </dev/null | egrep 2\.\[3\|4\|5\|6] > /dev/null; then
	AC_DEFINE(OLD_BATCHER, 1,
	    [Enable use old batcher in runinc program])
	BATCHER='time $PATHBIN/batcher -N $QUEUEJOBS -b500000 -p"$RFC2FTN -b -n" $SITE $BATCHFILE'
    else
	BATCHER='time $RFC2FTN -f $BATCHFILE -m 500'
    fi

    if test "$with_news" != no ;then
	if test "$with_owner" = news -a "x$with_rsf" != xno; then
	    AC_DEFINE(USE_RUNINC_SFGT, 1,
	        [Enable use send_fidogate function in runinc program])
	fi
    fi
])
else # if test "$with_news" != no ;then
    AC_MSG_NOTICE([Building without news support])
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

AH_VERBATIM(HAVE_TM_GMTOFF,
[
/* left from initial configurator */
#undef HAVE_TM_GMTOFF
])

# Checking sendmail
AC_MSG_CHECKING(for sendmail)
if test "${with_sendmail+set}" = set ;then
	SENDMAIL="$with_sendmail"
else
    XPATH="/sbin:/usr/sbin:/usr/lib:/usr/ucblib:/usr/share"
    AC_PATH_PROG(SENDMAIL, sendmail,, $XPATH)
    if test "x$SENDMAIL" = "x"; then
	AC_MSG_ERROR([sendmail not found, re-run with valid --with-sendmail=DIR])
    fi
fi
AC_SUBST(SENDMAIL)

AC_MSG_CHECKING(for lock directory)
if test "${with_lockdir+set}" = set; then
	LOCKDIR="$with_lockdir"
else
	LOCKDIR="${localstatedir}/run/fidogate"
fi
AC_MSG_RESULT($LOCKDIR)
AC_SUBST(LOCKDIR)

AC_MSG_CHECKING(for var directory)
if test "${with_vardir+set}" = set; then
	vardir="$with_vardir"
else
	vardir="${localstatedir}/lib/fidogate"
fi
AC_MSG_RESULT(${vardir})
AC_SUBST(vardir)

AC_MSG_CHECKING(for log directory)
if test "${with_logdir+set}" = set; then
	LOGDIR="$with_logdir"
else
	LOGDIR="${localstatedir}/log/fidogate"
fi
AC_MSG_RESULT($LOGDIR)
AC_SUBST(LOGDIR)

AC_MSG_CHECKING(for spool directory)
if test "${with_spooldir+set}" = set; then
	SPOOLDIR="$with_spooldir"
else
	SPOOLDIR="${localstatedir}/spool/fidogate"
fi
AC_MSG_RESULT($SPOOLDIR)
AC_SUBST(SPOOLDIR)

SUBSTCMD='$(PERL) $(top_srcdir)/build/subst.pl -c$(top_srcdir)/build/config.make -t$(top_srcdir)'
M4MAKE='$(M4) $(M4OPTIONS)'

AC_SUBST(SUBSTCMD)
AC_SUBST(M4MAKE)

AC_ARG_ENABLE([extraversion],
	AS_HELP_STRING([--disable-extraversion], [do not add GIT hash]))

AS_IF([test "x$enable_extraversion" != "xno"], [

  if test -d $srcdir/.git; then
    AC_DEFINE_UNQUOTED(GIT_HASH_STR,
	["`cd $srcdir && git log -1 --format=format:%h`"],
	[A string containing the git hash])
  fi

])

HOSTNAME=`hostname -s`
DOMAINNAME=`hostname|sed -e "s/$HOSTNAME//"`
AC_SUBST(HOSTNAME)
AC_SUBST(DOMAINNAME)

DATE="$(date -R)"
AC_SUBST(DATE)

AC_CONFIG_MACRO_DIRS([m4])

AC_CONFIG_COMMANDS([build_headers],[build/subst.pl -h])

AC_CONFIG_FILES([build/config.make \
                 Makefile \
		 src/Makefile \
		 tests/Makefile \
		 packages/rpm/fidogate.spec \
		 debian/changelog ])
AC_OUTPUT

# as template taken from kmod
AC_MSG_RESULT([
	$PACKAGE $VERSION
	=======

	prefix:			${prefix}
	sysconfdir:		${sysconfdir}
	libexecdir:		${libexecdir}
	includedir:		${includedir}
	bindir:			${bindir}
	logdir:			${LOGDIR}
	vardir:			${vardir}
	lockdir:		${LOCKDIR}
	spooldir:		${SPOOLDIR}
	btbasedir:		${btbasedir}
	netmaildir:		${netmaildir}

	compiler:		${CC}
	cflags:			${with_cflags} ${CFLAGS}
	ldflags:		${with_ldflags} ${LDFLAGS}

	sendmail:		${SENDMAIL}
	owner:			${OWNER}
	group:			${GROUP}

	NEWS:
	BATCHER:		$BATCHER
	RNEWS:			$RNEWS
	NEWSLIBDIR:		$NEWSLIBDIR
	PATHOUTGOING:		$PATHOUTGOING
	PATHARTICLES:		$PATHARTICLES
	PATHNEWS:		$PATHNEWS
	PATHBIN:		$PATHBIN
	PATHETC:		$PATHETC
	PATHDB:			$PATHDB
	INEWS:			$INEWS
])
