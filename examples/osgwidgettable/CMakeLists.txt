PROJECT(osgwidgettable)

LINK_LIBRARIES(debug osgWidgetd optimized osgWidget)

ADD_EXECUTABLE(osgwidgettable osgwidgettable.cpp)

SET_TARGET_PROPERTIES(osgwidgettable PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})

INSTALL(TARGETS osgwidgettable DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
