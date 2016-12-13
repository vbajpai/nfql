# NFQL
- - - -

An efficient C implementation of the network flow query language (NFQL).


### Query Language 

Understanding intricate traffic patterns require sophisticated flow
analysis tools that can mine flow records for complex use cases.
Unfortunately current tools fail to deliver owing to their language
design and simplistic filtering methods. We have designed a network flow
query language (NFQL) that aims to cater to such needs.

#### Features

- Filter flows.
- Combine flows into groups.
- Aggregate flows on flow-keys as one grouped flow aggregate.
- Merge grouped flows, supporting temporal relations between groups.
- Apply absolute or relative filters when grouping or merging.
- Unfold grouped flows back into individual flows.

### Implementation

`nfql`  is  a  reference  implementation of NFQL.  `nfql` is composed of
an execution engine and a query parser. The execution engine is the
brain of  `nfql`  where  the flows are processed. `nfql` reads a
flow‐query in an intermediate `JSON` format alongwith tracefiles that
are read  in memory for efficient processing. The query parser can be
used to read a custom query domain specific language (DSL) and generate
the `JSON` intermediate representation required by the execution engine.

 A common usage is shown below:

     $ nfql examples/query‐tcp‐session.json examples/trace‐2009.ftz
     $ flow‐cat tracefile[s] | nfql examples/query‐tcp‐session.json ‐
     $ nfql ‐-ipfix examples/query‐tcp‐session.json examples/trace‐2009.ipfix

#### Features

