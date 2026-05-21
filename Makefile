CC = gcc

CFLAGS = -Wall -Wextra $(shell pkg-config --cflags raylib) -Isrc

LDFLAGS = $(shell pkg-config --libs raylib) -lm

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