# NFQL
- - - -

An efficient C implementation of a stream-based flow query language.

### Installation on Debian-based Linux

Install Dependencies

    $ sudo apt-get install cmake flow-tools-dev zlib1g-dev libjson0-dev
    $ sudo apt-get install doxygen graphviz

Build the Execution Engine

	[engine] $ make

Generate Documentation (optional)

  [engine] $ make doc

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

