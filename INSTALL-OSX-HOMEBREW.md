# NFQL
- - - -

An efficient C implementation of the network flow query language (NFQL).

### Installation on Mac OS X using Homebrew

Install [Homebrew &rarr;](http://mxcl.github.com/homebrew/)

    $ ruby -e "$(curl -fsSL https://raw.github.com/Homebrew/homebrew/go/install)"

Install Dependencies

    $ brew install cmake
    $ brew install flow-tools

NOTE: `json-c` starting from v0.11 has renamed the library from
`libjson` to `libjson-c`. Debian-based systems are currently providing
v0.10 in the repository. In order to avoid complexity in the makefile,
we have decided to freeze the library dependency to v0.10. Install
`json-c` v0.10:

    $  brew install https://raw.github.com/mxcl/homebrew/9f78662acff4c3cb1bff3587c8ead7f5935925a9/Library/Formula/json-c.rb

Install IPFIX protocol library

    $ brew install libfixbuf
    $ brew install gettext
    $ brew link --force gettext

Build `nfql`

    [nfql] $ make

Read the man page

    $ brew install most
    [nfql] $ nroff -man nfql.1 | most

Generate Documentation (optional)

    $ brew install doxygen
    $ brew install graphviz
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
