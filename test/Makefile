#:ts=8
#
#    $Id: Makefile,v 1.1 2003/09/23 17:45:39 rusfidogate Exp $
#
#    Makefile for FIDOGATE test/
#

#
# Normally it should not be necessary to modify this file, see
# ../config.make and ../rules.make
#

TOPDIR		= ..

include $(TOPDIR)/build/config.make


all depend install:

all:
	mkdir -p bt tmp
	mkdir -p bt/out bt/pin
	(cd bin; ./mklinks)

check run::
	for d in tc.*; do (echo "$$d"; cd $$d; ./run) || exit 1; done 


clean::
	@for d in tc.* tmp*; do \
	if test -d $$d; then \
	(echo "$$d"; cd $$d; ../tr.clean; rm -f run.out); \
	fi \
	done 
	rm -rf bt tmp

distclean:: clean
	rm -f *~ *.bak *.o tags TAGS core paths.tmp
	(cd bin; rm -f ftn* runinc charset* rfc*)

verify::
	@for d in tc.*; do \
	  if [ -f $$d/run.verify ]; then \
	    (cd $$d; ../tr.verify $$d) \
	  fi; \
	done 

all-mk-verify::
	@for d in tc.*; do \
	  if [ -f $$d/run.verify ]; then \
	    (echo $$d; cd $$d; ../mk.verify $$d) \
	  fi; \
	done 

uninstall:
installdirs:
