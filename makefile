CC = gcc
CFLAGS = -Wall -Wextra -Og -g `sdl2-config --cflags`
LIBS = `sdl2-config --libs` -lSDL2_ttf -lm
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
