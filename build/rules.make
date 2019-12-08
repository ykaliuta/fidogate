########### common rules

$(subst_scripts): %: %.in
	$(AM_V_GEN)$(SUBSTCMD) $< > $@
	@chmod +x $@

clean-am:
	rm -f $(cleanup_files)

distclean-am:
	rm -f $(distcleanup_files)

installcheck-am: $(install_hooks)

.PHONY: $(install_hooks)
