SET(TARGET_SRC
    unzip.cpp
    ReaderWriterZIP.cpp
)

SET(TARGET_H
    unzip.h
)

ADD_DEFINITIONS(-DZIP_STD)

SETUP_PLUGIN(zip)
