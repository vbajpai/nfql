# NFQL
- - - -

An efficient C implementation of the network flow query language (NFQL)

### Installation on Fedora Linux

Install Dependencies

    $ sudo yum install cmake flow-tools-devel json-c-devel

Install IPFIX protocol library

    $ sudo yum install glib2-devel
    $ wget http://tools.netsa.cert.org/releases/libfixbuf-1.4.0.tar.gz
    $ tar -zxvf libfixbuf-1.4.0.tar.gz
    [libfixbuf-1.4.0] $ ./configure
    [libfixbuf-1.4.0] $ make
    [libfixbuf-1.4.0] $ sudo make install

Build `nfql`

    [nfql] $ make

Read the man page

    $ sudo yum install most
    [nfql] $ nroff -man nfql.1 | most

Generate Documentation (optional)

    $ sudo yum install doxygen graphviz
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
