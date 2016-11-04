CXX = g++
LD = g++
CFLAGS = --std=c++11 -Wall
LDFLAGS = 
LIBS = -lSDL2 -lSDL2_net
INCLUDE = include
BIN_NAME = bomberman_server

ifeq ($(OS),Windows_NT)
	LDFLAGS := -lmingw32 -lSDL2main
endif

SOURCES = $(wildcard src/*.cpp)

CFLAGS_DEBUG = $(CFLAGS) -g
OBJ_DEBUG = obj/Debug
BIN_DEBUG = bin/Debug
LDFLAGS_DEBUG = $(LDFLAGS)

OBJECTS_DEBUG = $(patsubst src/%,$(OBJ_DEBUG)/%,$(SOURCES:.cpp=.o))
OUT_DEBUG = $(BIN_DEBUG)/$(BIN_NAME)

CFLAGS_RELEASE = $(CFLAGS) -O2
OBJ_RELEASE = obj/Release
BIN_RELEASE = bin/Release
LDFLAGS_RELEASE = $(LDGLAGS) -s

OBJECTS_RELEASE = $(patsubst src/%,$(OBJ_RELEASE)/%,$(SOURCES:.cpp=.o))
OUT_RELEASE = $(BIN_RELEASE)/$(BIN_NAME)

all: debug

clean: clean_debug clean_release

debug: before_debug $(OUT_DEBUG)

before_debug:
	test -d $(BIN_DEBUG) || mkdir -p $(BIN_DEBUG)
	test -d $(OBJ_DEBUG) || mkdir -p $(OBJ_DEBUG)

$(OUT_DEBUG): $(OBJECTS_DEBUG)
	$(LD) -o $(OUT_DEBUG) $(OBJECTS_DEBUG) $(LDFLAGS_DEBUG) $(LIBS)

$(OBJ_DEBUG)/%.o: src/%.cpp $(INCLUDE)/%.h
	$(CXX) $(CFLAGS_DEBUG) -I$(INCLUDE) -c -o $@ $<

debug_clean: clean_debug

clean_debug:
	rm -f $(OUT_DEBUG)
	rm -f $(OBJECTS_DEBUG)

release: before_release $(OUT_RELEASE)

before_release:
	test -d $(BIN_RELEASE) || mkdir -p $(BIN_RELEASE)
	test -d $(OBJ_RELEASE) || mkdir -p $(OBJ_RELEASE)

$(OUT_RELEASE): $(OBJECTS_RELEASE)
	$(LD) -o $(OUT_RELEASE) $(OBJECTS_RELEASE) $(LDFLAGS_RELEASE) $(LIBS)

$(OBJ_RELEASE)/%.o: src/%.cpp $(INCLUDE)/%.h
	$(CXX) $(CFLAGS_RELEASE) -I$(INCLUDE) -c -o $@ $<

release_clean: clean_release

clean_release:
	rm -f $(OUT_RELEASE)
	rm -f $(OBJECTS_RELEASE)

