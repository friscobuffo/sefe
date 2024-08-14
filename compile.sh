emcc \
    -IOGDF/include \
    -LOGDF \
    main.cpp \
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
    sefe/embedderSefe.cpp \
    -lOGDF -lCOIN \
    -o static/main.js \
    -s EXPORTED_FUNCTIONS='["_embedLoadedFile", "_sefeMainTest"]' \
    -s EXPORTED_RUNTIME_METHODS='["ccall", "cwrap", "FS"]' \
    --preload-file graphs-sefe/a0.txt \
    --preload-file graphs-sefe/a1.txt