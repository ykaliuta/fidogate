#:ts=8
#
#    $Id: Makefile.am,v 5.2 2004/11/23 00:50:39 anray Exp $
#
#    Makefile for FIDOGATE scripts/filebase/
#

bin_SCRIPTS	=	nl-autoupd \
			nl-del \
			nl-diff

noinst_SCRIPTS	=	html-tmpl \
			nl-2hosts \
			nl-2routing \
			nl-html \
			nl-isdn \
			nl-print 

$(bin_SCRIPTS) $(noinst_SCRIPTS):
	$(SUBSTCMD) $@.in > $@
	chmod +x $@

clean-am:
	rm -f $(bin_SCRIPTS) $(noinst_SCRIPTS)
