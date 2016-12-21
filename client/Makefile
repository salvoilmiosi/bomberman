CXX = g++
LD = g++
MAKE = make
CFLAGS = -g -Wall --std=c++11

RESPACK_DIR = resource_pack
RESPACK = resource_pack/bin/Debug/resource_pack
RESLOAD = resource_pack/bin/Debug/libresource_load.a

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

$(BIN_DIR)/resource.dat: $(RESPACK) resource/resource.txt
	$(RESPACK) resource/resource.txt $(BIN_DIR)/resource.dat

$(RESLOAD): $(RESPACK)
$(RESPACK):
	$(MAKE) -C $(RESPACK_DIR)

$(BIN_DIR)/$(OUT_BIN): $(OBJECTS)
	$(LD) -o $(BIN_DIR)/$(OUT_BIN) $(OBJECTS) $(LDFLAGS) $(LIBS)

$(OBJ_DIR)/%.o : src/%.cpp
$(OBJ_DIR)/%.o : src/%.cpp $(OBJ_DIR)/%.d
	$(CXX) $(DEPFLAGS) $(CFLAGS) -c -I $(INCLUDE) -o $@ $<
	@mv -f $(OBJ_DIR)/$*.Td $(OBJ_DIR)/$*.d

$(OBJ_DIR)/%.d: ;
.PRECIOUS: $(OBJ_DIR)/%.d

-include $(patsubst src/%,$(OBJ_DIR)/%.d,$(basename $(SOURCES)))