#pragma once 

#include <osgViewer/GraphicsWindow>
#include <osgDB/Registry>

//windowing system
USE_GRAPICSWINDOW_IMPLEMENTATION(IOS)


//plugins

//USE_OSGPLUGIN(obj)
//USE_OSGPLUGIN(ive)

USE_OSGPLUGIN(osg)
USE_DOTOSGWRAPPER_LIBRARY(osg)

USE_OSGPLUGIN(osg2)
USE_SERIALIZER_WRAPPER_LIBRARY(osg)

USE_OSGPLUGIN(imageio)
USE_OSGPLUGIN(rgb)
//USE_OSGPLUGIN(avfoundation)
//USE_OSGPLUGIN(freetype)

