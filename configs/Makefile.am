# Example configs which are installed

include $(top_builddir)/build/rules.make

dist_sysconf_DATA = bounce.acl \
		    bounce.acl_netmail \
		    bounce.addrinto \
		    bounce.down \
		    bounce.insecure \
		    bounce.noto \
		    bounce.restricted \
		    acl.sample \
		    aliases.sample \
		    areafix.help.sample \
		    areafix.help.rus \
		    fidogate.conf.sample \
		    fidokill.sample \
		    ftnacl.sample \
		    hosts.sample \
		    packing.sample \
		    passwd.sample \
		    routing.sample \
		    spyes.sample \
		    uplinks.sample

generated = fidogate.conf.sample \
	    packing.sample

subst_scripts        = $(generated)
CLEANFILES           = $(generated)

all-am: $(generated)

dist_var_DATA = areas.bbs.sample \
                fareas.bbs.sample
