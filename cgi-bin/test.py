#!/usr/bin/env python3

# このスクリプトは、CGI環境変数、標準入力、およびヘッダーパースが
# サーバー側で正しく機能しているかを確認するために使用します。

import sys
import os
import urllib.parse
from html import escape

# --- 1. 入力データの解析とデフォルト値の設定 ---

# 標準入力 (リクエストボディ) を読み込む
try:
    # 標準入力がテキストとして開かれていると仮定
    request_body = sys.stdin.read()
except Exception as e:
    request_body = f"ERROR reading stdin: {e}"

# クエリ文字列を取得し、パラメータを解析する
query_string = os.environ.get("QUERY_STRING", "")
params = urllib.parse.parse_qs(query_string) 

# name パラメータを取得。なければ 'ゲスト' をデフォルト値とする。
name = params.get('name', ['ゲスト'])[0] 
request_method = os.environ.get('REQUEST_METHOD', 'N/A')


# --- 2. CGIヘッダーの出力 ---
# 【修正点】文字化け対策として charset=UTF-8 を明示的に追加
print("Content-Type: text/html; charset=UTF-8") 
print("Status: 200 OK") # DELETE の場合、サーバー側で 204 に変換されることを期待
print() # ヘッダーセクションの終了


# --- 3. HTMLレスポンスボディの出力 ---
print("<!DOCTYPE html>")
print("<html><head><title>CGI Test Result</title>")
print('<style>body { font-family: monospace; } h2 { color: #333; border-bottom: 1px solid #ccc; padding-bottom: 5px; }</style>')
print("</head><body>")

# A. メソッドごとの処理と表示
print(f"<h1>Method: {request_method}</h1>")
print(f"<h2>Hello, {escape(name)}! 👋</h2>")

# HEAD メソッドの場合、ボディは出力しない (サーバー側でボディが削除されることを確認するため)
if request_method == "HEAD":
    print("")
    
elif request_method == "DELETE":
    # DELETE のテスト時は、204 No Content が返されることを期待する
    print("<p>DELETE method received. Simulating resource deletion successful.</p>")
    
else:
    # GET / POST / その他 のデバッグ表示
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
    # POST リクエストでボディが正しく渡されたかを確認
    print(escape(request_body))
    print("</pre>")

print("<hr>")
print("</body></html>")

sys.exit(0)
