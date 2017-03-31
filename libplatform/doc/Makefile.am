
MAINTAINERCLEANFILES = Makefile.in

EXTRA_DIST = doxy.config

if API_DOC
install-data-local: install-html-local

install-html-local: doxygen-docs
	$(INSTALL) -d $(docdir)
	cp -r html $(docdir)

doxygen-docs: doxy.config
	doxygen $<
endif

clean-local:
	rm -rf html
