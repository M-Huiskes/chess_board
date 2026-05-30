CC = gcc
CFLAGS  = -Wall -MMD -MP $(shell pkg-config --cflags sdl2)
-include $(OBJ:.o=.d)
LDFLAGS = $(shell pkg-config --libs sdl2 SDL2_image) -lm

SRC = main.c board.c pieces.c terminal.c state.c bit_utils.c ui.c evaluation.c computer.c
OBJ = $(SRC:.c=.o)
EXEC = chess

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(EXEC)

run: all
	./chess ui computer

run-no-bot: all
	./chess ui no_computer
