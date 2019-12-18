########### common rules

.SUFFIXES: .in

.in:
	$(AM_V_GEN)$(SUBSTCMD) $< > $@
	@chmod +x $@

installcheck-am: $(install_hooks)

.PHONY: $(install_hooks)
