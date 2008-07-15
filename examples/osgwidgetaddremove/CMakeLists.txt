PROJECT(osgwidgetaddremove)

LINK_LIBRARIES(debug osgWidgetd optimized osgWidget)

ADD_EXECUTABLE(osgwidgetaddremove osgwidgetaddremove.cpp)

SET_TARGET_PROPERTIES(osgwidgetaddremove PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})

INSTALL(TARGETS osgwidgetaddremove DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
