---
layout: post
title: "Removing Tracked Content from an Updated .hgignore"
description: ""
category: 
tags: []
---
{% include JB/setup %}
A change in the `.hgignore` file does NOT automatically remove the currently tracked items from the `hg` repository. A manual `$ hg remove` is  required. However if the recent  ignore pattern involves several files kept in fragmented place of  the repository, a manual remove may involve some work. Mercurial' support for a functional language to select a set of files comes to the rescue and it even has a special predicate to specify the current `.hgignore` file pattern to get this done in one simple command - 

    $ hg remove -f "set:hgignore()"