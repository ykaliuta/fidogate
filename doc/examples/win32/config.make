# -*- Makefile -*-
#
#
# FIDOGATE main configuration (Win32 port)
#

##############################################################################
#         C O N F I G   P A R A M E T E R S   D E F I N I T I O N S          #
##############################################################################
#
# DEFAULT_* macros used by subst.pl:
#
#	DEFAULT_P_XXX	variable config parameter
#			- default value define here
#			- substitutes <XXX> in scripts
#			- config file parameter XXX
#			- C code cf_p_xxx(), cf_s_xxx() automatically generated
#			- #define DEFAULT_XXX in paths.h
#
#	DEFAULT_F_XXX	fixed config parameter
#			- default value define here
#			- substitutes <XXX> in scripts
#			- #define DEFAULT_XXX in paths.h
#
#	DEFAULT_A_XXX	abbreviation %X for config parameters
#			- C, perl code automatically generated
#
# WARNING!!!
# DON'T REMOVE ANY OF THE MACRO DEFINITIONS BELOW, THIS MAY BREAK FIDOGATE!!!!
# 
##############################################################################
#
# Directory		Compile-time		Run-time	Abbrev
# ---------		------------		--------	------
# Config		DEFAULT_CONFIGDIR	ConfigDir	%C
# Main lib		DEFAULT_LIBDIR		LibDir		%L
# Admin utilities	DEFAULT_BINDIR		BinDir		%N
# Log			DEFAULT_LOGDIR		LogDir		%G
# Var lib		DEFAULT_VARDIR		VarDir		%V
# Lock files		DEFAULT_LOCKDIR		LockDir		%K
# Spool			DEFAULT_SPOOLDIR	SpoolDir	%S
# Outbound/inbound base	DEFAULT_BTBASEDIR	BTBaseDir	%B
#
# Inbound		DEFAULT_INBOUND		Inbound		%I
# Protected inbound	DEFAULT_PINBOUND	PInbound	%P
# Uuencode inbound	DEFAULT_UUINBOUND	UUInbound	%U
# FTP inbound		DEFAULT_FTPINBOUND	FTPInbound
#
# INN config		DEFAULT_NEWSETCDIR
# INN var lib		DEFAULT_NEWSVARDIR
# INN main lib		DEFAULT_NEWSLIBDIR
# INN spool		DEFAULT_NEWSSPOOL
#
# Ifmail main lib	DEFAULT_IFMAILDIR
#

# variable parameters, can be changed at run-time, DO NOT DELETE ANYTHING!!!
DEFAULT_V_CONFIGDIR	= /fidogate/conf
DEFAULT_V_LIBDIR	= /fidogate/lib
DEFAULT_V_BINDIR	= /fidogate/bin
DEFAULT_V_LOGDIR	= /fidogate/log
DEFAULT_V_VARDIR	= /fidogate/var
DEFAULT_V_LOCKDIR	= /fidogate/lock
DEFAULT_V_SPOOLDIR	= /fidogate/spool
DEFAULT_V_BTBASEDIR	= /fidogate/bt
DEFAULT_V_INBOUND	= $(DEFAULT_V_BTBASEDIR)/in
DEFAULT_V_PINBOUND	= $(DEFAULT_V_BTBASEDIR)/pin
DEFAULT_V_UUINBOUND	= $(DEFAULT_V_BTBASEDIR)/uuin
DEFAULT_V_FTPINBOUND	= $(DEFAULT_V_BTBASEDIR)/ftpin

DEFAULT_V_ACL		= %C/acl
DEFAULT_V_ALIASES	= %C/aliases
DEFAULT_V_AREAS		= %C/areas
DEFAULT_V_HOSTS		= %C/hosts
DEFAULT_V_PASSWD	= %C/passwd
DEFAULT_V_PACKING	= %C/packing
DEFAULT_V_ROUTING	= %C/routing
DEFAULT_V_HISTORY	= %V/history
DEFAULT_V_LOGFILE	= %G/log
DEFAULT_V_CHARSETMAP	= %L/charset.bin


# fixed parameters, DO NOT DELETE ANYTHING!!!
DEFAULT_F_NEWSETCDIR	= /fidogate/bin
DEFAULT_F_NEWSVARDIR	= <NEWSVARDIR>
DEFAULT_F_NEWSLIBDIR	= <NEWSLIBDIR>
DEFAULT_F_NEWSSPOOLDIR	= <NEWSSPOOLDIR>
DEFAULT_F_IFMAILDIR     = <IFMAILDIR>

# old-style config
#DEFAULT_F_CONFIG_GATE	= %C/gate.conf
#DEFAULT_F_CONFIG_MAIN	= %C/toss.conf
#DEFAULT_F_CONFIG_FFX	= %C/ffx.conf
# new-style config, single config file (4.3.0+)
DEFAULT_F_CONFIG_GATE	= %C/fidogate.conf
DEFAULT_F_CONFIG_MAIN	= %C/fidogate.conf
DEFAULT_F_CONFIG_FFX	= %C/fidogate.conf

DEFAULT_F_SEQ_MAIL	= %V/seq/mail
DEFAULT_F_SEQ_NEWS	= %V/seq/news
DEFAULT_F_SEQ_MSGID	= %V/seq/msgid
DEFAULT_F_SEQ_PKT	= %V/seq/pkt
DEFAULT_F_SEQ_SPLIT	= %V/seq/split
DEFAULT_F_SEQ_FF	= %V/seq/ff
DEFAULT_F_SEQ_TOSS	= %V/seq/toss
DEFAULT_F_SEQ_PACK	= %V/seq/pack
DEFAULT_F_SEQ_TICK	= %V/seq/tick

DEFAULT_F_LOCK_HISTORY	= history

DEFAULT_F_OUTRFC_MAIL	= %S/outrfc/mail
DEFAULT_F_OUTRFC_NEWS	= %S/outrfc/news
DEFAULT_F_OUTPKT	= %S/outpkt
DEFAULT_F_OUTPKT_MAIL	= %S/outpkt/mail
DEFAULT_F_OUTPKT_NEWS	= %S/outpkt/news

DEFAULT_F_TOSS_TOSS	= %S/toss/toss
DEFAULT_F_TOSS_ROUTE	= %S/toss/route
DEFAULT_F_TOSS_PACK	= %S/toss/pack
DEFAULT_F_TOSS_BAD	= %S/toss/bad

DEFAULT_F_TICK_HOLD	= %B/tick



# directory abbreviations, DO NOT CHANGE!!!
DEFAULT_A_CONFIGDIR	= %C
DEFAULT_A_LIBDIR	= %L
DEFAULT_A_BINDIR	= %N
DEFAULT_A_LOGDIR	= %G
DEFAULT_A_VARDIR	= %V
DEFAULT_A_LOCKDIR	= %K
DEFAULT_A_SPOOLDIR	= %S
DEFAULT_A_BTBASEDIR	= %B
DEFAULT_A_INBOUND	= %I
DEFAULT_A_PINBOUND	= %P
DEFAULT_A_UUINBOUND	= %U


# The perl interpreter used by subst.pl
# Requires ActiveState perl to be installed in directory c:\usr\...
PERL			= /usr/bin/perl

# Directories for installing documentation, not used by subst.pl
INFODIR			= /fidogate/doc
HTMLDIR			= /fidogate/doc
HTMLLOGDIR		= /fidogate/bin

##############################################################################

# The old names back again ... (to be out-phased)
BINDIR		= $(DEFAULT_V_BINDIR)
LIBDIR		= $(DEFAULT_V_LIBDIR)
SPOOLDIR	= $(DEFAULT_V_SPOOLDIR)
LOGDIR		= $(DEFAULT_V_LOGDIR)
OUTBOUND	= $(DEFAULT_V_BTBASEDIR)
INBOUND		= $(DEFAULT_V_INBOUND)
PINBOUND	= $(DEFAULT_V_PINBOUND)
UUINBOUND	= $(DEFAULT_V_UUINBOUND)

NEWSETCDIR	= $(DEFAULT_F_NEWSETCDIR)
NEWSVARDIR	= $(DEFAULT_F_NEWSVARDIR)
NEWSLIBDIR	= $(DEFAULT_F_NEWSLIBDIR)
NEWSSPOOLDIR	= $(DEFAULT_F_NEWSSPOOLDIR)
IFMAILDIR       = $(DEFAULT_F_IFMAILDIR)


##############################################################################
#           S Y S T E M   S P E C I F I C   D E F I N I T I O N S            #
##############################################################################
##FIXME:comment out?
#SHELL		= /bin/sh

# GNU m4
M4		= m4

# owner / group
OWNER		= 0
GROUP		= 0

# install permissions
PERM_PROG	= 755
PERM_DATA	= 644
PERM_SETUID	= 4755
PERM_DIR	= 755

# C compiler / flags
CC		= gcc
# YACC		= yacc					# Use yacc, not bison
YACC		= bison -y
AR		= ar
# RANLIB	= @echo >/dev/null			# No ranlib
RANLIB		= ranlib
# RANLIB	= ar s					# OS2

# DEBUG		= -O2
DEBUG		= -g

INCLUDE		= -I$(TOPDIR) -I$(TOPDIR)/src/include

# NEXTSTEP 3.3
# CFLAGS	= $(DEBUG) $(INCLUDE) -Wall -posix
# ISC 3.x
# CFLAGS	= $(DEBUG) $(INCLUDE) -Wall -posix -DISC
# OS2			   
# CFLAGS	= $(DEBUG) $(INCLUDE) -Wall -DOS2
# Linux, SunOS, Win32
CFLAGS		= $(DEBUG) $(INCLUDE) -Wall

# NEXTSTEP 3.3
# LFLAGS	= $(DEBUG) -L$(TOPDIR)/src/common -posix
# OS2
# LFLAGS	= -Zexe $(DEBUG) -L$(TOPDIR)/src/common
# Linux, SunOS, Win32
LFLAGS		= $(DEBUG) -L$(TOPDIR)/src/common

# ISC 3.x
# LIBS		= -lfidogate -linet -lPW -lcposix
LIBS		= -lfidogate

# installation program
# ISC 3.x: use bsdinst
INSTALL		= install
# MSDOS, OS2, WIN32
EXE		= .exe
#EXE		=
INSTALL_PROG	= $(INSTALL) -c -g $(GROUP) -o $(OWNER) -m $(PERM_PROG)
INSTALL_DATA	= $(INSTALL) -c -g $(GROUP) -o $(OWNER) -m $(PERM_DATA)
INSTALL_SETUID	= $(INSTALL) -c -g $(GROUP) -o $(OWNER) -m $(PERM_SETUID)
INSTALL_DIR	= $(INSTALL) -g $(GROUP) -o $(OWNER) -m $(PERM_DIR) -d
# extra prefix for installation
PREFIX		=

# library name
# LIB		= fidogate.a			# OS2
LIB		= libfidogate.a
