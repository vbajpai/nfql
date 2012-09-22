---
layout: post
title: "Remote iPhone Packet Captures"
description: ""
category: 
tags: []
---
{% include JB/setup %}
Create the Remote Virtual Interface

	$ rvictl -s $UDID

The `UDID` is visible by double clicking the Serial Number in iTunes.
Run `ifconfig` to check the interface name

	$ ifconfig
	...
	rvi0: flags=3005<UP,DEBUG,LINK0,LINK1> mtu 0
	
Run Wireshark to capture the packets  

Delete the Interface

	$ rvictl -x $UDID
