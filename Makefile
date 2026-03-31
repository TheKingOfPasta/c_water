CC=gcc
CFLAGS += -Wall -Wextra -Werror -Wvla -pedantic -Wswitch -O3 -g -std=c23 -fopenmp
CFLAGS += -Wno-error=unused-variable -Wno-error=unused-result
LDFLAGS = -lglfw -lGL -lm -lGLEW

ifeq ($(shell test -f /etc/NIXOS && echo yes),yes)
    CFLAGS += -D__NIXOS__
endif

CFLAGS += $(shell pkg-config --cflags glfw3)
LDFLAGS += $(shell pkg-config --libs glfw3)

SRC=$(shell ls src/*.c)
OBJ=$(SRC:.c=.o)

SHADER_DIR = shaders
SHADERS_SRC = $(SHADER_DIR)/full_red.frag $(SHADER_DIR)/default.vert
SHADERS_SPV := $(SHADERS_SRC:%=%.spv)
SHADERS_C := $(SHADERS_SPV:%=%.c)
SRC += $(SHADERS_C)

TARGET=c_water

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

$(SHADER_DIR)/%.spv: $(SHADER_DIR)/%
	glslc $< -o $@

$(SHADER_DIR)/%.spv.c: $(SHADER_DIR)/%.spv
	xxd -i $< > $@

clean:
	rm $(TARGET) $(OBJ) -fr

.PHONY: all clean
