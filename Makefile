.PHONY: clean
CC= g++
CFLAGS= -std=c++2a -pedantic -Wfatal-errors -Wconversion -Wredundant-decls -Wshadow -Wall -Wextra -O3

SRC_DIR= src
OBJ_DIR= obj

SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC_FILES))

test: 
	$(info $(SRC_FILES)) $(info $(OBJ_FILES))

main: $(OBJ_FILES)
	$(CC) $(CFLAGS) $(OBJ_FILES) -lglpk -o main

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(SRC_DIR)/%.hpp
	$(CC) $(CFLAGS) $< -c -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CC) $(CFLAGS) $< -c -o $@

clean:
	@rm -rf $(OBJ_DIR)/*.o main