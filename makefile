CC = gcc
CFLAGS = -Wall -I/usr/include/SDL2
LDFLAGS = -lSDL2 -lSDL2_image

SRC = board.c pieces.c
OBJ = $(SRC:.c=.o)
EXEC = chess

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(EXEC)