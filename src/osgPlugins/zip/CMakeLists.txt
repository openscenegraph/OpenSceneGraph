SET(TARGET_SRC
    unzip.cpp
    ZipArchive.cpp
    ReaderWriterZIP.cpp
)

SET(TARGET_H
    unzip.h
    ZipArchive.h
)

ADD_DEFINITIONS(-DZIP_STD)

SETUP_PLUGIN(zip)
