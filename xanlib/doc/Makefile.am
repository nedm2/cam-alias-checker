EXTRA_DIST=		\
	xanlib-doc.tar.bz2

install-data-hook: xanlib-doc
	if test ! -e $(prefix)/share ; then						\
		echo "The directory $(prefix)/share not exist, I make it." ;	\
		mkdir $(prefix)/share ;						\
	fi ;
	if test ! -e $(prefix)/share/doc ; then						\
		echo "The directory $(prefix)/share/doc not exist, I make it." ;	\
		mkdir $(prefix)/share/doc ;						\
	fi ;
	cp -r ./xanlib-doc $(prefix)/share/doc/ ;

xanlib-doc: xanlib-doc.tar.bz2
	tar xfj $(srcdir)/xanlib-doc.tar.bz2

clean-local:
	rm -rf xanlib-doc

uninstall-hook:
	if test -e $(prefix)/share/doc/xanlib-doc ; then				\
		echo "Remove the directory $(prefix)/share/doc/xanlib-doc" ; 		\
		rm -rf $(prefix)/share/doc/xanlib-doc ;					\
	fi ;
