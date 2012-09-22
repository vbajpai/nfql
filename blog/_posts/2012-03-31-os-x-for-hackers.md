---
layout: post
title: "OS X for Hackers"
description: ""
category: 
tags: []
---
{% include JB/setup %}
I found some useful tips from [OSX for Hackers &rarr;](https://gist.github.com/2260182)

	echo "Enable subpixel font rendering on non-Apple LCDs"
	defaults write NSGlobalDomain AppleFontSmoothing -int 2

    echo "Enable AirDrop over Ethernet and on unsupported Macs running Lion"
    defaults write com.apple.NetworkBrowser BrowseAllInterfaces -bool true
    
	echo "Use current directory as default search scope in Finder"
	defaults write com.apple.finder FXDefaultSearchScope -string "SCcf"    