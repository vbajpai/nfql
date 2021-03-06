`nfql v0.7.1`: Ready for Packaging

    $ git show v0.7.1

    tag v0.7.1
    Tagger: Vaibhav Bajpai <contact@vaibhavbajpai.com>
    Date:   Sat Dec 28 15:53:39 2013 +0100
    Commit bff58ec77ca8a7a84c7f2ae8a9dbc63e0d22dc1c

    * proper nfql binary and man page installation at standard locations.
    * updated README.md with installation instructions.

`nfql v0.7`: It supports IPFIX!

    $ git show v0.7

    tag v0.7
    Tagger: Vaibhav Bajpai <contact@vaibhavbajpai.com>
    Date:   Wed Dec 25 10:42:24 2013 +0100
    Commit 0e7e79cf2f9e46d85a29810d220a707a65221124

    * execution engine supports IPFIX
      - the query DSL now uses IPFIX entity names and datatypes [RFC 7012].
      - the IPFIX I/O processing is supported using libfixbuf.
      - the read and written IPFIX flows are RFC 5655 compatible.
      - added --ipfix/-I in bin/engine -h
      - updated installation instructions for Mac OS X homebrew.
      - updated installation instructions for Debian and Fedora.
    * IPFIX to NetFlow v5 backward compatibility layer:
      - allows usage of IPFIX entitiy names and datatypes for NetFlow v5 flows.
      - query DSL only supports IPFIX entity name and datatypes as default.
    * deprecated and removed flowy parser codebase
    * removed all references to flowy
    * the executable binary is now called nfql (and not engine)
    * removed all references to engine with nfql
    * added a nfql.1 man page, updated README.md with reading instructions.
    * resolved issues:
      - fixed asprintf(...) unchecked return warnings

`nfql v0.6`: It is fast!

    $ git show v0.6

    tag v0.6
    Tagger: Vaibhav Bajpai <contact@vaibhavbajpai.com>
    Date:   Mon Dec 23 15:20:17 2013 +0100
    Commit 96eb75b018f5b8e68ac923bd17c3e5777f9ba6b8

    * engine understands the NFQL DSL represented as a DNF expression.
      - using clauses and terms instead of modules and rulesets.
      - multiple clauses supporting or expressions in each stage of the
        pipeline.
      - aggregation substage within grouper can only have a single clause.
    * JSON query can disable the stages at RUNTIME:
      - added conditional MACROS for each stage of the processing pipeline.
      - no filter in query => no filter in engine.
      - no grouper in query => no grouper in engine.
      - no groupaggregations in query => no groupaggregations in engine.
      - no groupfilter in query => no groupfilter in engine.
      - no merger in query => no merger and no ungrouper in engine.
      - no ungrouper in query => no ungrouper in engine.
    * performance optimizations:
      - reading the trace after successfully parsing the query in struct
        flowquery.
      - inline writes: writing intermediates results to files as soon as
        processed.
      - faster filter:
        * inline filtering while reading the trace.
        * saving the filtered recordset only once for each branch.
      - faster grouper:
        * nested binary search by sorting on each term of the clauset
      - don't call rule assignment functions for each record.
    * --dirpath option to save the results as flow-tools files in given
      dirpath.
    * --zlevel option to change flow-tools trace compression level (default:
      5).
    * using STDIN_FILENO instead of 0 (describes standard input).
    * using -O2 compiler flag to increase level of GCC optimization.
    * documentation:
      - added installation instructions for FreeBSD and FEDORA.
      - updated bin/engine -h, tracefile must be in flow-tools format.
      - replaced all references of flowy with f.
      - added installation instructions using MACPORTS for Mac OS X.
      - froze json-c library dependency to v0.10 (v0.11 now uses libjson-c
        name)
    * resolved issues:
      - no segfaults with only filter enabled.
      - successful compilation on removing any conditional macro.
      - removed duplicate aggregation stages in JSON queries
      - operation type of relative comparisons is RULE_REL (not RULE_ABS)
      - changed the datatype of zlevel from uint_8 to int_8 (gcc v4.2.1
        issue)
      - removed duplicate const decl on one declaration statement (clang
        issue)

`nfql v0.5`: It is verifiable!

    $ git show v0.5

    tag v0.5
    Tagger: Vaibhav Bajpai <contact@vaibhavbajpai.com>
    Date:   Wed Jul 11 10:33:58 2012 +0200
    Commit 8d2f9b374a1e104e97398de47542cd5c0479a0dc

    * better engine usage on run.
    * evaluation of query ruleset lengths at RUNTIME.
    * python pipeline module to encapsulate pipeline stage classes.
    * painless parser installation using make.
    * parser installation instructions on debian/ubuntu and osx.
    * regression test-suite for the execution engine.
    * silk installation and usage instructions.
    * instructions to convert flow-tools traces to silk.
    * automated benchmarking suite.
    * resolved issues:
      * no segfault on srcIP = dstIP in a grouper rule.
      * no segfault when no grouper rules are defined.


