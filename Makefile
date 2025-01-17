NAME=webserv

SRC= main.cpp parsing/config/Config.cpp atoi.cpp server/Server.cpp parsing/request/ParsRequest.cpp  cgi/cgi.cpp parsing/ConfigErrors.cpp \
	response/Response.cpp response/Post.cpp response/Delete.cpp response/Get.cpp response/GenearteResponse.cpp \
	response/Locations.cpp response/CheckRequest.cpp
OBJ=$(SRC:.cpp=.o)

FLAGS=-Wall -Wextra -Werror -std=c++98 -fsanitize=address -g3

%.o: %.cpp
	c++ $(FLAGS) -c $< -o $@

all: $(NAME)

run : all
	@./$(NAME) webserv.conf

$(NAME): $(OBJ)
	c++ $(FLAGS) $(OBJ) -o $(NAME)

clean:
	rm -rf $(OBJ)

fclean: clean
	rm -rf $(NAME)

re: fclean all