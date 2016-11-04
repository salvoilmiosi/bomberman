CXX = g++
LD = g++
CFLAGS = -g -Wall --std=c++11

RESPACK = utils/resource_pack

LDFLAGS =
LIBS = -lSDL2 -lSDL2_net -lSDL2_image -lSDL2_mixer -lSDL2_ttf

INCLUDE = include
BIN_DIR = bin/Debug
OBJ_DIR = obj/Debug

OUT_BIN = bomberman

ifeq ($(OS),Windows_NT)
	LDFLAGS += -mwindows -lmingw32 -lSDL2main
	OUT_BIN := $(OUT_BIN).exe
	RESPACK := $(RESPACK).exe
endif

$(shell mkdir -p $(BIN_DIR) >/dev/null)
$(shell mkdir -p $(OBJ_DIR) >/dev/null)

DEPFLAGS = -MT $@ -MMD -MP -MF $(OBJ_DIR)/$*.Td

SOURCES = $(wildcard src/*.cpp)
OBJECTS = $(patsubst src/%,$(OBJ_DIR)/%.o,$(basename $(SOURCES)))

all: $(BIN_DIR)/$(OUT_BIN) $(BIN_DIR)/resource.dat

clean:
	rm -rf $(BIN_DIR)
	rm -rf $(OBJ_DIR)

$(BIN_DIR)/resource.dat: $(RESPACK) resource/resource.txt
	utils/resource_pack resource/resource.txt $(BIN_DIR)/resource.dat

$(RESPACK): utils/resource_pack.cpp
	g++ -g -Wall --std=c++11 -o $(RESPACK) utils/resource_pack.cpp

$(BIN_DIR)/$(OUT_BIN): $(OBJECTS)
	$(LD) -o $(BIN_DIR)/$(OUT_BIN) $(OBJECTS) $(LDFLAGS) $(LIBS)

$(OBJ_DIR)/%.o : src/%.cpp
$(OBJ_DIR)/%.o : src/%.cpp $(OBJ_DIR)/%.d
	$(CXX) $(DEPFLAGS) $(CFLAGS) -c -I $(INCLUDE) -o $@ $<
	@mv -f $(OBJ_DIR)/$*.Td $(OBJ_DIR)/$*.d

$(OBJ_DIR)/%.d: ;
.PRECIOUS: $(OBJ_DIR)/%.d

-include $(patsubst src/%,$(OBJ_DIR)/%.d,$(basename $(SOURCES)))