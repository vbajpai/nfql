---
layout: post
title: "Basic Graph Plots in MATLAB"
description: ""
category: 
tags: []
---
{% include JB/setup %}
Plot `x[1]` v `x[2]`

	plot(x(:,1), x(:,2)

Add legends for each plot

	legend('foo' , 'bar', ...)

Set axis extremas

	axis([Xmin, Xmax, Ymin, Ymax])

Label the axises

	xlabel('foo')
	ylabel('bar')

Add a title to the graph

	title('foobar')
