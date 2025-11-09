NAME = webserv

DEV_IMAGE_NAME = webserv-dev

CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -pedantic -g

SRC_DIR = src
SRC = $(wildcard $(SRC_DIR)/*/*.cpp)

HEADERS = # you can add *.h files here

OBJ = $(SRC:.cpp=.o)

TEST_NAME = test_runner

TEST_SRC = test/test.cpp \
           src/cgi/handler_cgi.cpp \
           src/cgi/request_cgi.cpp \
           src/cgi/response_cgi.cpp \
           src/cgi/executor_cgi.cpp \
           src/core/Common.cpp \

TEST_OBJ = $(TEST_SRC:.cpp=.o)

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

test: $(TEST_NAME)
	@echo "---Run CGI TEST---" # c++ -std=c++17 test_sample.cpp sample.cpp -L/usr/local/lib -lgtest -lgtest_main -pthread
	./$(TEST_NAME)

$(TEST_NAME): $(TEST_OBJ)
	$(CXX) $(CXXFLAGS) -o $(TEST_NAME) $(TEST_OBJ)
build:
	docker build -t $(DEV_IMAGE_NAME) .

run:
	docker run -it --rm -p 8080:8080 --mount type=bind,src="$(CURDIR)",target=/src $(DEV_IMAGE_NAME)

up: build run

.PHONY: all clean fclean re test build run up

