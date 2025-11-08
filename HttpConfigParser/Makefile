# 実行ファイル名 (Day 1 のテスト用)
NAME = test_parser

# コンパイラとフラグ
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -g

# ソースファイル (main とパーサー本体、common を追加)
SRCS = main.cpp \
       HttpConfigParser.cpp \
       common.cpp

# オブジェクトファイル
OBJS = $(SRCS:.cpp=.o)

# ヘッダーファイル (common.h を追加)
HEADERS = HttpConfigParser.hpp \
          HttpConfig.h \
          common.h

# 削除コマンド
RM = rm -f

# --- ルール ---

# デフォルトルール (make)
all: $(NAME)

# 実行ファイルのリンク
$(NAME): $(OBJS) $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJS)

# .cpp から .o を作るルール
%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# (make clean) オブジェクトファイルの削除
clean:
	$(RM) $(OBJS)

# (make fclean) 実行ファイルとオブジェクトの全削除
fclean: clean
	$(RM) $(NAME)

# (make re) リコンパイル
re: fclean all

# (make run) test.conf を使って実行
run: $(NAME)
	@./$(NAME) test.conf

.PHONY: all clean fclean re run