all: build make

install: build/Makefile
	(cd .build; make install)

test: build/Makefile
	(cd .build; make test)

make: build/Makefile
	(cd .build; make)

build/Makefile: build
	(cd .build; cmake ..)

build:
	mkdir -p .build

doc: Doxyfile
	doxygen Doxyfile

clean:
	rm -f  -r html 
	rm -f  -r latex
	rm -f  -r .build
	rm -f  -r bin
