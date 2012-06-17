F (v2)
--------------------  
- - -

`F (v2)` is the execution engine of our in-house stream-based flow record 
query language `NFQL`. The execution engine is written in `C` making it
relatively faster in order of magniture to the execution module of the 
first prototype implementation, `Flowy`. `F (v2)` takes the flow-query
in a `JSON` format and reads in the trace files in memory for efficient
processing. The `JSON` query itself can be generated using a `Python`
script. A new query and its corresponding unit test can quickly be 
prepared by adapting to one of the respective `example/` and `tests/` 
scripts provided in the package.

It is foreseen that in the future, the flowy query format previously used
by `Flowy` will be given to its validator module that will syntactically 
check and parse it to generate a `JSON` file. This `JSON` file will then
be piped into `F (v2)` for faster processing. 


Installing on Ubuntu
--------------------  
- - -

Tried on Ubuntu 10.04 (LTS) x86_64 and 12.04 (LTS) x86_64

Install CMake

	$ sudo apt-get install cmake
	
Install Flow-tool Development Package

  $ sudo apt-get install flow-tools-dev	
	
Install Compression Library Development Package

	$ sudo apt-get install zlib1g-dev

Install JSON Manipulation Library Development Package

	$ sudo apt-get install libjson0-dev

Build Engine

	[engine] $ make

Install Doxygen (optional)

	$ sudo apt-get install doxygen
	
Install GraphVIZ (optional)

	$ sudo apt-get install graphviz
	
Generate Documentation (optional)	

	[engine] $ make doc
	
Cleanup
	
	[engine] $ make clean



Installing on Mac OS X
----------------------      
- - -  

Tried on Mac OS X 10.7.

Install [Homebrew &rarr;](http://mxcl.github.com/homebrew/)

	$ /usr/bin/ruby -e "$(/usr/bin/curl -fsSL https://raw.github.com/mxcl/homebrew/master/Library/Contributions/install_homebrew.rb)"

Install CMake

	$ brew install cmake
	
Install Flow-Tools from source

	$ wget http://dl.dropbox.com/u/500389/flow-tools-0.68.4.tar.bz2
	$ tar -xvf flow-tools-0.68.4.tar.bz2

	[flow-tools-0.68.4] $ ./configure
	[flow-tools-0.68.4] $ make 
	[flow-tools-0.68.4] $ make install	
	
Install JSON Manipulation Library Package

	$ brew install json-c
	
Build Engine

	[engine] $ make CMAKE_PREFIX_PATH=/usr/local/flow-tools/

	
Install Doxygen (optional)

	$ brew install doxygen

Install GraphVIZ (optional)

	$ brew install graphviz
	
Generate Documentation (optional)

	[engine] $ make doc	

Cleanup
	
	[engine] $ make clean
	

Running the Engine
----------------------      
- - -  

Some example queries are provided in `examples/` along with a sample trace.

	[engine] $ bin/engine examples/query-http-tcp-session.json examples/trace-2009.ft 
	
The sample queries can also be run on your own `NetFlow v5` records

	[engine] $ flow-cat $TRACE[s] | bin/engine examples/query-http-tcp-session.json


Running the Test Suite
----------------------      
- - -  

To run the complete regression test-suite:

	[engine] $ tests/regression.py [-v]
	
Regression tests can also be run individually on a specific example query type. For instance:

	[engine] $ tests/test-query-http-tcp-session.py [-v]



Contributors
------------
- - - 

`F (v2)`: Vaibhav Bajpai <contact@vaibhavbajpai.com>, 2012  
`F (v1)`: Johannes 'josch' Schauer <j.schauer@email.de>, 2011
