PROJECT(osgwidgetcanvas)

LINK_LIBRARIES(debug osgWidgetd optimized osgWidget)

ADD_EXECUTABLE(osgwidgetcanvas osgwidgetcanvas.cpp)

SET_TARGET_PROPERTIES(osgwidgetcanvas PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})

INSTALL(TARGETS osgwidgetcanvas DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
