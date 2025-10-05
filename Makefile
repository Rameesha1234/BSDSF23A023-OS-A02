CC = gcc
CFLAGS = -Wall -g -std=gnu11
SRC = src/ls-v1.0.0.c
OBJ = obj/ls-v1.0.0.o
BIN = bin/ls

all: $(BIN)

$(BIN): $(OBJ) | bin
	$(CC) $(CFLAGS) -o $(BIN) $(OBJ)

$(OBJ): $(SRC) | obj
	$(CC) $(CFLAGS) -c $(SRC) -o $(OBJ)

bin:
	mkdir -p bin

obj:
	mkdir -p obj

clean:
	rm -f $(OBJ) $(BIN)
