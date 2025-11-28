#!/usr/bin/python3
import sys
import os

# Docker内の絶対パス
IMAGE_PATH = "/home/yehara/work/webserv/docs/img/100MB.jpg"

# ファイル存在確認
if not os.path.exists(IMAGE_PATH):
    print("Status: 404 Not Found")
    print("Content-Type: text/plain")
    print()
    print("Image not found.")
    sys.exit(0)

# --- 修正箇所ここから ---

# 環境変数からContent-Lengthを取得
content_length = os.environ.get('CONTENT_LENGTH')
if content_length:
    size_to_read = int(content_length)
    # 指定されたサイズだけ読み込む（これでEOF待ちを防ぐ）
    if size_to_read > 0:
        _ = sys.stdin.buffer.read(size_to_read)
else:
    # Content-Lengthがない場合は仕方なく全部読む（リスクあり）
    _ = sys.stdin.buffer.read()

# --- 修正箇所ここまで ---

# 以下、画像の送り返し処理
with open(IMAGE_PATH, "rb") as f:
    data = f.read()

print("Content-Type: image/jpeg")
print(f"Content-Length: {len(data)}")
print()
sys.stdout.flush()

sys.stdout.buffer.write(data)
sys.stdout.buffer.flush()
