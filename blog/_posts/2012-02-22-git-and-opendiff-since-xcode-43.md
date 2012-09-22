---
layout: post
title: "Git and opendiff since XCode 4.3"
description: ""
category: 
tags: []
---
{% include JB/setup %}
Apple released Xcode 4.3 on February 16, 2012.

Xcode 4.3 now bundles as a single app and recommends to remove older
versions of Xcode during the installation.  The `/Developer` directory
is eliminated in the process. As a result, `git` using `opendiff` to
will no longer work after the installation.

	$ gd $SOURCE
	Error: No developer directory found at /Developer. [...]

The `/Developer` directory is not gone, but now bundled within the
`Xcode.app`.

Switching the path will do the trick to get it back to normal.

	$ sudo /usr/bin/xcode-select -switch /Applications/Xcode.app/Contents/Developer/
	$ gd $SOURCE

Resources:

- [Mac Rumors - Apple Releases Xcode 4.3, Now Offered as App Instead of Installer &rarr;](http://www.macrumors.com/2012/02/16/apple-releases-xcode-4-3-now-offered-as-app-instead-of-installer/)  
- [iOS Developer Library - New Features in Xcode 4.3 &rarr;](https://developer.apple.com/library/ios/#documentation/DeveloperTools/Conceptual/WhatsNewXcode/Articles/xcode_4_3.html)
