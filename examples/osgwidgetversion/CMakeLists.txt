PROJECT(osgwidgetversion)

LINK_LIBRARIES(debug osgWidgetd optimized osgWidget)

ADD_EXECUTABLE(osgwidgetversion osgwidgetversion.cpp)

SET_TARGET_PROPERTIES(osgwidgetversion PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})

INSTALL(TARGETS osgwidgetversion DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
