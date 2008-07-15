PROJECT(osgwidgetstyled)

LINK_LIBRARIES(debug osgWidgetd optimized osgWidget)

ADD_EXECUTABLE(osgwidgetstyled osgwidgetstyled.cpp)

SET_TARGET_PROPERTIES(osgwidgetstyled PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})

INSTALL(TARGETS osgwidgetstyled DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
