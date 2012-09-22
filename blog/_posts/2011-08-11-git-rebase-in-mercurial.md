---
layout: post
title: "Git rebase in Mercurial"
description: ""
category: 
tags: []
---
{% include JB/setup %}

The scenario is to rebase the commits of your feature branch on top of the newer updates from master branch:
To achieve it in `git`:

    [master] $ git checkout feature
    [feature] $ git rebase master

To achieve it in `hg`, enable rebase extension in `~/.hgrc`

    [...]
    [extensions]
    rebase = 

and then rebase

    [default] $ hg update feature
    [feature] $ hg rebase -d default --keepbranches
