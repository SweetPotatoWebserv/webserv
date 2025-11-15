NAME = webserv

DEV_IMAGE_NAME = webserv-dev

CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -pedantic -g

SRC_DIR = src
SRC = $(wildcard $(SRC_DIR)/*/*.cpp)
SRC_WITHOUT_MAIN = $(filter-out src/core/main.cpp,$(SRC))

TEST_DIR = test
# test 追加時は TEST_SRC に追加
TEST_SRC = $(addprefix $(TEST_DIR)/, test_main.cpp test_split.cpp test_trim.cpp test_http_exception.cpp test_mime_types.cpp test_is_request_ready.cpp test_search_header_field.cpp)

HEADERS = # you can add *.h files here

OBJ = $(SRC:.cpp=.o)

CGI_TEST_NAME = cgi_test_runner

CGI_TEST_SRC = test/test.cpp \
           src/cgi/handler_cgi.cpp \
           src/cgi/request_cgi.cpp \
           src/cgi/response_cgi.cpp \
           src/cgi/executor_cgi.cpp \
           src/core/Common.cpp \

CGI_TEST_OBJ = $(CGI_TEST_SRC:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJ)

%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

re: fclean all

test_cgi: build
	docker run -it --rm \
		--mount type=bind,src="$(CURDIR)",target=/src \
		-w /src $(DEV_IMAGE_NAME) \
		make internal_test_cgi

internal_test_cgi: $(CGI_TEST_NAME)
	@echo "---Run CGI TEST (inside container)---"
	./$(CGI_TEST_NAME)

$(CGI_TEST_NAME): $(CGI_TEST_OBJ)
	$(CXX) $(CXXFLAGS) -o $(CGI_TEST_NAME) $(CGI_TEST_OBJ)

run-test:
	$(CXX) -std=c++17 -I./src $(TEST_SRC) $(SRC_WITHOUT_MAIN) -L/usr/local/lib -lgtest -pthread -o $(TEST_DIR)/all_tests \
	&& $(TEST_DIR)/all_tests

test:
	docker run -it --rm --mount type=bind,src="$(CURDIR)",target=/src $(DEV_IMAGE_NAME) make -C /src run-test

build:
	docker build -t $(DEV_IMAGE_NAME) .

clean:
	$(RM) $(OBJ) $(CGI_TEST_OBJ)

fclean: clean
	$(RM) $(NAME) $(CGI_TEST_NAME)

run:
	docker run -it --rm -p 8080:8080 --mount type=bind,src="$(CURDIR)",target=/src $(DEV_IMAGE_NAME)

up: build run

.PHONY: all clean fclean re test run-test build run up
