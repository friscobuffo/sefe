emcc \
    -IOGDF/include \
    -LOGDF \
    main.cpp \
    graph.cpp \
    biconnectedComponent.cpp \
    cycle.cpp \
    graphLoader.cpp \
    segment.cpp \
    interlacement.cpp \
    embedder.cpp \
    -lOGDF -lCOIN \
    -o main.js \
    -s EXPORTED_FUNCTIONS='["_embedLoadedFile"]' \
    -s EXPORTED_RUNTIME_METHODS='["ccall", "cwrap", "FS"]'