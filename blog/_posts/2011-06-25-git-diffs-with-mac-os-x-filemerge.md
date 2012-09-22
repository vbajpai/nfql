---
layout: post
title: "Git diffs with Mac OS X FileMerge"
description: ""
category: 
tags: []
---
{% include JB/setup %}
Add a function for `git difftool` in your `.zshrc`

	gd () {
		git difftool -t opendiff $1
	}

Diff using `gd()`

	$ gd $file

