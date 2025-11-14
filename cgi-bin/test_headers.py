#!/usr/bin/env python3
import os

query = os.environ.get("QUERY_STRING", "")

if query == "status=204":
    print("Content-Type: text/plain")
    print("Status: 204 No Content")
    print("") 
elif query == "status=301":
    print("Content-Type: text/plain")
    print("Status: 301 Moved Permanently")
    print("Location: http://www.google.com/")
    print("") 
    print("Redirecting...")
elif query == "status=400":
    print("Content-Type: text/plain")
    print("Status: 400 Bad Request")
    print("") 
    print("Bad query parameter.")
elif query == "status=bad_header":
    print("Status: 200 OK")
    print("") 
    print("This response is invalid.")
else:
    print("Content-Type: text/plain")
    print("Status: 200 OK")
    print("") 
    print("Default OK response from test_headers.py")
