INCLUDE_DIR = include
SRC_DIR = src
OBJ_DIR = obj
OUT_DIR = bin

CC = g++
CCFLAGS = -g -Wall --std=c++11 -I$(INCLUDE_DIR)
LIBS = -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_net
MKDIR = mkdir -p

EXEC = $(OUT_DIR)/bomberman
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(patsubst $(SRC_DIR)/%,$(OBJ_DIR)/%,$(SOURCES:.cpp=.o))

all: directories $(EXEC)

directories: $(OBJ_DIR) $(OUT_DIR)

$(OBJ_DIR):
	$(MKDIR) $(OBJ_DIR)

$(OUT_DIR):
	$(MKDIR) $(OUT_DIR)

$(EXEC): $(OBJECTS)
	$(CC) -o $(EXEC) $(OBJECTS) $(LIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(INCLUDE_DIR)/%.h
	$(CC) -c $(CCFLAGS) $< -o $@

clean:
	rm -f $(EXEC) $(OBJECTS)