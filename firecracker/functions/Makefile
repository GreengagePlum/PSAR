##### Common
CC = gcc

##### Options
CPPFLAGS =
CFLAGS = -static -pipe -Wall -Wextra -Werror -std=iso9899:2018 -pedantic -march=native -O3
LDFLAGS =
LDLIBS =

##### Files
SRC = $(wildcard *.c)
OBJ = $(patsubst %.c,%.o, $(SRC))
EXE = $(patsubst %.c,%, $(SRC))

##### Build rules
.PHONY: all clean

all: $(EXE)

clean:
	rm -f $(EXE) $(OBJ)