- Supports reading and writing NetFlow v5 flows in `flow-tools` format and IPFIX flows in [RFC 5655](http://tools.ietf.org/search/rfc5655) format.
- The default query DSL can be mathematically expressed as a `DNF` expression. 
- The default query DSL uses IPFIX information model defined in [RFC 7012](http://tools.ietf.org/html/rfc7012). for both NetFlow v5 and IPFIX.
- The query is parsed in a `JSON` format allowing `M2M` communication.
- The query parser and execution engine are completely decoupled from one another.
- The flow processing is modular adhering to a five-stage processing pipeline.
- Each stage of the processing pipeline can be enabled/disabled at runtime.
- Compression levels for flow-traces can be configured at runtime.

### Releases

You can either fork the bleeding edge development version or download a
release from [here &rarr;](https://github.com/vbajpai/nfql/releases)

### Installation and Usage

- [Debian-based Linux &rarr;](install-debian.html)
- [Fedora Linux &rarr;](install-fedora.html)
- [Mac OS X using Homebrew &rarr;](install-osx-homebrew.html)
- [Mac OS X using Mac Ports &rarr;](install-osx-macports.html)
- [FreeBSD &rarr;](install-freebsd.html)

### Publications

[Network Flow Query Language - Design, Implementation, Performance and Applications &rarr;](http://vaibhavbajpai.com/documents/papers/proceedings/nfql-tnsm-2017.pdf)  
Vaibhav Bajpai, Jürgen Schönwälder  
TNSM 2017, XXX 2017

[NFQL: A Tool for Querying Network Flow Records &rarr;](http://www.vaibhavbajpai.com/documents/papers/proceedings/nfql-im-2013.pdf)  
Vaibhav Bajpai, Johannes Schauer, Jürgen Schönwälder  
IM 2013, Ghent, May 2013

[Flow-based Identification of Failures Caused by IPv6 Transition Mechanisms &rarr;](http://www.springerlink.com/content/2t0mv427lk824224/)  
Vaibhav Bajpai, Nikolay Melnikov, Anuj Sehgal, Jürgen Schönwälder  
AIMS 2012, Luxembourg, June 2012

[Flow Signatures of Popular Applications &rarr;](http://ieeexplore.ieee.org/xpls/abs_all.jsp?arnumber=5990668)  
Vladislav Perelman, Nikolay Melnikov, Jürgen Schönwälder  
IM 2011, Dublin, May 2011.

[Cybermetrics: User Identiﬁcation through Network Flow Analysis &rarr;](http://www.springerlink.com/content/l85hk73487086024/)  
Nikolay Melnikov, Jürgen Schönwälder
AIMS 2010, Zurich, June 2010.

[Implementation of a Stream-Based IP Flow Record Query Language &rarr;](http://www.springerlink.com/content/a565783288655j67)  
Kaloyan Kanev, Nikolay Melnikov, Jürgen Schönwälder  
AIMS 2010, Zurich, June 2010.

[Design of a Stream-Based IP Flow Record Query Language &rarr;](http://www.springerlink.com/content/j4555jj848l8q862/)    
Vladislav Marinov, Jürgen Schönwälder  
DSOM 2009, Venice, October 2009.

[Design of an IP Flow Record Query Language &rarr;](http://www.springerlink.com/content/0m76rk7653872426/)  
Vladislav Marinov, Jürgen Schönwälder  
AIMS 2008, Bremen, July 2008.


### Thesis

[NFQL Front-End Parser
&rarr;](http://cnds.eecs.jacobs-university.de/archive/bsc-2013-dmorina.pdf)  
Durim Morina, Bachelors Thesis, May 2013.

[A Complete System Integration of the Network Flow Query Language &rarr;](http://cnds.eecs.jacobs-university.de/archive/msc-2012-vbajpai.pdf)  
Vaibhav Bajpai, Masters Thesis, August 2012.

[Flowy 2.0: Fast Execution of Stream-based IP Flow Queries &rarr;](http://cnds.eecs.jacobs-university.de/archive/bsc-2011-jschauer.pdf)  
Johannes Schauer, Bachelors Thesis, May 2011.

[Cybermetrics: Identification of Users Through Network Flow Analysis &rarr;](http://cnds.eecs.jacobs-university.de/archive/msc-2010-nmelnikov.pdf)  
Nikolay Melnikov, Masters Thesis, August 2010.

[Flow Signatures of Popular Applications &rarr;](http://cnds.eecs.jacobs-university.de/archive/bsc-2010-vperelman.pdf)  
Vladislav Perelman, Bachelors Thesis, May 2010.

[Flowy improvements using MapReduce &rarr;](http://cnds.eecs.jacobs-university.de/archive/bsc-2010-pnemeth.pdf)  
Peter Nemeth, Bachelors Thesis, May 2010.

[Flowy - Network Flow Analysis Application &rarr;](http://cnds.eecs.jacobs-university.de/archive/msc-2009-kkanev.pdf)  
Kaloyan Kanev, Masters Thesis, August 2009.

[Design of an IP Flow Record Query Language &rarr;](http://cnds.eecs.jacobs-university.de/archive/msc-2009-vmarinov.pdf)  
Vladislav Marinov, Masters Thesis, August 2009.


### Talks

[NFQL: A Tool for Querying Network Flow Records
&rarr;](http://vaibhavbajpai.com/documents/talks/nfql-ietf87-2013.pdf)  
Vaibhav Bajpai, Johannes Schauer, Corneliu Claudiu Prodescu, Jürgen Schönwälder  
NetFlow/IPFIX NMRG Workshop, IETF 87, Berlin, July 2013  

[NFQL: A Tool for Querying Network Flow Records
&rarr;](http://www.vaibhavbajpai.com/documents/talks/nfql-im-2013.pdf)  
Vaibhav Bajpai, Johannes Schauer, Jürgen Schönwälder  
IM 2013, Ghent, May 2013

[Flow-based Identification of Failures Caused by IPv6 Transition Mechanisms &rarr;](http://cnds.eecs.jacobs-university.de/slides/2012-aims-ipv6transeval.pdf)  
Vaibhav Bajpai, Nikolay Melnikov, Anuj Sehgal, Jürgen Schönwälder  
AIMS 2012, Luxembourg, June 2012

[Flow signatures of Popular Applications &rarr;](http://cnds.eecs.jacobs-university.de/slides/2011-im-flow-signatures.pdf)  
Vladislav Perelman, Nikolay Melnikov, Jürgen Schönwälder  
IM 2011, Dublin, May 2011.

[Flow Signatures of Popular Applications &rarr;](http://cnds.eecs.jacobs-university.de/slides/2010-ietf-78-nmrg-app-signatures.pdf)  
Nikolay Melnikov  
78th IETF, Maastricht, July 2010.

[Cybermetrics: User Identiﬁcation through Network Flow Analysis &rarr;](http://cnds.eecs.jacobs-university.de/slides/2010-aims-cybermetrics.pdf)  
Nikolay Melnikov, Jürgen Schönwälder  
AIMS 2010, Zurich, June 2010.

[Implementation of a Stream-Based IP Flow Record Query Language &rarr;](http://cnds.eecs.jacobs-university.de/slides/2010-aims-flowy-implementation.pdf)  
Kaloyan Kanev, Nikolay Melnikov, Jürgen Schönwälder  
AIMS 2010, Zurich, June 2010.

[Design of a Stream-Based IP Flow Record Query Language &rarr;](http://cnds.eecs.jacobs-university.de/slides/2009-dsom-flow-query.pdf)    
Vladislav Marinov, Jürgen Schönwälder  
DSOM 2009, Venice, October 2009.


### Contributors

- Vaibhav Bajpai [contact@vaibhavbajpai.com](contact@vaibhavbajpai.com)  
- Johannes Schauer [j.schauer@email.de](j.schauer@email.de)  
- Corneliu Claudiu Prodescu [c.prodescu@jacobs-university.de](c.prodescu@jacobs-university.de)  
- Kaloyan Kanev [k.kanev@jacobs-university.de](k.kanev@jacobs-university.de)
- Vladislav Marinov [v.marinov@jacobs-university.de](v.marinov@jacobs-university.de)
- Vladislav Perelman [v.perelman@jacobs-university.de](v.perelman@jacobs-university.de)
- Nikolay Melnikov [n.melnikov@jacobs-university.de](n.melnikov@jacobs-university.de)
- Jürgen Schönwälder [j.schoenwaelder@jacobs-university.de](j.schoenwaelder@jacobs-university.de)

### History

Flowy was a first feature complete Python prototype of NFQL. Due to
performance problems, it was superseeded by a complete rewrite in C,
called Flowy 2.0. `nfql` extends on Flowy 2.0, making it feature
complete. The execution engine has been optimized with crispier
algorithms to make it scale to real-world sized traces. `nfql` now has
comparable execution times to contemporary flow analysis tools.

### Development Blog

A development blog discussing the internal details is [here
&rarr;](http://blog.nfql.vaibhavbajpai.com)

### License
<pre>
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:

 1. Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
</pre>
