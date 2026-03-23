CC=gcc
CFLAGS += -Wall -Wextra -Werror -Wvla -pedantic -Wswitch -O3 -g -std=c23
LDFLAGS = -lglfw -lGL
SRC=$(shell ls src/*.c)

TARGET=c_water

OBJ=$(SRC:.c=.o)

all: $(OBJ)
	$(CC) -o $(TARGET) $(OBJ) $(CFLAGS) $(LDFLAGS)

clean:
	rm $(TARGET)

.PHONY: all clean
