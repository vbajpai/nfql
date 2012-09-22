---
layout: post
title: "zsh on Ubuntu"
description: ""
category: 
tags: []
---
{% include JB/setup %}
Check your current shell: 

	$ echo $SHELL

Check the available shells:

	$ cat /etc/shells

Install `zsh` (if not available): 

	$ sudo apt-get install zsh

Switch to `zsh`:

	$ chsh -s bin/zsh

Logout and Login back

