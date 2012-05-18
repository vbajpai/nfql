F (v2)
--------------------  
- - -

`F (v2)` is the execution engine of our in-house stream-based flow record query language `NFQL`. The query language allows to describe patterns in `NetFlow v5` records in a declarative and orthogonal fashion, making it easy to read and flexible enough to describe complex relationships among a given set of flows. It can process flow-records, aggregate them into groups, apply absolute (or relative) filters and invoke Allen interval algebra rules. The execution engine is written in `C`.


Installing on Ubuntu
--------------------  
- - -

Tried on Ubuntu 10.04 (LTS) x86_64 and 12.04 (LTS) x86_64

Install CMake

	>> sudo apt-get install cmake
	
Install Flow-tool Development Package

	>> sudo apt-get install flow-tools-dev	
	
Install Compression Library Development Package

	>> sudo apt-get install zlib1g-dev

Install JSON Manipulation Library Development Package

	>> sudo apt-get install libjson0-dev

Build Engine

	[engine] >> make

Install Doxygen (optional)

	>> sudo apt-get install doxygen
	
Install GraphVIZ (optional)

	>> sudo apt-get install graphviz
	
Generate Documentation (optional)	

	[engine] >> make doc
	
Cleanup
	
	[engine] >> make clean



Installing on Mac OS X
----------------------      
- - -  

Tried on Mac OS X 10.7.

Install [Homebrew &rarr;](http://mxcl.github.com/homebrew/)

	>> /usr/bin/ruby -e "$(/usr/bin/curl -fsSL https://raw.github.com/mxcl/homebrew/master/Library/Contributions/install_homebrew.rb)"

Install CMake

	>> brew install cmake
	
Install Flow-Tools from source

	>> wget http://dl.dropbox.com/u/500389/flow-tools-0.68.4.tar.bz2
	>> tar -xvf flow-tools-0.68.4.tar.bz2

	[flow-tools-0.68.4] >> ./configure
	[flow-tools-0.68.4] >> make 
	[flow-tools-0.68.4] >> make install	
	
Install JSON Manipulation Library Package

	>> brew install json-c
	
Build Engine

	[engine] >> make CMAKE_PREFIX_PATH=/usr/local/flow-tools/

	
Install Doxygen (optional)

	>> brew install doxygen

Install GraphVIZ (optional)

	>> brew install graphviz
	
Generate Documentation (optional)

	[engine] >> make doc	

Cleanup
	
	[engine] >> make clean
	

Running the Engine
----------------------      
- - -  

Some example queries are provided in `examples/` along with a sample trace.

	[engine] >> bin/engine examples/query-http-tcp-session.json examples/trace.ft 
	
The sample queries can also be run on your own `NetFlow v5` records

	[engine] >> flow-cat $TRACE[s] | bin/engine examples/query-http-tcp-session.json
