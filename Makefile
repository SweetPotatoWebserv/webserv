NAME = webserv

DEV_IMAGE_NAME = webserv-dev

CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -pedantic

SRC_DIR = src/
INCLUDES_DIR = includes/

SRC_FILES =  # you can add *.cpp files here 
SRC = $(addprefix($(SRC_DIR), $(SRC_FILES)))

HEADERS_FILES = # you can add *.h files here
HEADERS = $(addprefix($(INCLUDES_DIR), $(HEADERS_FILES)))

OBJS = $(SRC:.cpp=.o)
	
INCLUDES = -I $(INCLUDES_DIR)

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJS)

%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@
	
clean:
	$(RM) $(OBJS)

fclean: clean
	$(RM) $(NAME)

re: fclean all

test: 
	@echo "No test defined" # c++ -std=c++17 test_sample.cpp sample.cpp -L/usr/local/lib -lgtest -lgtest_main -pthread

build:
	docker build -t $(DEV_IMAGE_NAME) .

run:
	docker run -it --rm -v "$(CURDIR)":/src $(DEV_IMAGE_NAME)

up: build run

.PHONY: all clean fclean re test build run up

