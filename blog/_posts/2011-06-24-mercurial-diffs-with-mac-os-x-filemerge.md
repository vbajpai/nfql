---
layout: post
title: "Mercurial diffs with Mac OS X FileMerge"
description: ""
category: 
tags: []
---
{% include JB/setup %}
Create `opendiff-w` script in `/usr/bin`:

    #!/bin/sh
    # opendiff returns immediately, without waiting for FileMerge to exit.
    # Piping the output makes opendiff wait for FileMerge.
    opendiff "$@" | cat

Give execute permissions to the script: `$ chmod +x /usr/bin/opendiff-w`

Edit `~/.hgrc`

    [extensions]
    hgext.extdiff = 

    [extdiff]
    cmd.interdiff = hg-interdiff
    cmd.opendiff = opendiff-w

    [merge-tools]
    filemerge.executable = opendiff-w
    filemerge.args = $local $other -ancestor $base -merge $output         

Add a function for `hg opendiff` in your `.zshrc`

	hgd () {
		hg opendiff $1
	}

Diff using `hgd()`

	$ hgd $file

