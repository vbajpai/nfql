---
layout: post
title: "Dropbox on Ubuntu Server"
description: ""
category: 
tags: []
---
{% include JB/setup %}
Download the latest tarball: [x86](http://www.dropbox.com/download/?plat=lnx.x86) or [x86-64](http://www.dropbox.com/download/?plat=lnx.x86_64)

Extract the tarball and relocate it.

    $ tar -zxvf $TARBALL
    $ mv .dropbox-dist ~/

Download the Dropbox CLI and put it in your `PATH`

    $ curl -O http://linux.dropbox.com/packages/dropbox.py
    $ chmod +x dropbox.py
    $ mv dropbox.py ~/.bin/dropbox
    $ dropbox

	Dropbox command-line interface

	commands:

	Note: use dropbox help <command> to view usage for a specific command.

	 status       get current status of the dropboxd
	 help         provide help
	 puburl       get public url of a file in your dropbox
	 stop         stop dropboxd
	 running      return whether dropbox is running
	 start        start dropboxd
	 filestatus   get current sync status of one or more files
	 ls           list directory contents with current sync status
	 autostart    automatically start dropbox at login
	 exclude      ignores/excludes a directory from syncing

Run the Dropbox Daemon; and goto the URL to link the machine and start syncing!

    [$HOME/.dropbox-dist]$ ./dropboxd
    This client is not linked to any account... Please visit $URL to link this machine.     

Download the [init.d script](http://wiki.dropbox.com/TipsAndTricks/TextBasedLinuxInstall/UbuntuStartup), change `DROPBOX_USERS` to include your `username` and save the script at `/etc/init.d/dropbox`.

Add execute permissions: 

    $ sudo chmod +x /etc/init.d/dropbox

Add to the very top of the script:

	### BEGIN INIT INFO
	# Provides:          dropbox
	# Required-Start:    $local_fs $remote_fs $network $syslog $named
	# Required-Stop:     $local_fs $remote_fs $network $syslog $named
	# Default-Start:     2 3 4 5
	# Default-Stop:      0 1 6
	# X-Interactive:     false
	# Short-Description: dropbox service
	### END INIT INFO

Update

    $ sudo update-rc.d dropbox defaults
      
Reboot and then test:

    $ dropbox status    
    Idle