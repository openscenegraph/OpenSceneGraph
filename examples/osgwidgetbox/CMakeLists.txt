PROJECT(osgwidgetbox)

LINK_LIBRARIES(debug osgWidgetd optimized osgWidget)

ADD_EXECUTABLE(osgwidgetbox osgwidgetbox.cpp)

SET_TARGET_PROPERTIES(osgwidgetbox PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})

INSTALL(TARGETS osgwidgetbox DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
