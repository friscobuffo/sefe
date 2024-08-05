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
    -lOGDF -lCOIN \
    -o static/main.js \
    -s EXPORTED_FUNCTIONS='["_embedLoadedFile"]' \
    -s EXPORTED_RUNTIME_METHODS='["ccall", "cwrap", "FS"]'