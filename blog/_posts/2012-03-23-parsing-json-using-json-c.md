---
layout: post
title: "Parsing JSON using json c"
description: ""
category: 
tags: []
---
{% include JB/setup %}
Install `json-c`

	$ brew install json-c
	
Read the `JSON` file and parse it into a `json_object` struct.
	

	#include <json/json.h>
	...
	int main(int argc, char **argv) {

	  /* read and mmap the json file */
	  fsock = open(argv[1], O_RDONLY);
	  if (fsock == -1)
	    perror("open");
	  if (fstat(fsock, &sb) == -1)
	    perror("fstat");
	  fmap = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fsock, 0); 
	  if (fmap == MAP_FAILED)
	    perror("mmap");

	  /* parse the json into a json_object struct */  
	  struct json_object *fjson = json_tokener_parse(fmap);  
	  ...
	  /* cleanup the object when done */
	  json_object_object_del(fjson, "");
	}  
  
Use getters to get to specific part in the `JSON` object

	struct json_object* child = json_object_object_get(fjson, "child");

Use type specific getters on the specific object.

	int json_object_get_int(struct json_object*);
	char* json_object_get_string(struct json_object*);
	...
	
