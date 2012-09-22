---
layout: post
title: "WPA on Ubuntu Server"
description: ""
category: 
tags: []
---
{% include JB/setup %}
Find your network's PSK string

    $ wpa_passphrase $SSID $PSK

where `$SSID` is the network name and `$PSK` is WPA password

Edit `/etc/wpa_supplicant.conf`

    ctrl_interface=/var/run/wpa_supplicant
    #ap_scan=2
    network={
       ssid="$SSID"
       scan_ssid=1
       proto=WPA RSN
       key_mgmt=WPA-PSK
       pairwise=CCMP TKIP
       group=CCMP TKIP
       psk=$PSKSTRING
    }

where `$SSID` is the network name and `$PSKSTRING` is from step1

Edit `/etc/network/interfaces`

    auto wlan0
    iface wlan0 inet static
    address $IPADDR
    netmask 255.255.255.0
    wireless-essid $SSID
    gateway 192.168.1.1
    pre-up wpa_supplicant -B -Dwext -iwlan0 -c/etc/wpa_supplicant.conf
    post-down killall -q wpa_supplicant

where `$SSID` is the network name and `$IPADDR` is `192.168.1.x`