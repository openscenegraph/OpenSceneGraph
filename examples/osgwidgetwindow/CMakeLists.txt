PROJECT(osgwidgetwindow)

LINK_LIBRARIES(debug osgWidgetd optimized osgWidget)

ADD_EXECUTABLE(osgwidgetwindow osgwidgetwindow.cpp)

SET_TARGET_PROPERTIES(osgwidgetwindow PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})

INSTALL(TARGETS osgwidgetwindow DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
