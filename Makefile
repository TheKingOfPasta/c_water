CC=gcc
CFLAGS += -Wall -Wextra -Werror -Wvla -pedantic -Wswitch
CFLAGS += -Wno-error=unused-variable -Wno-error=unused-result
# CFLAGS += -fsanitize=address -g
CFLAGS += -O3 -std=c23 -fopenmp -Isrc -Iconfig
LDFLAGS = -lglfw -lGL -lm -lGLEW -ldl

ifeq ($(shell test -f /etc/NIXOS && echo yes),yes)
    CFLAGS += -D__NIXOS__
endif

CFLAGS += $(shell pkg-config --cflags glfw3)
LDFLAGS += $(shell pkg-config --libs glfw3)

SRC=$(shell find src -name "*.c")
OBJ=$(SRC:.c=.o)
LIB=$(shell ls config/*.c)

TARGET=c_water

src/glad.o: src/glad.c
	$(CC) $(CFLAGS) -Wno-pedantic -c $< -o $@

all: $(LIB) $(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

config/config.so:
	$(CC) -shared -fPIC $(CFLAGS) config/config.c -o config/config.so

clean:
	rm $(TARGET) $(LIB:.c=.so) $(OBJ) -fr

.PHONY: all clean
