CC = gcc
CFLAGS = -Wall -Iinclude -Wextra -pedantic -std=c99 
SRC_DIR = src
INCLUDE_DIR = include
OBJ_DIR = obj
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
TARGET = kzt

all: $(TARGET)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJ_FILES)
	$(CC) $(OBJ_FILES) -o $(TARGET)

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR) $(TARGET)

.PHONY: clean all
