
SUBDIRS = third-party src

dist_doc_DATA = \
	README \
	COPYING \
	AUTHORS \
	ChangeLog \
	INSTALL \
	NEWS



# Remove doc directory on uninstall
uninstall-local:
	-rm -r $(docdir)
	-rm -r $(libtransmissiondir)

install-exec-local:
	-ldconfig


