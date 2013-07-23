INCLUDE_DIRECTORIES(${CMAKE_CURRENT_LIST_DIR})

SET(TARGET_SRC
    ip/IpEndpointName.cpp
    osc/OscOutboundPacketStream.cpp
    osc/OscPrintReceivedElements.cpp
    osc/OscReceivedElements.cpp
    osc/OscTypes.cpp
    OscReceivingDevice.cpp
    OscSendingDevice.cpp
    ReaderWriterOscDevice.cpp
)

SET(TARGET_H
    ip/IpEndpointName.h
    ip/NetworkingUtils.h
    ip/PacketListener.h
    ip/TimerListener.h
    ip/UdpSocket.h
    osc/MessageMappingOscPacketListener.h
    osc/OscException.h
    osc/OscHostEndianness.h
    osc/OscOutboundPacketStream.h
    osc/OscPacketListener.h
    osc/OscPrintReceivedElements.h
    osc/OscReceivedElements.h
    osc/OscTypes.h
    OscReceivingDevice.hpp
    OscSendingDevice.hpp
)

if(WIN32 AND NOT ANDROID)
    SET(TARGET_SRC
        ${TARGET_SRC}
        ip/win32/NetworkingUtils.cpp
        ip/win32/UdpSocket.cpp
    )
    SET(TARGET_EXTERNAL_LIBRARIES "${TARGET_EXTERNAL_LIBRARIES};Ws2_32.lib;winmm")
ELSE()
    SET(TARGET_SRC
        ${TARGET_SRC}
        ip/posix/NetworkingUtils.cpp
        ip/posix/UdpSocket.cpp
    )
ENDIF()

INCLUDE_DIRECTORIES(${OpenSceneGraph_SOURCE_DIR}/src/osgPlugins/osc)

SET(TARGET_ADDED_LIBRARIES osgGA )

#### end var setup  ###
SETUP_PLUGIN(osc)
