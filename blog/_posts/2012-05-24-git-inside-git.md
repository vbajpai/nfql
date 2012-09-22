---
layout: post
title: "Git inside Git"
description: ""
category: 
tags: []
---
{% include JB/setup %}
I use [janus &rarr;](https://github.com/carlhuda/janus) and
[oh-my-zsh &rarr;](https://github.com/robbyrussell/oh-my-zsh/) 
to setup a part of my dotfiles configuration.

My [dotfiles &rarr;](https://github.com/vbajpai/dotfiles/) configuration
is setup as a git repository and because the aforementioned frameworks
are git repositories themselves, I was faced with the task of setting up
git inside git! Since I have used `svn externals` before, I was sure
there must be a way to do this. It turned out to be called [git
submodules &rarr;](http://git-scm.com/book/en/Git-Tools-Submodules)

To clone a submodule in the local path of your git repository -

	[root-repo] >> git submodule add <remote repository> <local path>
	
To update the submodule 

	[root-repo] >> git submodule update

A new git clone requires reinitializing the submodules.

	>> git clone <root-repo>
	[root-repo] >> git submodule init
	[root-repo] >> git submodule update 

To pull recent changes from all the submodules at once.
  
	[root-repo] >> git submodule foreach git pull origin master
