CC = gcc

RAYLIB_CFLAGS := $(shell pkg-config --cflags raylib 2>/dev/null)
RAYLIB_LIBS := $(shell pkg-config --libs raylib 2>/dev/null)

ifeq ($(strip $(RAYLIB_LIBS)),)
RAYLIB_CFLAGS := -I/usr/local/include
RAYLIB_LIBS := -L/usr/local/lib -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
endif

CFLAGS = -Wall -Wextra $(RAYLIB_CFLAGS) -Isrc

LDFLAGS = $(RAYLIB_LIBS) -lm

BUILD_DIR = build

SRC = $(wildcard src/*.c) \
      $(wildcard src/entities/*.c) \
      $(wildcard src/steps/*.c) \
      $(wildcard src/structure/*.c) \
      $(wildcard src/utils/*.c) \
      $(wildcard src/gfx/*.c)

OBJ = $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRC))

TARGET = $(BUILD_DIR)/deixaeu

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJ)
	@mkdir -p $(dir $@)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

run: all
	./$(TARGET)

clean:
	rm -rf $(BUILD_DIR)
