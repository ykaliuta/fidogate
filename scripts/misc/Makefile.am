#:ts=8
#
#    $Id$
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

