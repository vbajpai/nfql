---
layout: post
title: "Rasdaman on Ubuntu"
description: ""
category: 
tags: []
---
{% include JB/setup %}
> rasdaman  is a universal, domain-independent, array 
DBMS  for  multidimensional  arrays  of  arbitrary  size  and 
structure.  A  declarative,  SQL-based  array  query  language 
offers  flexible  retrieval  and  manipulation.  Efficient  server- 
based  query  evaluation  is  enabled  by  an  intelligent 
optimizer  and  a  streamlined  storage  architecture  based on 
flexible  array tiling  and compression. 

for more information visit the [rasdaman homepage →
](http://www.rasdaman.com/)



Create a `rasdaman` user account

Install Dependencies 

    $ sudo apt-get install git-core g++ autoconf libtool gawk bison
      postgresql libecpg-dev libncurses5-dev libhdf4g-dev flex
      libpng12-dev libjpeg-dev libnetpbm10-dev libreadline-dev
      libtiff-dev make build-essential openjdk-6-jdk doxygen

... it also sets up `PostgreSQL` by default 

    - Adding user postgres to group ssl-cert ...
    - Setting up postgresql-8.4 (8.4.5-0ubuntu10.10) ...
    - Creating new cluster (configuration: ...
    - Configuring postgresql.conf to use port 5432...
    - Starting PostgreSQL 8.4 database server ...
    - Done

Clone `rasdaman` source

    $ git clone git://kahlua.eecs.jacobs-university.de/rasdaman.git

Compile and Install from source

    $ ./autoreconf
    $ ./configure
    $ sudo make 
    $ sudo make install 

Add `rasdaman` and `PostgreSQL` binaries to `PATH`

    $ export PATH=$PATH:$RASDAMAN/bin
    $ export PATH=$PATH:/usr/lib/postgresql/8.4/bin

Either edit `/etc/postgresql/8.4/main/postgresql.conf` and change port from 5432 to 5433 or:

	$ sudo pg_dropcluster --stop 8.4 main

Change ownership of `PostgreSQL` for user `rasdaman`

    $ sudo chown -R rasdaman /var/run/postgresql

Initialize and Start a new Database Cluster

    $ initdb -D $DBDATA
    $ pg_ctl -D $DBDATA start

where `$DBDATA` is the location where the cluster will be stored.

Create the database on the cluster and start `rasdaman`

    $ bash create_db.sh
    $ bash start_rasdaman.sh
    
check the logs to make sure the server daemon started successfully.

Run the insert script

    $ insertdemo.sh localhost 7001 
      $RASDAMAN/share/rasdaman/images rasadmin rasadmin

Test `rasdaman`

    $ rasql &ndash;q "select r from RAS_COLLECTIONNAMES as r" --out string


Pointers:

Databases: `RASBASE` and `petascopedb`    
Superusers: `rasdaman` and `petauser`    
User: `rasguest`    
