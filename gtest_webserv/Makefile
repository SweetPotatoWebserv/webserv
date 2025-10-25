CXX		= c++
NAME	= run_tests

# GTestの場所
GTEST_DIR		= ./googletest/googletest

# Webservのヘッダーファイルの場所
INCLUDES		= -I. -I$(GTEST_DIR)/include -I../includes

# --- フラグの定義 ---GTestヘッダーを使うために-std=c++11を追加
#-std=c++98の使用に合わせるとけいこくがでるため
PROJ_CXXFLAGS	= -std=c++98 -Wall -Wextra -Werror -g
TEST_CXXFLAGS	= -std=c++11 -Wall -Wextra -g
GTEST_CXXFLAGS	= -std=c++11 -g -pthread
LDFLAGS		= -pthread

# ---ソースファイルの定義 ---

# プロジェクトのソース (C++98でコンパイルするもの)
PROJ_SRCS_DIR	= ../src#Webservのソースディレクトリ
PROJ_SRCS		= $(wildcard $(PROJ_SRCS_DIR)/*.cpp)
#↑Webservのソースファイル全て
PROJ_SRCS		:= $(filter-out $(PROJ_SRCS_DIR)/main.cpp, $(PROJ_SRCS))#main.cppを除外:test用にするため
PROJ_OBJS		= $(PROJ_SRCS:.cpp=.o)

#テストコードのソース (C++11でコンパイルするもの)
#_test.cppという名前のファイルをテストコードとする
TEST_SRCS_DIR   = .
TEST_SRCS		= $(wildcard $(TEST_SRCS_DIR)/*_test.cpp) test_main.cpp
TEST_OBJS		= $(TEST_SRCS:.cpp=.o)

#GTestライブラリのソース (C++11でコンパイルするもの)
GTEST_SRC		= $(GTEST_DIR)/src/gtest-all.cc
GTEST_OBJ		= $(GTEST_SRC:.cc=.o) # .cc -> .o

# --- ビルドルール ---

all: $(NAME)

$(NAME): $(PROJ_OBJS) $(TEST_OBJS) $(GTEST_OBJ)
	$(CXX) -o $(NAME) $(PROJ_OBJS) $(TEST_OBJS) $(GTEST_OBJ) $(LDFLAGS)

# --- コンパイルルール (3種類) ---
# 1. プロジェクト コンパイル (C++98)
$(PROJ_OBJS): %.o: %.cpp
	$(CXX) $(PROJ_CXXFLAGS) $(INCLUDES) -c $< -o $@

# 2. テストコード コンパイル (C++11)
$(TEST_OBJS): %.o: %.cpp
	$(CXX) $(TEST_CXXFLAGS) $(INCLUDES) -c $< -o $@

# 3. GTestライブラリ コンパイル (C++11)←ルール緩め
$(GTEST_OBJ): $(GTEST_SRC)
	$(CXX) $(GTEST_CXXFLAGS) $(INCLUDES) -I$(GTEST_DIR) -c $< -o $@

# --- クリーンルール ---

clean:
	rm -f $(PROJ_OBJS) $(TEST_OBJS) $(GTEST_OBJ)
fclean: clean
	rm -f $(NAME)
re: fclean all
test: all
	@./$(NAME)

.PHONY: all clean fclean re test