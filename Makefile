CC=gcc
CFLAGS += -Wall -Wextra -Werror -Wvla -pedantic -Wswitch -O3 -g -std=c23
LDFLAGS = -lglfw -lGL -lm

CFLAGS += $(shell pkg-config --cflags glfw3)
LDFLAGS += $(shell pkg-config --libs glfw3)

SRC=$(shell ls src/*.c)
OBJ=$(SRC:.c=.o)

TARGET=c_water

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

clean:
	rm $(TARGET) $(OBJ) -fr

.PHONY: all clean
