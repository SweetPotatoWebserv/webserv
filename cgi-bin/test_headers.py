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
    # Content-Type がないため、parseCgiResponse で 500 になることを期待
    print("Status: 200 OK")
    print("")
    print("This response is invalid.")

# --- 【追加の修正】暗黙的な 302 リダイレクトのテストケース ---
elif query == "location_only=true":
    # Status を出力しないことで、サーバーが 302 (Found) を暗黙的に設定することをテスト
    print("Location: /test/new_resource") 
    print("") # ヘッダーセクションの終了
    print("Implicit 302 redirecting to /test/new_resource")
# -------------------------------------------------------------

else:
    print("Content-Type: text/plain")
    print("Status: 200 OK")
    print("")
    print("Default OK response from test_headers.py")
