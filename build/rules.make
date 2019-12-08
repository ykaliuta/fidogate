########### common rules

.SUFFIXES: .in

.in:
	$(SUBSTCMD) $< > $@
	@chmod +x $@

installcheck-am: $(install_hooks)

.PHONY: $(install_hooks)
