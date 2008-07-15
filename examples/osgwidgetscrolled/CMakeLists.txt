PROJECT(osgwidgetscrolled)

LINK_LIBRARIES(debug osgWidgetd optimized osgWidget)

ADD_EXECUTABLE(osgwidgetscrolled osgwidgetscrolled.cpp)

SET_TARGET_PROPERTIES(osgwidgetscrolled PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})

INSTALL(TARGETS osgwidgetscrolled DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
