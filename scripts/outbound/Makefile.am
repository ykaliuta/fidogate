#:ts=8
#
#    $Id: Makefile.am,v 5.2 2004/11/23 00:50:39 anray Exp $
#
#    Makefile for FIDOGATE scripts/outbound/
#

bin_SCRIPTS	=	out-attach \
			out-freq \
			out-ls \
			out-manip \
			out-rm0 \
			out-rmbsy \
			outb-kill \
			outb

$(bin_SCRIPTS) $(noinst_SCRIPTS):
	$(SUBSTCMD) $@.in > $@
	chmod +x $@

clean-am:
	rm -f $(bin_SCRIPTS) $(noinst_SCRIPTS)
