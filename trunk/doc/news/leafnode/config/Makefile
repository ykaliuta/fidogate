install:	
	install -o news -g news -d $(DESTCFG)
	install -o news -g news -m 664 util-leafnode.conf $(DESTCFG)/util-leafnode.conf

uninstall:
	rm -f $(DESTCFG)/util-leafnode.conf
	rmdir $(DESTCFG)
save:
	save util-leafnode.conf
