#!/usr/bin/env python
import os


print "Content-Type: text/html; charset=utf8\n\n"
env = os.environ
print """<table border="1"><tr><th>Name</th><th>Value</th></tr>"""
for e in env:
	print "<tr><td>%s&nbsp;</td><td>%s&nbsp;</td></tr>"%(e,env[e])
print """</table>"""
