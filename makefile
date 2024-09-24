# Compiler
EMCC = emcc

# Include and Library Directories
INCLUDES = -IOGDF/include
LIBS = -LOGDF

# Source Files
SRCS = src/main.cpp \
       src/basic/graph.cpp \
       src/basic/utils.cpp \
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
OBJ_DIR = bin

# Target output
TARGET = static/main.js

# Libraries to link
LDFLAGS = -lOGDF -lCOIN

# Emscripten options
EMFLAGS = -s EXPORTED_FUNCTIONS='["_embedLoadedFile", "_sefeLoadedFiles"]' \
          -s EXPORTED_RUNTIME_METHODS='["ccall", "cwrap", "FS"]'

# Preload files
PRELOAD_FILES = --preload-file example-graphs@/example-graphs

# Default rule
all: $(TARGET)

# Rule to build the target
$(TARGET): $(OBJS)
	$(EMCC) $(INCLUDES) $(LIBS) $(OBJS) $(LDFLAGS) -o $(TARGET) $(EMFLAGS) $(PRELOAD_FILES)

# Optimization flags
OPTFLAGS = -O3

# Rule to compile .cpp files to .o files
$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(EMCC) $(INCLUDES) $(OPTFLAGS) -c $< -o $@