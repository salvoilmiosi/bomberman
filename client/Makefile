CXX = g++
LD = g++
MAKE = make
CFLAGS = -g -Wall --std=c++14

RESPACK_DIR = ../resource_pack
RESPACK = $(RESPACK_DIR)/bin/resource_pack
RESLOAD = $(RESPACK_DIR)/bin/libresource_load.a

LDFLAGS =
LIBS = -lSDL2 -lSDL2_net -lSDL2_image -lSDL2_mixer -lSDL2_ttf -pthread

INCLUDE = -Iinclude -I$(RESPACK_DIR)/include
BIN_DIR = ../bin
OBJ_DIR = obj

OUT_BIN = bomberman

ifeq ($(OS),Windows_NT)
	LDFLAGS += -mwindows -lmingw32 -lSDL2main
	OUT_BIN := $(OUT_BIN).exe
	RESPACK := $(RESPACK).exe
	MAKE := mingw32-make
endif

$(shell mkdir -p $(BIN_DIR) >/dev/null)
$(shell mkdir -p $(OBJ_DIR) >/dev/null)

DEPFLAGS = -MT $@ -MMD -MP -MF $(OBJ_DIR)/$*.Td

SOURCES = $(wildcard src/*.cpp)
OBJECTS = $(patsubst src/%,$(OBJ_DIR)/%.o,$(basename $(SOURCES))) $(RESLOAD)

all: $(BIN_DIR)/$(OUT_BIN) $(BIN_DIR)/resource.dat

clean:
	rm -rf $(BIN_DIR)
	rm -rf $(OBJ_DIR)
	$(MAKE) -C $(RESPACK_DIR) clean

$(BIN_DIR)/resource.dat: $(RESPACK) ../resource/resource.txt
	$(RESPACK) ../resource/resource.txt $(BIN_DIR)/resource.dat

$(RESLOAD): $(RESPACK)
$(RESPACK):
	$(MAKE) -C $(RESPACK_DIR)

$(BIN_DIR)/$(OUT_BIN): $(OBJECTS)
	$(LD) -o $(BIN_DIR)/$(OUT_BIN) $(OBJECTS) $(LDFLAGS) $(LIBS)

$(OBJ_DIR)/%.o : src/%.cpp
$(OBJ_DIR)/%.o : src/%.cpp $(OBJ_DIR)/%.d
	$(CXX) $(DEPFLAGS) $(CFLAGS) -c $(INCLUDE) -o $@ $<
	@mv -f $(OBJ_DIR)/$*.Td $(OBJ_DIR)/$*.d

$(OBJ_DIR)/%.d: ;
.PRECIOUS: $(OBJ_DIR)/%.d

-include $(patsubst src/%,$(OBJ_DIR)/%.d,$(basename $(SOURCES)))