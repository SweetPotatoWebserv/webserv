#!/usr/bin/python3
import datetime
print("Content-Type: text/html\r\n\r\n")
print("<html><body><h1>CGI Time Check</h1>")
print("<p>Current Time: {}</p></body></html>".format(datetime.datetime.now()))