`nfql v0.4`: It is portable!

	$ git show v0.4
	
	tag v0.4
	Tagger: Vaibhav Bajpai <contact@vaibhavbajpai.com>
	Date:   Fri May 18 15:07:42 2012 +0200
	Commit 00c17385e37dd944c9139205a5eb3660c707858a	
	
	*  _GNU_SOURCE feature test MACRO and -std=c99
	*  (__FreeBSD, __APPLE__) and __linux MACROS around qsort_r(…)
	*  reverted to a flat source structure for the CMake build process.
	*  CMake custom command to call a script to create auto-generated sources and headers.
	*  CMake custom command to call a scripts in queries/ to save sample JSON queries in examples/
	*  Makefile to automate invocation of CMake commands.
	*  installation instruction for Ubuntu.
	*  installation instruction for Mac OS X.


`nfql v0.3`: It is flexible!
  
	$ git show v0.3
	
	tag v0.3
	Tagger: Vaibhav Bajpai <contact@vaibhavbajpai.com>
	Date:   Wed May 16 18:25:22 2012 +0200
	Commit 1c323fa66b9aaaad56ad7c4127b8d187eaf4ec0c
	
	* complete query is read at RUNTIME using JSON-C
	* JSON queries are generated using python scripts
	* glibc backtrace(...) to print the back trace on errExit(...)
	* gracefully exiting when trace cannot be read
	* gracefully exiting when JSON query cannot be parsed
	* branch thread returns EXIT_FAILURE if either stage returns NULL
	* branch thread returns EXIT_SUCCESS on normal exit
	* each stage proceeds only when previous returned results
	* flow-cat ... | flowy-engine $QUERY -		


`nfql v0.2`: It is robust!
	
	$ git show v0.2
	
	tag v0.2
	Tagger: Vaibhav Bajpai <contact@vaibhavbajpai.com>
	Date:   Wed Apr 18 13:24:16 2012 +0200    
	Commit 2c571f80cd076172cbd00ef7f9976b88cb44b425
    
	* complete engine refactor.
	* complete engine profiling (no memory leaks).
	* issues closed:
		- greedily deallocating non-filtered records in `O(n)` before `merger(…)`.
	    - resolved a grouper segfault when NO records got filtered.
	    - all records are grouped into 1 group when no grouping rule specified.
	    - aggregation on common fields touched by filter/grouper rules is ignored.
	    - no `uintX_t` assumptions for field offsets.
	    - rules are clubbed together and assigned using a loop.
	    - function parameters are as minimum as required.
	    - function parameters are safe using `[const]` ptr and ptr to `[const]`.
	    - lazy `rule->func(…)` assignment when the stage is entered.	Summary: (since after `v0.2`)
	    

`nfql v0.1`: It works!

	$ git show v0.1
	
	tag v0.1		
	Tagger: Vaibhav Bajpai <contact@vaibhavbajpai.com>
	Date:   Fri Apr 6 19:07:49 2012 +0200	
	Commit a8a67a13aa07f671d21d062537a2ef17e58dcc07
	…
	
	* reverse engineered parser to generate UML.
	* froze requirements to allow single step installation of the python parser.
	* doxygen documentation of the engine.
	* prelim JSON parsing framework for the parser and engine to spit and parse the JSON queries.
	* replaced GNU99 extensions dependent code with c99.
	* resolved numerous segfaults in grouper and merger.
	* generated group aggregations as a separate (cooked) NetFlow v5 record.
	* flexible group aggregations with no uintX_t assumptions on field offsets.
	* first-ever group filter implementation.
	* reorganized the src/ directory structure
	* enabled multiple verbosity levels in the engine.
	* first-ever merger implementation.
	* flexible filters and group filters with no uintX_t assumptions on field offsets.
	* first-ever ungrouper implementation.
  
