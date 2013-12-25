# NFQL
- - - -

An efficient C implementation of the network flow query language (NFQL).

*WARNING: This documentation is obsolete.*

### Installation on Mac OS X using MacPorts

Install [MacPorts &rarr;](http://guide.macports.org/#installing)

Install Dependencies:

    $ sudo port install cmake flow-tools json-c

Build `nfql`:

    [nfql] $ make CMAKE_PREFIX_PATH=/opt/local

Read the man page:

    [nfql] $ nroff -man nfql.1 | most

Generate Documentation (optional)

    $ sudo port install doxygen graphviz
    [nfql] $ make doc

Cleanup

    [nfql] $ make clean

#### Running `nfql`

Some example queries are provided in `examples/` along with a sample trace.

    [nfql] $ bin/nfql --ipfix examples/query-http-tcp-session.json examples/trace-2009.ipfix
    [nfql] $ bin/nfql examples/query-http-tcp-session.json examples/trace-2009.ft

The sample queries can also be run on your own `NetFlow v5` records

    [nfql] $ flow-cat tracefile[s] | bin/nfql examples/query-http-tcp-session.json

#### Running the Test Suite

To run the complete regression test-suite:

    [nfql] $ tests/regression.py [-v]

 Regression tests can also be run individually on a specific example
 query type. For instance:

    [nfql] $ tests/test-query-http-tcp-session.py [-v]

#### Running the Benchmarks

To run the `nfql` benchmarks:

    [nfql] $ make
    [nfql] $ sudo benchmarks/nfql.py bin/nfql tracefile[s]/ querie[s]/

Example `nfql` traces and queries are provided in `examples/`

To run the `SiLK` benchmarks:

    [nfql] $ sudo benchmarks/silk.py tracefile[s]/ querie[s]/

Example `SiLK` traces and queries are provided in `examples/silk/`
