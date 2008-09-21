#:ts=8
#
#    $Id: Makefile.am,v 5.2 2004/11/23 00:50:39 anray Exp $
#
#    Makefile for FIDOGATE scripts/misc/
#

bin_SCRIPTS	=	hosts2dns \
			recvuu \
			senduu \
			senduumail

$(bin_SCRIPTS) $(noinst_SCRIPTS):
	$(SUBSTCMD) $@.in > $@
	chmod +x $@

clean-am:
	rm -f $(bin_SCRIPTS) $(noinst_SCRIPTS)

