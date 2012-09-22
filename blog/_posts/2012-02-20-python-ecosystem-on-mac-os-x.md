---
layout: post
title: "Python Ecosystem on Mac OS X"
description: ""
category: 
tags: []
---
{% include JB/setup %}
Install Homebrew and Prioritize it 

	$ ruby -e "$(curl -fsS https://raw.github.com/gist/323731)"
	$ export PATH=/usr/local/bin:$PATH

Install Python

	$ brew install python --framework

Put `easy_install` in PATH

	export PATH=/usr/local/share/python:$PATH

Install `pip` and Upgrade it to latest version available in PyPI

	$ easy_install pip
	$ pip install pip --upgrade

Using `pip` to install (or uninstall) and upgrade (or degrade) packages

    $ pip install $PACKAGE
    $ pip install $PACKAGE==$VERSION
    $ pip install $PACKAGE --upgrade
    $ pip install $PACKAGE==$VERSION --upgrade
	$ pip uninstall $PACKAGE

Using `pip` to search (or list installed) packages

	$ pip search $PACKAGE
	$ pip freeze

Installing IPython and BPython

	$ pip install bpython
	$ pip install ipython

Install Virtualenv and VirtualenvWrapper

	$ pip install virtualenv
	$ pip install virtualenvwrapper

VirtualenvWrapper configuration for your `.bashrc`

	if [ `id -u` != '0' ]; then	
	  export VIRTUALENV_USE_DISTRIBUTE=1        
	  export WORKON_HOME=$HOME/.virtualenvs	
	  source /usr/local/share/python/virtualenvwrapper.sh
	fi

Using VirtualenvWrapper

	$ mkvirtualenv foo
    
	(foo) $ which python
	$HOME/.virtualenvs/foo/bin/python

	(foo) $ which pip
	$HOME/.virtualenvs/foo/bin/pip

	(foo) $ pip freeze
	distribute==0.6.24
	wsgiref==0.1.2

	(foo) $ deactivate

	$ which python
	/usr/local/bin/python

	$ which pip
	/usr/local/share/python/pip

	$ pip freeze
	bpython==0.10.1
	distribute==0.6.24
	ipython==0.12
	virtualenv==1.7.1.2
	virtualenvwrapper==3.0

	$ lsvirtualenv
	foo

	$ rmvirtualenv foo

Using Virtualenv to freeze current state of environment packages

	(foo) $ pip freeze > requirements.txt
	(bar) $ pip install -r requirements.txt

Resources:

- [Python Ecosystem - An Introduction &rarr;](http://mirnazim.org/writings/python-ecosystem-introduction/)  
- [The Hitchhiker’s Guide to Python! &rarr;](http://docs.python-guide.org/en/latest/index.html)
