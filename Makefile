NAME = webserv

DEV_IMAGE_NAME = webserv-dev

CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -pedantic -g

SRC_DIR = src
SRC = $(wildcard $(SRC_DIR)/*/*.cpp)
SRC_WITHOUT_MAIN = $(filter-out src/core/main.cpp,$(SRC))

TEST_DIR = test
# test 追加時は TEST_SRC に追加
TEST_SRC = $(addprefix $(TEST_DIR)/, test_main.cpp test_split.cpp test_http_exception.cpp)

HEADERS = # you can add *.h files here

OBJ = $(SRC:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJ)

%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJ)

fclean: clean
	$(RM) $(NAME)

re: fclean all

run-test:
	$(CXX) -std=c++17 -I./src $(TEST_SRC) $(SRC_WITHOUT_MAIN) -L/usr/local/lib -lgtest -pthread -o $(TEST_DIR)/all_tests \
	&& $(TEST_DIR)/all_tests

test:
	docker run -it --rm --mount type=bind,src="$(CURDIR)",target=/src $(DEV_IMAGE_NAME) make -C /src run-test

build:
	docker build -t $(DEV_IMAGE_NAME) .

run:
	docker run -it --rm -p 8080:8080 --mount type=bind,src="$(CURDIR)",target=/src $(DEV_IMAGE_NAME)

up: build run

.PHONY: all clean fclean re test run-test build run up
