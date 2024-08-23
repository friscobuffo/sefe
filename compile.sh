emcc \
    -IOGDF/include \
    -LOGDF \
    src/main.cpp \
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
    src/sefe/embedderSefe.cpp \
    -lOGDF -lCOIN \
    -o static/main.js \
    -s EXPORTED_FUNCTIONS='["_embedLoadedFile", "_sefeMainTest"]' \
    -s EXPORTED_RUNTIME_METHODS='["ccall", "cwrap", "FS"]' \
    --preload-file example-graphs@/example-graphs