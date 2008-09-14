#:ts=8
#
#    $Id: Makefile,v 4.5 2004/08/27 14:20:02 n0ll Exp $
#
#    Makefile for FIDOGATE sendmail/
#

#
# Normally it should not be necessary to modify this file, see
# ../config.make and ../rules.make
#

TOPDIR		= ..

include $(TOPDIR)/config.make
include $(TOPDIR)/rules.make

SUBDIR		= cf


all::		README


README:		$(SENDMAIL_CF_DIR)/README
	./mklinks $(SENDMAIL_CF_DIR)

make-cf:

install::

test::

clean::
	./mkclean
	rm -f *~ *.bak *.o tags TAGS core paths.tmp

all clean veryclean check test verify depend install::
	make -C $(SUBDIR) $@

check::

depend::

depend-all::
