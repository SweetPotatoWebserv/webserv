#!/usr/bin/env python3

import sys
import os
import urllib.parse
from html import escape

try:
    request_body = sys.stdin.read()
except Exception as e:
    request_body = f"ERROR reading stdin: {e}"

query_string = os.environ.get("QUERY_STRING", "")
params = urllib.parse.parse_qs(query_string) 

name = params.get('name', ['ゲスト'])[0] 


print("Content-Type: text/html") 
print("Status: 200 OK")
print() # ヘッダーセクションの終了


print("<!DOCTYPE html>")
print("<html><head><title>CGI Test Result</title>")
print('<style>body { font-family: monospace; } h2 { color: #333; border-bottom: 1px solid #ccc; padding-bottom: 5px; }</style>')
print("</head><body>")

print(f"<h1>Hello, {escape(name)}! 👋</h1>")

print("<h2>CGI & Request Details</h2>")

print("<table>")
env_vars = [
    "REQUEST_METHOD",
    "QUERY_STRING",
    "CONTENT_TYPE",
    "CONTENT_LENGTH",
    "SERVER_PROTOCOL",
    "SERVER_NAME",
    "SERVER_PORT",
    "REMOTE_ADDR",
    "SCRIPT_NAME",
    "PATH_INFO"
]

for var in env_vars:
    value = os.environ.get(var, 'N/A')
    print(f"<tr><td><b>{var}:</b></td><td>{escape(value)}</td></tr>")
print("</table>")

print("<h2>Standard Input (Request Body)</h2>")
print("<pre>")
print(escape(request_body))
print("</pre>")

print("<hr>")
print("</body></html>")

sys.exit(0)
