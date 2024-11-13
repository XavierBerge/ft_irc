SRCDIR = srcs
INCDIR = includes
CMDIR = $(SRCDIR)/Commands
OBJDIR = objs

SRCS = $(SRCDIR)/main.cpp \
       $(SRCDIR)/Client.cpp \
       $(SRCDIR)/Server.cpp \
       $(SRCDIR)/Channel.cpp \
       $(SRCDIR)/Authentification.cpp \
       $(SRCDIR)/Command_parsing.cpp \
       $(CMDIR)/Invite.cpp \
       $(CMDIR)/Join.cpp \
       $(CMDIR)/Kick.cpp \
       $(CMDIR)/Nick.cpp \
       $(CMDIR)/Part.cpp \
       $(CMDIR)/Ping.cpp \
       $(CMDIR)/Privmsg.cpp \
       $(CMDIR)/Quit.cpp \
       $(CMDIR)/Topic.cpp \
       $(SRCDIR)/Mode.cpp

NAME = ircserv
OBJS = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SRCS))
DEPS = $(OBJS:.o=.d)

CC = g++
CFLAGS = -Wall -Wextra -Werror -std=c++98 -I$(INCDIR)
RM = rm -f


$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJS)

all: $(NAME)

clean:
	$(RM) -r $(OBJDIR)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re

