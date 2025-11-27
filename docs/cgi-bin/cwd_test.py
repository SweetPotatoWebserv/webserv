#!/usr/bin/env python3

# CWD (Current Working Directory) がスクリプト自身のディレクトリに設定されているかを確認するCGIスクリプト

import os
import sys

# --- 1. CWDの取得 ---
try:
    # os.getcwd() で現在の作業ディレクトリを取得
    current_working_directory = os.getcwd()
except Exception as e:
    current_working_directory = f"ERROR: Could not get CWD: {e}"

# --- 2. CGIヘッダーの出力 ---
print("Content-Type: text/plain") 
print("Status: 200 OK")
print() # ヘッダーセクションの終了

# --- 3. ボディの出力 ---
# テストコード (test.cpp) で 'CWD: cgi-bin' を探すアサートに対応
# プロジェクトルート (/src) から実行される場合、CWD は 'cgi-bin' になっているはず
print(f"CWD: {current_working_directory}")

# 正常終了
sys.exit(0)
