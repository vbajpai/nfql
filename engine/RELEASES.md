`F (v2.5):` It is verifiable!

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


`F (v2.4):` It is portable!

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


`F (v2.3):` It is flexible!
  
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


`F (v2.2):` It is robust!
	
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
	    

`F (v2.1):` It works!

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
  
`F (v2.0)`

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
