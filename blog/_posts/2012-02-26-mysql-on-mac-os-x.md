---
layout: post
title: "MySQL on Mac OS X"
description: ""
category: 
tags: []
---
{% include JB/setup %}
Install MySQL using Homebrew

	$ brew install mysql
	
Set up databases to run with your user with:

    $ unset TMPDIR
    $ mysql_install_db --verbose \
                       --user=`whoami` \
                       --basedir="$(brew --prefix mysql)" \ 
                       --datadir=/usr/local/var/mysql \
                       --tmpdir=/tmp
    
Start MySQL Daemon

	$ mysql.server start
	
Connect

	$ mysql -uroot
	[…]	
	mysql> 		
	
Stop MySQL Daemon

	$ mysql.server stop
