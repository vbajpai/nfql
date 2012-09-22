---
layout: post
title: "Default to clang on Mac OS X"
description: ""
category: 
tags: []
---
{% include JB/setup %}
The default compiler on Mac OS X Lion is `gcc-llvm`.  
I have decided to default to `clang` to take advantage of its static analysis capabilities:

	export CC=clang
	export CPP=c++
	export CXX=c++
