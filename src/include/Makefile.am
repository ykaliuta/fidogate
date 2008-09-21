#:ts=8
#
#    $Id: Makefile.am,v 5.2 2004/11/23 00:50:41 anray Exp $
#
#    Makefile for FIDOGATE src/include/
#

all-am:
	cmp paths.mak paths.h >/dev/null || cp paths.mak paths.h; \
	cmp cf_funcs.mak cf_funcs.h >/dev/null || cp cf_funcs.mak cf_funcs.h;\
	cmp $(top_srcdir)/src/common/cf_funcs.mak $(top_srcdir)/src/common/cf_funcs.c >/dev/null || cp $(top_srcdir)/src/common/cf_funcs.mak $(top_srcdir)/src/common/cf_funcs.c
	cmp $(top_srcdir)/src/common/cf_abbrev.mak $(top_srcdir)/src/common/cf_abbrev.c >/dev/null || cp $(top_srcdir)/src/common/cf_abbrev.mak $(top_srcdir)/src/common/cf_abbrev.c
	cmp $(top_srcdir)/src/common/cf_env.mak $(top_srcdir)/src/common/cf_env.c >/dev/null || cp $(top_srcdir)/src/common/cf_env.mak $(top_srcdir)/src/common/cf_env.c

distclean-am:
	rm -f cf_* paths.*
