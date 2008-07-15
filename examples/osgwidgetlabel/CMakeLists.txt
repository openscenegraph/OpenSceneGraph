PROJECT(osgwidgetlabel)

LINK_LIBRARIES(debug osgWidgetd optimized osgWidget)

ADD_EXECUTABLE(osgwidgetlabel osgwidgetlabel.cpp)

SET_TARGET_PROPERTIES(osgwidgetlabel PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})

INSTALL(TARGETS osgwidgetlabel DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
