#!/usr/bin/env python

import cgi

# Create instance of FieldStorage
form = cgi.FieldStorage()

# Get data from fields
name = form.getvalue('name')

# Print the response header
print("Content-type: text/html\n")

# Print the HTML response
print("<!DOCTYPE html>")
print("<html>")
print("<head>")
print("    <title>Form Response</title>")
print("</head>")
print("<body>")
print("    <h1>Form Response</h1>")
print("    <p>Hello, {}!</p>".format(name))
print("</body>")
print("</html>")