`nfql v0.0`: It is a start!

	$ git show v0.0

	tag v0.0
	Tagger: Vaibhav Bajpai <contact@vaibhavbajpai.com>
	Date:   Thu May 17 10:48:02 2012 +0200
	Commit 8cb309c8a956c99e6b1494eddb601c8f6a520696
 
	* read flow-records into memory
	* rewrite of the execution pipeline in C (non functional)
	* efficient rule processing with dedicated function pointers
	* reduced grouper complexity using qsort(...) and bsearch(...)
	* concerns
	        - flow query is currently hardcoded in pipeline structs
	        - functions assume specific uintX_t offsets
	        - numerous grouper segfaults
	        - no group filter
	        - commented out merger (segfaults when uncommented)
	        - no ungrouper
	        - code dependent on GNU99 extensions
	        - some headers are missing include guards
	        - unused extraneous source files and headers

:::::::::::::::::::

    * execution engine supports IPFIX
      - the query DSL now uses IPFIX entity names and datatypes [RFC 7012].
      - the IPFIX I/O processing is supported using libfixbuf.
      - the read and written IPFIX flows are RFC 5655 compatible.
      - added --ipfix/-I in bin/engine -h
      - updated installation instructions for Mac OS X homebrew.
      - updated installation instructions for Debian and Fedora.
    * IPFIX to NetFlow v5 backward compatibility layer:
      - allows usage of IPFIX entitiy names and datatypes for NetFlow v5 flows.
      - query DSL only supports IPFIX entity name and datatypes as default.
    * deprecated and removed flowy parser codebase
    * removed all references to flowy
    * the executable binary is now called nfql (and not engine)
    * removed all references to engine with nfql
    * added a nfql.1 man page, updated README.md with reading instructions.
    * resolved issues:
      - fixed asprintf(...) unchecked return warnings

`nfql v0.6`: It is fast!

    $ git show v0.6

    tag v0.6
    Tagger: Vaibhav Bajpai <contact@vaibhavbajpai.com>
    Date:   Mon Dec 23 15:20:17 2013 +0100

    * engine understands the NFQL DSL represented as a DNF expression.
      - using clauses and terms instead of modules and rulesets.
      - multiple clauses supporting or expressions in each stage of the
        pipeline.
      - aggregation substage within grouper can only have a single clause.
    * JSON query can disable the stages at RUNTIME:
      - added conditional MACROS for each stage of the processing pipeline.
      - no filter in query => no filter in engine.
      - no grouper in query => no grouper in engine.
      - no groupaggregations in query => no groupaggregations in engine.
      - no groupfilter in query => no groupfilter in engine.
      - no merger in query => no merger and no ungrouper in engine.
      - no ungrouper in query => no ungrouper in engine.
    * performance optimizations:
      - reading the trace after successfully parsing the query in struct
        flowquery.
      - inline writes: writing intermediates results to files as soon as
        processed.
      - faster filter:
        * inline filtering while reading the trace.
        * saving the filtered recordset only once for each branch.
      - faster grouper:
        * nested binary search by sorting on each term of the clauset
      - don't call rule assignment functions for each record.
    * --dirpath option to save the results as flow-tools files in given
      dirpath.
    * --zlevel option to change flow-tools trace compression level (default:
      5).
    * using STDIN_FILENO instead of 0 (describes standard input).
    * using -O2 compiler flag to increase level of GCC optimization.
    * documentation:
      - added installation instructions for FreeBSD and FEDORA.
      - updated bin/engine -h, tracefile must be in flow-tools format.
      - replaced all references of flowy with f.
      - added installation instructions using MACPORTS for Mac OS X.
      - froze json-c library dependency to v0.10 (v0.11 now uses libjson-c
        name)
    * resolved issues:
      - no segfaults with only filter enabled.
      - successful compilation on removing any conditional macro.
      - removed duplicate aggregation stages in JSON queries
      - operation type of relative comparisons is RULE_REL (not RULE_ABS)
      - changed the datatype of zlevel from uint_8 to int_8 (gcc v4.2.1
        issue)
      - removed duplicate const decl on one declaration statement (clang
        issue)

`nfql v0.5`: It is verifiable!

    $ git show v0.5

    tag v0.5
    Tagger: Vaibhav Bajpai <contact@vaibhavbajpai.com>
    Date:   Wed Jul 11 10:33:58 2012 +0200
    Commit 8d2f9b374a1e104e97398de47542cd5c0479a0dc

    * better engine usage on run.
    * evaluation of query ruleset lengths at RUNTIME.
    * python pipeline module to encapsulate pipeline stage classes.
    * painless parser installation using make.
    * parser installation instructions on debian/ubuntu and osx.
    * regression test-suite for the execution engine.
    * silk installation and usage instructions.
    * instructions to convert flow-tools traces to silk.
    * automated benchmarking suite.
    * resolved issues:
      * no segfault on srcIP = dstIP in a grouper rule.
      * no segfault when no grouper rules are defined.


