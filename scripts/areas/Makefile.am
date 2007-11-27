#:ts=8
#
#    $Id$
#
#    Makefile for FIDOGATE scripts/areas/
#

bin_SCRIPTS	=	areasbbssync areassucksync

$(bin_SCRIPTS) $(noinst_SCRIPTS):
	$(SUBSTCMD) $@.in > $@
	chmod +x $@

clean-am:
	rm -f $(bin_SCRIPTS) $(noinst_SCRIPTS)
