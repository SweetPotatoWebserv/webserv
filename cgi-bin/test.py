#!/usr/bin/env python3

# CgiProcessの関数が動いているかのみを確かめるテスト
# stdinから読み込みが出来てるかを確かめる

import sys
import os

body = sys.stdin.read()

# 実際にはresponse_cgi.cppがパースしてstatus_code_などに設定する
print("Content-Type: text/plain")
print("Status: 200 OK")
print()

# --- CgiExecutor が stdin/stdout を正しく処理したかのテスト ---
print("Hello from CgiProcess")
print(body)

# --- CgiEnvBuilder (createEnvp) が envp を正しく構築したかのテスト ---
method = os.environ.get("REQUEST_METHOD", "NOT_FOUND")
query = os.environ.get("QUERY_STRING", "NOT_FOUND")
content_type = os.environ.get("CONTENT_TYPE", "NOT_FOUND")

print(f"Method: {method}")
print(f"Query String: {query}")
print(f"Content-Type: {content_type}")
