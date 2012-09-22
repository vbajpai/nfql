---
layout: post
title: "Slam6D on Mac OS X"
description: ""
category: 
tags: []
---
{% include JB/setup %}
Prepare for Homebrew

- Install Xcode from the Mac App Store (Some Formulae require Xcode)
- Install [Java Developer Update](http://support.apple.com/kb/DL1421) (CMake tries to build the Java bindings)    

Install Homebrew

    $ /usr/bin/ruby -e "$(curl -fsSL https://raw.github.com/gist/323731)"

Install CMake

    $ brew install cmake

Install Boost Libraries

    $ brew install boost

Checkout [3DTK &rarr;](http://slam6d.sourceforge.net/)

    $ svn checkout https://slam6d.svn.sourceforge.net/svnroot/slam6d/trunk slam6d

Compile Slam6D

    [slam6d] $ make

Run Slam6D

    [slam6d] $ bin/slam6D dat

Run the Show Program

    [slam6d] $ bin/show dat

![Slam6D on Mac OS X Lion](http://i.imgur.com/iOrox.png)