`nfql v0.4`: It is portable!

	$ git show v0.4
	
	tag v0.4
	Tagger: Vaibhav Bajpai <contact@vaibhavbajpai.com>
	Date:   Fri May 18 15:07:42 2012 +0200
	Commit 00c17385e37dd944c9139205a5eb3660c707858a	
	
	*  _GNU_SOURCE feature test MACRO and -std=c99
	*  (__FreeBSD, __APPLE__) and __linux MACROS around qsort_r(…)
	*  reverted to a flat source structure for the CMake build process.
	*  CMake custom command to call a script to create auto-generated sources and headers.
	*  CMake custom command to call a scripts in queries/ to save sample JSON queries in examples/
	*  Makefile to automate invocation of CMake commands.
	*  installation instruction for Ubuntu.
	*  installation instruction for Mac OS X.


`nfql v0.3`: It is flexible!
  
	$ git show v0.3
	
	tag v0.3
	Tagger: Vaibhav Bajpai <contact@vaibhavbajpai.com>
	Date:   Wed May 16 18:25:22 2012 +0200
	Commit 1c323fa66b9aaaad56ad7c4127b8d187eaf4ec0c
	
	* complete query is read at RUNTIME using JSON-C
	* JSON queries are generated using python scripts
	* glibc backtrace(...) to print the back trace on errExit(...)
	* gracefully exiting when trace cannot be read
	* gracefully exiting when JSON query cannot be parsed
	* branch thread returns EXIT_FAILURE if either stage returns NULL
	* branch thread returns EXIT_SUCCESS on normal exit
	* each stage proceeds only when previous returned results
	* flow-cat ... | flowy-engine $QUERY -		


`nfql v0.2`: It is robust!
	
	$ git show v0.2
	
	tag v0.2
	Tagger: Vaibhav Bajpai <contact@vaibhavbajpai.com>
	Date:   Wed Apr 18 13:24:16 2012 +0200    
	Commit 2c571f80cd076172cbd00ef7f9976b88cb44b425
    
	* complete engine refactor.
	* complete engine profiling (no memory leaks).
	* issues closed:
		- greedily deallocating non-filtered records in `O(n)` before `merger(…)`.
	    - resolved a grouper segfault when NO records got filtered.
	    - all records are grouped into 1 group when no grouping rule specified.
	    - aggregation on common fields touched by filter/grouper rules is ignored.
	    - no `uintX_t` assumptions for field offsets.
	    - rules are clubbed together and assigned using a loop.
	    - function parameters are as minimum as required.
	    - function parameters are safe using `[const]` ptr and ptr to `[const]`.
	    - lazy `rule->func(…)` assignment when the stage is entered.	Summary: (since after `v0.2`)
	    

`nfql v0.1`: It works!

	$ git show v0.1
	
	tag v0.1		
	Tagger: Vaibhav Bajpai <contact@vaibhavbajpai.com>
	Date:   Fri Apr 6 19:07:49 2012 +0200	
	Commit a8a67a13aa07f671d21d062537a2ef17e58dcc07
	…
	
	* reverse engineered parser to generate UML.
	* froze requirements to allow single step installation of the python parser.
	* doxygen documentation of the engine.
	* prelim JSON parsing framework for the parser and engine to spit and parse the JSON queries.
	* replaced GNU99 extensions dependent code with c99.
	* resolved numerous segfaults in grouper and merger.
	* generated group aggregations as a separate (cooked) NetFlow v5 record.
	* flexible group aggregations with no uintX_t assumptions on field offsets.
	* first-ever group filter implementation.
	* reorganized the src/ directory structure
	* enabled multiple verbosity levels in the engine.
	* first-ever merger implementation.
	* flexible filters and group filters with no uintX_t assumptions on field offsets.
	* first-ever ungrouper implementation.
  
`nfql v0.0`: It is a start!

	$ git show v0.0

	tag v0.0
	Tagger: Vaibhav Bajpai <contact@vaibhavbajpai.com>
	Date:   Thu May 17 10:48:02 2012 +0200
	Commit 8cb309c8a956c99e6b1494eddb601c8f6a520696
 
	* read flow-records into memory
	* rewrite of the execution pipeline in C (non functional)
	* efficient rule processing with dedicated function pointers
	* reduced grouper complexity using qsort(...) and bsearch(...)
	* concerns
	        - flow query is currently hardcoded in pipeline structs
	        - functions assume specific uintX_t offsets
	        - numerous grouper segfaults
	        - no group filter
	        - commented out merger (segfaults when uncommented)
	        - no ungrouper
	        - code dependent on GNU99 extensions
	        - some headers are missing include guards
	        - unused extraneous source files and headers
