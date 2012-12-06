    
SET(TARGET_SRC
    AutoDiscovery.cpp
    ReaderWriterZeroConfDevice.cpp
)

SET(TARGET_H
    AutoDiscovery.h
)

IF(WIN32)
    INCLUDE_DIRECTORIES(${CMAKE_CURRENT_LIST_DIR}/mdns_win)
    SET(TARGET_SRC
        ${TARGET_SRC}
        AutoDiscoveryWinImpl.cpp
        AutoDiscoveryWinImpl.h
        mdns_win/ClientCommon.c
        mdns_win/ClientCommon.h
        mdns_win/dns_sd.h
        mdns_win/dns-sd.c
    )
    SET(TARGET_EXTERNAL_LIBRARIES "${TARGET_EXTERNAL_LIBRARIES};Ws2_32.lib;winmm")
    SET(TARGET_LIBRARIES_VARS ZEROCONF_LIBRARY)
    ADD_DEFINITIONS(-DNOT_HAVE_GETOPT)
    ADD_DEFINITIONS(-DNOT_HAVE_SETLINEBUF)
ELSE()
    IF(APPLE)
        SET(TARGET_SRC
            ${TARGET_SRC}
            AutoDiscoveryBonjourImpl.h
            AutoDiscoveryBonjourImpl.mm
        )
        SET(TARGET_LIBRARIES_VARS COCOA_LIBRARY )
    ENDIF(APPLE)
ENDIF(WIN32)

SET(TARGET_ADDED_LIBRARIES osgGA )

#### end var setup  ###
SETUP_PLUGIN(zeroconf)
