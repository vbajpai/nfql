---
layout: post
title: "Exporting a Mercurial Repository to Git"
description: ""
category: 
tags: []
---
{% include JB/setup %}
Clone [hg-fast-export &rarr;](http://repo.or.cz/w/fast-export.git)

    $ git clone git://repo.or.cz/fast-export.git

Put `hg-fast-export` in your `PATH`

Initialize a `Git` repository: 

    $ git init foo

Export `hg` repository: 

    [foo] $ hg-fast-export.sh -r bar

  where `bar` is the path to your `hg` repository.

Checkout `master`: 

    [foo] $ git checkout