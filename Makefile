CC=clang
CXX = clang++
ARCHFLAGS ?=
CFLAGS =
CXXFLAGS = -std=c++11 -Wfatal-errors -Wall -Wextra -Wpedantic -Wshadow -W -pedantic -Wno-reserved-id-macro -Wno-keyword-macro
LDFLAGS = -latomic -lpthread
INC=-I./src/inc/json -I./src/inc -I./src/json -I./src/sketch -I./src

# Final binary
BIN = esplora-sim
# Put all auto generated stuff to this build dir.
BUILD_DIR = ./build

# List of all .cpp source files.
SRC = $(wildcard src/*.cpp) $(wildcard src/sketch/*.cpp)

# All .o files go to build dir.
OBJ = $(SRC:%.cpp=$(BUILD_DIR)/%.o)

# List of .c source files
JSRC = $(wildcard src/json/*.c)

JOBJ = $(JSRC:%.c=$(BUILD_DIR)/%.o)
# Gcc/Clang will create these .d files containing dependencies.
DEP = $(OBJ:%.o=%.d)

# Default target named after the binary.
$(BIN) : $(BUILD_DIR)/$(BIN)

# Actual target of the binary - depends on all .o files.
# Create build directories - same structure as sources.
$(BUILD_DIR)/$(BIN) : $(OBJ) $(JOBJ)
	mkdir -p $(@D)
	$(CXX) $(LDFLAGS) $(ARCHFLAGS) $(CXXFLAGS) $^ -o $@

# Include all .d files
-include $(DEP)

# Build target for every single object file.
# The potential dependency on header files is covered
# by calling `-include $(DEP)`.
# The -MMD flags additionaly creates a .d file with
# the same name as the .o file.
$(BUILD_DIR)/%.o : %.cpp
	mkdir -p $(@D)
	$(CXX) $(ARCHFLAGS) $(CXXFLAGS) $(INC) -MMD -c $< -o $@

$(BUILD_DIR)/%.o : %.c
	mkdir -p $(@D)
	$(CC) $(ARCHFLAGS) $(CFLAGS) $(INC) -MMD -c $< -o $@


.PHONY : clean
clean :
	# This should remove all generated files.
	-rm $(BUILD_DIR)/$(BIN) $(OBJ) $(JOBJ) $(DEP)
	touch ___client_events ___device_updates
	rm ___device_updates ___client_events
