#!/usr/bin/env python3

import sys

# 1. stdin からリクエストボディを読み込む
body = sys.stdin.read()

# 2. レスポンスヘッダーを出力
print("Content-Type: text/plain")
print("Status: 200 OK")
print() # 空行

# 3. レスポンスボディを出力
print("--- Hello from CgiProcess ---")
print("I received this body via stdin:")
print(body)
print("-----------------------------")
