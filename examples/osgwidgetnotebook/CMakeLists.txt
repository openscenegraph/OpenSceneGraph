PROJECT(osgwidgetnotebook)

LINK_LIBRARIES(debug osgWidgetd optimized osgWidget)

ADD_EXECUTABLE(osgwidgetnotebook osgwidgetnotebook.cpp)

SET_TARGET_PROPERTIES(osgwidgetnotebook PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})

INSTALL(TARGETS osgwidgetnotebook DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
