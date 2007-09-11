/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield 
 *
 * This library is open source and may be redistributed and/or modified under  
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or 
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * OpenSceneGraph Public License for more details.
*/
#include <osg/Version>
#include <string>
#include <stdio.h>

extern "C" {

const char* osgGetVersion()
{
    static char osg_version[256];
    static int osg_version_init = 1;
    if (osg_version_init)
    {
        if (OSG_VERSION_REVISION==0)
        {
            sprintf(osg_version,"%d.%d.%d",OSG_VERSION_MAJOR,OSG_VERSION_MINOR,OSG_VERSION_RELEASE);
        }
        else
        {
            sprintf(osg_version,"%d.%d.%d-%d",OSG_VERSION_MAJOR,OSG_VERSION_MINOR,OSG_VERSION_RELEASE,OSG_VERSION_REVISION);
        }

        osg_version_init = 0;
    }
    
    return osg_version;
}

const char* osgGetSOVersion()
{
    static char osg_soversion[32];
    static int osg_soversion_init = 1;
    if (osg_soversion_init)
    {
        sprintf(osg_soversion,"%d",OPENSCENEGRAPH_SOVERSION);
        osg_soversion_init = 0;
    }
    
    return osg_soversion;
}

const char* osgGetLibraryName()
{
    return "OpenSceneGraph Library";
}

}
