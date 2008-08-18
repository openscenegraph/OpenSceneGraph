// -*-c++-*- osgWidget - Code by: Jeremy Moles (cubicool) 2007-2008
// $Id: Version.cpp 64 2008-06-30 21:32:00Z cubicool $

#include <osgWidget/Version>
#include <osg/Version>

extern "C" {

const char* osgWidgetGetVersion()
{
    return osgGetVersion();
}

const char* osgWidgetGetLibraryName()
{
    return "OpenSceneGraph Widget Library";
}

}
