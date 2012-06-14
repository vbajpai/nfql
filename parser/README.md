Flowy
--------------------
- - -

`Flowy` is the first prototype implementation of our Network Flow Query 
Language (`NFQL`). It is written in `Python`. The framework is divided into
two main modules: the validator module (parser) and the execution module. The 
validator modules is used for query parsing and syntax checking while the 
execution module is used to perform the actions of each stage of the processing
pipeline. Since the execution module of the prototype has been completely 
re-written in `C`, only the validator (parser) module is currently significant
to `F(v2)`. It is foreseen that the next iteration will strip off the execution
module and connect the validator module to the `C` execution engine.


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


Installing on Mac OS X
----------------------
- - -

Install [Homebrew &rarr;](http://mxcl.github.com/homebrew/)

    $ /usr/bin/ruby -e "$(/usr/bin/curl -fsSL https://raw.github.com/mxcl/homebrew/master/Library/Contributions/install_homebrew.rb)"
    
Install Python

	$ brew install python --framework
	
Put `easy_install` in PATH

	$ export PATH=/usr/local/share/python:$PATH
	
Setup the Python Packaging Virtual Environment

    $ easy_install pip
    $ pip install pip --upgrade
    $ pip install virtualenv
    $ pip install virtualenvwrapper
    $ source /usr/local/bin/virtualenvwrapper.sh
    
Install External Dependencies

	$ brew install hdf5
	$ brew install lzo   
    
Create a Virtual Environment

	[parser] $ mkvirtualenv parser

Install Python Dependencies

	(parser)
	[parser] $ make
	
List the Installed Dependencies

	(parser)
	[parser] $ pip freeze	
	

Usage
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


Contributors
------------
- - - 

Packaging and Documentation: Vaibhav Bajpai <contact@vaibhavbajpai.com>, 2012  
Implementation: Kaloyan Kanev <k.kanev@jacobs-university.de>, 2009  
