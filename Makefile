CXX = g++
CC = gcc

BUILD_DIR = build
SRC_DIR = src
INCLUDE_DIR = include

CCFLAGS = -Wall -I$(INCLUDE_DIR) -O3
CXXFLAGS = -std=c++20 -Wall -I$(INCLUDE_DIR) -I/usr/include/freetype2 -O3
LDFLAGS = -lglfw -lfreetype

SRC_FILES_CPP := $(shell find $(SRC_DIR) -name "*.cpp")
SRC_FILES_C := $(shell find $(SRC_DIR) -name "*.c")
OBJ_FILES_CPP := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRC_FILES_CPP))
OBJ_FILES_C := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRC_FILES_C))

TARGET = $(BUILD_DIR)/main
MAIN_FILE := main.cpp

all: $(TARGET)

$(TARGET): $(MAIN_FILE) $(OBJ_FILES_CPP) $(OBJ_FILES_C)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp $(INCLUDE_DIR)/%.h
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(INCLUDE_DIR)/%.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(CCFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)
