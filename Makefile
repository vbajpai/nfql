all: build

build:
	pandoc INDEX.md -o INDEX.md.html
	pandoc RELEASES.md -o RELEASES.md.html
	pandoc INSTALL-OSX-HOMEBREW.md -o INSTALL-OSX-HOMEBREW.md.html
	pandoc INSTALL-OSX-MACPORTS.md -o INSTALL-OSX-MACPORTS.md.html
	pandoc INSTALL-DEBIAN.md -o INSTALL-DEBIAN.md.html
	pandoc INSTALL-FEDORA.md -o INSTALL-FEDORA.md.html
	pandoc INSTALL-FREEBSD.md -o INSTALL-FREEBSD.md.html
	
clean:
	rm -f -r INDEX.md.html RELEASES.md.html .DS_Store
	rm -f -r INSTALL-OSX-MACPORTS.md.html INSTALL-DEBIAN.md.html \
		       INSTALL-FEDORA.md.html INSTALL-OSX-HOMEBREW.md.html \
					 INSTALL-FREEBSD.md.html
