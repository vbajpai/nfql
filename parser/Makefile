make: numexpr
	 pip install -r requirements.txt

numexpr: numpy
	(pip install numexpr==2.0.1)

numpy: cython
	(pip install numpy==1.6.1)

cython:
	(pip install Cython==0.15.1)

clean:
	rm -f -r build/
	rm -f -r src/*.pyc
	rm -f -r flowy-run/
	rm -f -r parsetab.py parser.out
	rm -f -r examples/output.h5
