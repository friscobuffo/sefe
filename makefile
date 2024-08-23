# Compiler
EMCC = emcc

# Include and Library Directories
INCLUDES = -IOGDF/include
LIBS = -LOGDF

# Source Files
SRCS = main.cpp \
       basic/graph.cpp \
       basic/graphLoader.cpp \
       auslander-parter/biconnectedComponent.cpp \
       auslander-parter/cycle.cpp \
       auslander-parter/segment.cpp \
       auslander-parter/interlacement.cpp \
       auslander-parter/embedder.cpp \
       sefe/bicoloredGraph.cpp \
       sefe/intersectionCycle.cpp \
       sefe/bicoloredSegment.cpp \
       sefe/interlacementSefe.cpp \
       sefe/embedderSefe.cpp

# Object Files (stored in obj/ directory)
OBJS = $(SRCS:%.cpp=$(OBJ_DIR)/%.o)

# Output directories
OUT_DIR = static
OBJ_DIR = obj

# Target output
TARGET = $(OUT_DIR)/main.js

# Libraries to link
LDFLAGS = -lOGDF -lCOIN

# Emscripten options
EMFLAGS = -s EXPORTED_FUNCTIONS='["_embedLoadedFile", "_sefeMainTest"]' \
          -s EXPORTED_RUNTIME_METHODS='["ccall", "cwrap", "FS"]'

# Preload files
PRELOAD_FILES = --preload-file graphs-sefe/a0.txt \
                --preload-file graphs-sefe/a1.txt \
                --preload-file graphs/g1.txt \
                --preload-file graphs/g2.txt \
                --preload-file graphs/g3.txt \
                --preload-file graphs/g4.txt \
                --preload-file graphs/g5.txt \
                --preload-file graphs/g6.txt \
                --preload-file graphs/k5.txt \
                --preload-file graphs/k33.txt

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
