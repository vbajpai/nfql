Flowy Parser
--------------------
- - -

Installing on Ubuntu
--------------------
- - -

Install External Dependencies

	$ sudo apt-get install libhdf5-serial-dev
	$ sudo apt-get install liblzo2-dev


Setup the Python Packaging Virtual Environment

    $ sudo apt-get install python-pip
    $ sudo pip install pip --upgrade
    $ sudo pip install virtualenv
    $ sudo pip install virtualenvwrapper
    
Create a Virtual Environment

	[parser] $ mkvirtualenv parser

Install Python Dependencies

	(parser)
	[parser] $ make
	
List the Installed Dependencies

	(parser)
	[parser] $ pip freeze	
	

Running the Parser
------------------
- - -

Some example queries and traces are provided in `examples/`.

	(parser)
	[parser] $ python src/flowy.py examples/query-skype.flw

View the generated `output.h5` file

	(parser)
	[parser] $ python src/printhdf.py examples/output.h5

View multiple `h5` files 
	
	(parser)
	[parser] $ python src/print_hdf_in_step.py examples/*.h5
	
Generate a `h5` trace from `flow-tools` format

	(parser)
	[parser] $ python src/ft2hdf.py $TRACEPATH input.h5
		

Cleanup
-------	
- - -

Remove the build files

	(parser)
	[parser] $ make clean

Deactivate the Virtual Environment

	(parser)
	[parser] $ deactivate
	
Destroy the Virtual Environment

	[parser] $ rmvirtualenv parser