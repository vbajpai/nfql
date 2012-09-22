---
layout: post
title: "Provisioning a VM from Command Line using Vagrant"
description: ""
category: 
tags: []
---
{% include JB/setup %}
![vagrant logo](http://ajohnstone.com/wp-content/uploads/2011/12/vagrant_chilling.png)

> Web developers use virtual environments every day with their web applications. From EC2 and Rackspace Cloud to specialized 
> solutions such as EngineYard and Heroku, virtualization is the tool of choice for easy deployment and infrastructure management. 
> Vagrant aims to take those very same principles and put them to work in the heart of the application lifecycle. By providing easy 
> to configure, lightweight, reproducible, and portable virtual machines targeted at development environments, Vagrant helps 
> maximize the productivity and flexibility of you and your team.

Here is a quick getting started with Vagrant:  

Install [Oracle VirtualBox &rarr;](https://www.virtualbox.org/wiki/Downloads)

Install [Vagrant &rarr;](http://downloads.vagrantup.com/)

Add a [Base Box &rarr;](http://vagrantbox.es/)

	@local: $ vagrant box add $BOXNAME $BOXURL
	
where `$BOXNAME` is a box name and `$BOXURL` is a url to a `.box` file.  
The boxes are saved in `~/.vagrant.d/boxes`
	
[Additional Commands] List and/or Remove boxes:
	
	@local: $ vagrant box list
	@local: $ vagrant box remove $BOXNAME
			
`remove` removes the .box file from `~/.vagrant.d/boxes`
		
Initalize a VM in a project.

	@local: $ git init foo/
		
	@local: [foo] $ vagrant init $BOXNAME
		
`init` creates a `Vagrantfile` in the root of the project that describe the VM characteristics.
		
[Additional Commands] Destroy the initialized VM:
	
	@local: [foo] $ vagrant destroy
			
Start the VM and remotely login.

	@local: [foo] $ vagrant up
	@local: [foo] $ vagrant ssh
		
[Additional Commands] Suspend and Resume the VM:
	
	@local: [foo] $ vagrant suspend
	@local: [foo] $ vagrant resume
			
[Additional Commands] Shutdown the VM:	
	
	@local: [foo] $ vagrant halt
			
Vagrant VM characteristics

username/password: vagrant/vagrant  
sudo access does not require a password
	
	@vagrant: [\vagrant] <-- this is the same as foo with r/w access


Monitor the VM

	@local: [foo] $ vagrant status
		
			
Vagrantfile:

Switch to Bridged Mode:
	
	# uncomment
	config.vm.network :bridged
			
Port-forwarding:
	
	# uncomment
	config.vm.forward_port 80, 8080	
			
Reload the Vagrantfile
	
	@local: [foo] $ vagrant reload
			
Package the VM as BOX file for distribution

	@local: [foo] $ vagrant package	