# Compiler
EMCC = emcc

# Include and Library Directories
INCLUDES = -IOGDF/include
LIBS = -LOGDF

# Source Files
SRCS = src/main.cpp \
       src/basic/graph.cpp \
       src/basic/graphLoader.cpp \
       src/auslander-parter/biconnectedComponent.cpp \
       src/auslander-parter/cycle.cpp \
       src/auslander-parter/segment.cpp \
       src/auslander-parter/interlacement.cpp \
       src/auslander-parter/embedder.cpp \
       src/sefe/bicoloredGraph.cpp \
       src/sefe/intersectionCycle.cpp \
       src/sefe/bicoloredSegment.cpp \
       src/sefe/interlacementSefe.cpp \
       src/sefe/embedderSefe.cpp

# Object Files (stored in obj/ directory)
OBJS = $(SRCS:%.cpp=$(OBJ_DIR)/%.o)

# Output directories
OUT_DIR = static
OBJ_DIR = bin

# Target output
TARGET = $(OUT_DIR)/main.js

# Libraries to link
LDFLAGS = -lOGDF -lCOIN

# Emscripten options
EMFLAGS = -s EXPORTED_FUNCTIONS='["_embedLoadedFile", "_sefeMainTest"]' \
          -s EXPORTED_RUNTIME_METHODS='["ccall", "cwrap", "FS"]'

# Preload files
PRELOAD_FILES = --preload-file example-graphs@/example-graphs

# Default rule
all: $(TARGET)

# Rule to build the target
$(TARGET): $(OBJS)
	mkdir -p $(OUT_DIR)
	$(EMCC) $(INCLUDES) $(LIBS) $(OBJS) $(LDFLAGS) -o $(TARGET) $(EMFLAGS) $(PRELOAD_FILES)

# Rule to compile .cpp files to .o files
$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(EMCC) $(INCLUDES) -c $< -o $@

# Clean up
clean:
	rm -rf $(OUT_DIR) $(OBJ_DIR)
