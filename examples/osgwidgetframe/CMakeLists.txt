PROJECT(osgwidgetframe)

LINK_LIBRARIES(debug osgWidgetd optimized osgWidget)

ADD_EXECUTABLE(osgwidgetframe osgwidgetframe.cpp)

SET_TARGET_PROPERTIES(osgwidgetframe PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})

INSTALL(TARGETS osgwidgetframe DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
