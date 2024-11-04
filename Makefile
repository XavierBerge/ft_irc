SRCS = main.cpp  Client.cpp Server.cpp Channel.cpp
NAME = ircserv
OBJS = ${SRCS:.cpp=.o}
CC = g++
CFLAGS =  -Wall -Wextra -Werror -std=c++98
RM = rm -f

.cpp.o:
	${CC} ${CFLAGS} -c $< -o ${<:.cpp=.o}

${NAME}: ${OBJS}
	${CC} ${CFLAGS} -o ${NAME} ${OBJS}

all: ${NAME}

clean:
	${RM} ${OBJS}

fclean: clean
	${RM} ${NAME}

re: fclean all

.PHONY: all clean fclean re%  
