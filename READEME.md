## pre-commit 設定
1. linux または、mac のリンターのコメントアウトを外す
2. シンボリックリンクを貼る
`ln -snf $(pwd)/script/pre-commit .git/hooks/pre-commit`

## ディレクトリ構造
```
.
├── src/
│   ├── core # 基盤インフラ（エントリーポイント、ユーティリティ関数等）
│   ├── config # 設定ファイル関連（設定ファイルのロード）
│   ├── event # イベント関連（イベントシステムへの登録）
│   └── http # http関連（リクエストの受け取り、レスポンスを作成）
├── test/
│   ├── *_test.cpp
│   └── ...
├── conf/
│   └── nginx.conf
├── Makefile
├── docs/
│   └── design.md
├── .github
├── script/
│   └── pre-commit
├── .clang-format
├── .clang-tidy
├── .editorconfig
├── .gitignore
└── .gitmessage.txt
```
