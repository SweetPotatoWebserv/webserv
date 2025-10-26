NAME = webserv

DEV_IMAGE_NAME = webserv-dev

CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -pedantic

SRC_DIR = src/
SRC_FILES =  # you can add *.cpp files here 
SRC = $(addprefix($(SRC_DIR), $(SRC_FILES)))

HEADERS = # you can add *.h files here

OBJS = $(SRC:.cpp=.o)
	
all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJS)

%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@
	
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
	docker run -it --rm -p 8080:80 --mount type=bind,src="$(CURDIR)",target=/src $(DEV_IMAGE_NAME)

up: build run

.PHONY: all clean fclean re test build run up

