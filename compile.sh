g++ -o main \
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
    -lOGDF -lCOIN