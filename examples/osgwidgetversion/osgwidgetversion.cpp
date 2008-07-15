// -*-c++-*- osgWidget - Code by: Jeremy Moles (cubicool) 2007-2008
// $Id: osgwidgetversion.cpp 2 2008-01-24 16:11:26Z cubicool $

#include <osg/Notify>
#include <osgWidget/Version>

int main(int argc, char** argv) {
	osg::notify(osg::NOTICE)
		<< osgWidgetGetLibraryName() << " "
		<< osgWidgetGetVersion() << std::endl
	;

	return 0;
}
