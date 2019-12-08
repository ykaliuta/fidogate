#:ts=8
#
#    Makefile for FIDOGATE scripts/
#

include $(top_builddir)/build/rules.make

binscripts = areas/areassucksync \
	  areas/areasbbssync \
	  nodelist/nl-del \
	  nodelist/nl-diff \
	  nodelist/nl-autoupd \
	  maintenance/logstat \
	  maintenance/logdaily \
	  maintenance/runchklock \
	  maintenance/logsendmail \
	  maintenance/logcheck \
	  maintenance/logreport \
	  misc/senduumail \
	  misc/hosts2dns \
	  misc/senduu \
	  misc/recvuu \
	  outbound/out-ls \
	  outbound/outb \
	  outbound/out-freq \
	  outbound/out-attach \
	  outbound/out-rm0 \
	  outbound/out-manip \
	  outbound/outb-kill \
	  outbound/out-rmbsy

noinstscripts = www/gate.cgi \
	  filebase/fb-filelist \
	  filebase/fb-chkfiles \
	  maintenance/fidogate.logrotate \
	  maintenance/rc.fidogate \
	  nodelist/html-tmpl \
	  nodelist/nl-2hosts \
	  nodelist/nl-print \
	  nodelist/nl-2routing \
	  nodelist/nl-html \
	  nodelist/nl-isdn

scripts = $(binscripts) $(noinstscripts)

bin_SCRIPTS		= $(binscripts)
noinst_SCRIPTS		= $(noinstscripts)

subst_scripts		= $(scripts)
cleanup_files		= $(scripts)
