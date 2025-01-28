CC = gcc
CFLAGS = -Wall -Iinclude -Wextra -pedantic -std=c99
SRC_DIR = src
INCLUDE_DIR = include
OBJ_DIR = obj
OBJ_FILES = $(OBJ_DIR)/display.o $(OBJ_DIR)/input.o $(OBJ_DIR)/main.o $(OBJ_DIR)/terminal.o $(OBJ_DIR)/util.o $(OBJ_DIR)/fileio.o
TARGET = kzt

# Rule to compile .c files into .o files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(INCLUDE_DIR)/*.h | $(OBJ_DIR)
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Rule to link the object files into the final executable
$(TARGET): $(OBJ_FILES)
	$(CC) $(OBJ_FILES) -o $(TARGET)

# Clean up object files and the executable
clean:
	rm -rf $(OBJ_DIR) $(TARGET)

.PHONY: clean

