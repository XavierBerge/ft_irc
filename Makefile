
SRCDIR = srcs
INCDIR = includes
CMDIR = $(SRCDIR)/Commands


SRCS = $(SRCDIR)/main.cpp \
       $(SRCDIR)/Client.cpp \
       $(SRCDIR)/Server.cpp \
       $(SRCDIR)/Channel.cpp \
       $(SRCDIR)/Authentification.cpp \
       $(SRCDIR)/Command_parsing.cpp \
       $(CMDIR)/Commands.cpp \
       $(SRCDIR)/Mode.cpp

NAME = ircserv
OBJS = $(SRCS:.cpp=.o)


CC = g++
CFLAGS = -Wall -Wextra -Werror -std=c++98 -I$(INCDIR)
RM = rm -f


.cpp.o:
	$(CC) $(CFLAGS) -c $< -o ${<:.cpp=.o}


$(NAME): $(OBJS)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJS)


all: $(NAME)


clean:
	$(RM) $(OBJS)


fclean: clean
	$(RM) $(NAME)


re: fclean all


.PHONY: all clean fclean re
