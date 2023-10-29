#!/usr/local/bin/python3

import time

# Simulate a long-running script (e.g., 5 seconds)
time.sleep(10)
while(1):
	print("Content-Type: text/html\r\n\r\n")
print("Hello from CGI!")
