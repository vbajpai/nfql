NFQL
--------------------  
- - -

Network Flow Query Language `(NFQL)` is our in-house stream-based flow-query 
language. The query language allows to describe patterns in `NetFlow v5`
records in a declarative and orthogonal fashion, making it easy to read and
flexible enough to describe complex relationships among a given set of flows. 
It can process flow-records, aggregate them into groups, apply absolute (or 
relative) filters and invoke Allen interval algebra rules. 

`F (v2)` is the execution engine of `NFQL`. The execution engine is written
in `C`. It is a complete rewrite of the execution module of the first 
prototype implementation, `Flowy`. `Flowy` is written in `Python` and in 
addition to the execution module, also has a validator module to parse and 
check the syntactical structure of the input query. The convergence of the 
`Flowy`'s validator module to `F (v2)` is foreseen in the near future.


General Usage
-------------
- - - 

Instructions to build and use `F(v2)` are at `engine/README.md`



Development Instructions
------------------------
- - -

Developers interested to try out and build `Flowy` can check `parser/README.md`



Contributors
------------
- - - 

<u>NFQL Implementations:</u>

`F (v2)`: Vaibhav Bajpai <contact@vaibhavbajpai.com>, 2012  
`F (v1)`: Johannes 'josch' Schauer <j.schauer@email.de>, 2011  
`Flowy`: Kaloyan Kanev <k.kanev@jacobs-university.de>, 2009  

<u>NFQL Design:</u>

 
 Vladislav Marinov <v.marinov@jacobs-university.de>, 2009  
