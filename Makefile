CXX = g++
CFLAGS = -g -Wall --std=c++17 -D_USE_MATH_DEFINES

LIBS = $(resource_load) `pkg-config --libs SDL2 SDL2_net SDL2_image SDL2_mixer SDL2_ttf`

SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin
RESOURCE_DIR = resource

BIN_CLIENT = bomberman
BIN_SERVER = bomberman_server

INCLUDE = -Iresource_pack/include

resource_pack = resource_pack/bin/resource_pack
resource_load = resource_pack/bin/libresource_load.a

libtfd = tinyfiledialogs/libtfd.a

SOURCES_CLIENT = $(wildcard $(SRC_DIR)/client/*.cpp $(SRC_DIR)/client/**/*.cpp)
OBJECTS_CLIENT = $(patsubst $(SRC_DIR)/%,$(OBJ_DIR)/%.o,$(basename $(SOURCES_CLIENT)))

SOURCES_SERVER = $(wildcard $(SRC_DIR)/server/*.cpp $(SRC_DIR)/server/**/*.cpp)
OBJECTS_SERVER = $(patsubst $(SRC_DIR)/%,$(OBJ_DIR)/%.o,$(basename $(SOURCES_SERVER)))

ifeq ($(OS),Windows_NT)
	BIN_CLIENT := $(BIN_CLIENT).exe
	BIN_SERVER := $(BIN_SERVER).exe
	resource_pack := $(resource_pack).exe
endif

all: client server

clean: clean_client clean_server

clean_client:
	rm -f $(BIN_DIR)/$(BIN_CLIENT) $(OBJECTS_CLIENT) $(OBJECTS_CLIENT:.o=.d)

clean_server:
	rm -f $(BIN_DIR)/$(BIN_SERVER) $(OBJECTS_SERVER) $(OBJECTS_SERVER:.o=.d)

$(resource_load): $(resource_pack)
$(resource_pack):
	$(MAKE) -C resource_pack

$(shell mkdir -p $(BIN_DIR) >/dev/null)

client: $(BIN_DIR)/$(BIN_CLIENT) $(BIN_DIR)/resource.dat
$(BIN_DIR)/$(BIN_CLIENT): $(OBJECTS_CLIENT) $(resource_load)
	$(CXX) -o $(BIN_DIR)/$(BIN_CLIENT) $(LDFLAGS) $(OBJECTS_CLIENT) $(LIBS)

server: $(BIN_DIR)/$(BIN_SERVER) $(BIN_DIR)/resource.dat
$(BIN_DIR)/$(BIN_SERVER): $(OBJECTS_SERVER) $(resource_load)
	$(CXX) -o $(BIN_DIR)/$(BIN_SERVER) $(LDFLAGS) $(OBJECTS_SERVER) $(LIBS)

$(OBJ_DIR)/%.res: $(RESOURCE_DIR)/%.rc
	windres $< -O coff -o $@

$(BIN_DIR)/resource.dat: $(resource_pack) resource/resource.txt
	$(resource_pack) resource/resource.txt $(BIN_DIR)/resource.dat

DEPFLAGS = -MT $@ -MMD -MP -MF $(OBJ_DIR)/$*.Td

$(OBJ_DIR)/%.o : $(SRC_DIR)/%.cpp
$(OBJ_DIR)/%.o : $(SRC_DIR)/%.cpp $(OBJ_DIR)/%.d
	@mkdir -p $(dir $@)
	$(CXX) $(DEPFLAGS) $(CFLAGS) -c $(INCLUDE) -o $@ $<
	@mv -f $(OBJ_DIR)/$*.Td $(OBJ_DIR)/$*.d && touch $@

$(OBJ_DIR)/%.d: ;
.PRECIOUS: $(OBJ_DIR)/%.d

include $(wildcard $(patsubst $(SRC_DIR)/%,$(OBJ_DIR)/%.d,$(basename $(SOURCES_CLIENT) $(SOURCES_SERVER))))
