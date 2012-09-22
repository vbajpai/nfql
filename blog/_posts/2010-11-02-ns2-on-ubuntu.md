---
layout: post
title: "ns2 on Ubuntu"
description: ""
category: 
tags: []
---
{% include JB/setup %}

Add [Wouter Horré's](https://launchpad.net/~wouterh/+archive/ppa) PPA

	deb http://ppa.launchpad.net/wouterh/ppa/ubuntu YOUR_UBUNTU_VERSION_HERE main 
	deb-src http://ppa.launchpad.net/wouterh/ppa/ubuntu YOUR_UBUNTU_VERSION_HERE main 

Install `ns`

	$ sudo apt-get install ns nam xgraph
