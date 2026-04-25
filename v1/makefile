CC = gcc
CFLAGS  = -Wall $(shell pkg-config --cflags sdl2)
LDFLAGS = $(shell pkg-config --libs sdl2 SDL2_image)

SRC = v1_old_board.c v1_old_pieces.c
OBJ = $(SRC:.c=.o)
EXEC = chess

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(EXEC)