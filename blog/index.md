---
layout: page
title: The Geek Debauchery!
tagline: Supporting tagline
---
{% include JB/setup %}

Vaibhav Bajpai is a Graduate Student in the [Computer Science-Smart Systems Program](http://ses.jacobs-university.de/ses/smartsystems) at [Jacobs University Bremen](http://jacobs-university.de/). He is interested in Computer Networks, Operating Systems and Computer Vision.

[www.vaibhavbajpai.com &rarr;](http://www.vaibhavbajpai.com)  
  
<br/>
  
## Blog Posts
<ul class="posts">
  {% for post in site.posts %}
    <li><span>{{ post.date | date_to_string }}</span> &raquo; <a href="{{ BASE_PATH }}{{ post.url }}">{{ post.title }}</a></li>
  {% endfor %}
</ul>
