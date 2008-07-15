PROJECT(osgwidgetmenu)

LINK_LIBRARIES(debug osgWidgetd optimized osgWidget)

ADD_EXECUTABLE(osgwidgetmenu osgwidgetmenu.cpp)

SET_TARGET_PROPERTIES(osgwidgetmenu PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})

INSTALL(TARGETS osgwidgetmenu DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
