#!/usr/bin/env python3

import cgi

# Get the form data
form = cgi.FieldStorage()

# Extract the values from the form data
name = form.getvalue("name")
email = form.getvalue("email")

# Set the content type to HTML
# print("Content-Type: text/html\n")

# Print the response HTML
print("<html>")
print("<head>")
print("<title>CGI GET Method Example - Response</title>")
print("</head>")
print("<body>")
print("<h1>CGI GET Method Example - Response</h1>")
print("<p>Name: {0}</p>".format(name))
print("<p>Email: {0}</p>".format(email))
print("</body>")
print("</html>")
