PROJECT(osgwidgetinput)

LINK_LIBRARIES(debug osgWidgetd optimized osgWidget)

ADD_EXECUTABLE(osgwidgetinput osgwidgetinput.cpp)

SET_TARGET_PROPERTIES(osgwidgetinput PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})

INSTALL(TARGETS osgwidgetinput DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
