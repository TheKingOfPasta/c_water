CC=gcc
CFLAGS += -Wall -Wextra -Werror -Wvla -pedantic -Wswitch
CFLAGS += -Wno-error=unused-variable -Wno-error=unused-result
# CFLAGS += -fsanitize=address
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

SHADER_DIR = shaders
SHADERS_SRC = $(SHADER_DIR)/full_red.frag $(SHADER_DIR)/default.vert
SHADERS_SPV := $(SHADERS_SRC:%=%.spv)
SHADERS_C := $(SHADERS_SPV:%=%.c)
SRC += $(SHADERS_C)

TARGET=c_water

all: $(LIB) $(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

$(SHADER_DIR)/%.spv: $(SHADER_DIR)/%
	glslc $< -o $@

$(SHADER_DIR)/%.spv.c: $(SHADER_DIR)/%.spv
	xxd -i $< > $@

config/config.so:
	$(CC) -shared -fPIC $(CFLAGS) config/config.c -o config/config.so

clean:
	rm $(TARGET) $(LIB:.c=.so) $(OBJ) -fr

.PHONY: all clean
