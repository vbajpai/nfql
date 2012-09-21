# NFQL
- - - -

An efficient C implementation of a stream-based flow query language.

### Installation on Mac OS X using Homebrew

Install [Homebrew &rarr;](http://mxcl.github.com/homebrew/)

	$ /usr/bin/ruby -e "$(/usr/bin/curl -fsSL https://raw.github.com/mxcl/homebrew/master/Library/Contributions/install_homebrew.rb)"

Install Dependencies

	$ wget https://github.com/downloads/vbajpai/nfql/flow-tools-0.68.4.tar.bz2
	$ tar -xvf flow-tools-0.68.4.tar.bz2

	[flow-tools-0.68.4] $ ./configure
	[flow-tools-0.68.4] $ make 
	[flow-tools-0.68.4] $ make install	
	
	$ brew install json-c cmake doxygen graphviz
	
Build the Execution Engine

	[engine] $ make CMAKE_PREFIX_PATH=/usr/local/flow-tools/
	
#### Running the Execution Engine

Example queries are available in `examples/` along with sample traces.

	[engine] $ bin/engine examples/query-http-tcp-session.json examples/trace-2009.ftz

The sample queries can also be run on your own `NetFlow v5` records

	[engine] $ flow-cat $TRACE[s] | bin/engine examples/query-http-tcp-session.json


#### Running the Test Suite

To run the complete regression test-suite:

	[engine] $ tests/regression.py [-v]

Regression tests can also be run individually on a specific example query type. For instance:

	[engine] $ tests/test-query-http-tcp-session.py [-v]

#### Running the Benchmarks

To run the `NFQL` benchmarks:

	[engine] $ make
	[engine] $ sudo benchmarks/nfql.py bin/engine trace[s]/ querie[s]/

