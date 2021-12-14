#:ts=8
#
#    $Id: Makefile,v 4.19 2004/08/27 14:20:02 n0ll Exp $
#
#    Makefile for FIDOGATE sendmail/cf/
#

#
# Normally it should not be necessary to modify this file, see
# ../config.make and ../rules.make
#

TOPDIR		= ../..

include $(TOPDIR)/config.make
include $(TOPDIR)/rules.make

M4OPTIONS	= -DconfFIDOGATE_LIBDIR=$(LIBDIR)
#M4OPTIONS	= -DconfFIDOGATE_LIBDIR=$(LIBDIR) \
#		  -DconfFIDOGATE_OUTDIR=%S/outpkt/mail


#CF		= redhat6-orodruin.cf redhat7-orodruin.cf \
#		  redhat6-morannon.cf
#CF		= orodruin.cf orodruin-smtp.cf orodruin-check.cf\
#		  morannon.cf morannon-ffx.cf morannon-check.cf\
#		  point.cf


all:

redhat6:	redhat6-morannon.cf
redhat7:	redhat7-orodruin.cf
redhat9:	redhat9-orodruin.cf redhat9-submit.cf redhat9-morannon.cf

redhat9-morannon.cf:	redhat9-morannon.mc
		$(M4) $(M4OPTIONS) $< | ./mycfpost >$*.cf



install:

install-orodruin:
		$(INSTALL) -c -o root -g root -m 444 redhat9-orodruin.mc \
		  /etc/mail/sendmail.mc
		$(INSTALL) -c -o root -g root -m 444 redhat9-orodruin.cf \
		  /etc/mail/sendmail.cf
		$(INSTALL) -c -o root -g root -m 444 redhat9-submit.mc \
		  /etc/mail/submit.mc
		$(INSTALL) -c -o root -g root -m 444 redhat9-submit.cf \
		  /etc/mail/submit.cf

install-morannon:
		$(INSTALL) -c -o root -g root -m 444 redhat9-morannon.mc \
		  /etc/mail/sendmail.mc
		$(INSTALL) -c -o root -g root -m 444 redhat9-morannon.cf \
		  /etc/mail/sendmail.cf


clean:
	rm -f *.cf
	rm -f *~ *.bak *.o tags TAGS core paths.tmp

test check verify:


#
# Dependencies
#
$(CF):		../mailer/ftn.m4 ../mailer/ffx.m4

depend:

depend-all:
