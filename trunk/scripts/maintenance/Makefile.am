#:ts=8
#
#    $Id: Makefile.am,v 5.2 2004/11/23 00:50:39 anray Exp $
#
#    Makefile for FIDOGATE scripts/filebase/
#

bin_SCRIPTS	=	logcheck \
			logdaily \
			logreport \
			logsendmail \
			logstat \
			runchklock

noinst_SCRIPT	=	fidogate.logrotate \
			rc.fidogate

$(bin_SCRIPTS) $(noinst_SCRIPTS):
	$(SUBSTCMD) $@.in > $@
	chmod +x $@

clean-am:
	rm -f $(bin_SCRIPTS) $(noinst_SCRIPTS)
