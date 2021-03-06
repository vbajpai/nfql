all: build make

install: build/Makefile
	(cd .build; make install)

test: build/Makefile
	(cd .build; make test)

make: build/Makefile
	(cd .build; make)

build/Makefile: build
	(cd .build; cmake -D CMAKE_PREFIX_PATH=$(CMAKE_PREFIX_PATH) ..)

build:
	mkdir -p .build

doc: Doxyfile
	(mkdir -p docs; doxygen Doxyfile)

clean:
	rm -f -r .build
	rm -f -r bin
	rm -f -r doc
	rm -f -r examples/*.json
	rm -f -r tests/*.pyc
	rm -f -r scripts/queries/*.pyc
