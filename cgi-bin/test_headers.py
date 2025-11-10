#!/usr/bin/env python3
import os

print("Content-Type: text/plain")

query = os.environ.get("QUERY_STRING", "")

if query == "status=204":
    print("Status: 204 No Content")
    # 204はボディを含めてはならない
    print("") # ヘッダの終わり
elif query == "status=301":
    print("Status: 301 Moved Permanently")
    print("Location: http://www.google.com/")
    print("") # ヘッダの終わり
    print("Redirecting...")
elif query == "status=400":
    print("Status: 400 Bad Request")
    print("") # ヘッダの終わり
    print("Bad query parameter.")
elif query == "status=bad_header":
    # Content-Type なし
    print("Status: 200 OK")
    print("") # ヘッダの終わり
    print("This response is invalid.")
else:
    print("Status: 200 OK")
    print("") # ヘッダの終わり
    print("Default OK response from test_headers.py")
