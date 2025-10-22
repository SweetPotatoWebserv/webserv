NAME = webserv

DEV_IMAGE_NAME = webserv-dev

CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -pedantic

SRCS_DIR = srcs/
INCLUDES_DIR = includes/

SRCS_FILES =  # you can add *.cpp files here 
SRCS = $(addprefix($(SRCS_DIR), $(SRCS_FILES)))

HEADERS_FILES = # you can add *.hpp files here
HEADERS = $(addprefix($(INCLUDES_DIR), $(HEADERS_FILES)))

OBJS = $(SRCS:.cpp=.o)
	
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
	docker run -it --rm -v "$(CURDIR)":/srcs $(DEV_IMAGE_NAME)

up: build run

.PHONY: all clean fclean re test build run up

