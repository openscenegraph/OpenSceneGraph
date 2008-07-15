PROJECT(osgwidgetshader)

LINK_LIBRARIES(debug osgWidgetd optimized osgWidget)

ADD_EXECUTABLE(osgwidgetshader osgwidgetshader.cpp)

SET_TARGET_PROPERTIES(osgwidgetshader PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})

INSTALL(TARGETS osgwidgetshader DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
