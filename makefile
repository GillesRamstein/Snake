CC = gcc
CFLAGS = -Wall -Wextra -O0 -g
INCLUDES = -I/usr/include/SDL2/
LIBS = -lSDL2 -lSDL2_ttf -lm
SRCS = snake_ai.c
# SRCS = snake.c
OBJS = $(SRCS:.c=.o)
MAIN = snake

$(MAIN): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(MAIN) $(OBJS) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	$(RM) *.o *~ $(MAIN)
