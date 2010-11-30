/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield 
 * Copyright (C) 2003-2005 3Dlabs Inc. Ltd.
 * Copyright (C) 2004-2005 Nathan Cournia
 * Copyright (C) 2007 Art Tevs
 * Copyright (C) 2008 Zebra Imaging
 *
 * This application is open source and may be redistributed and/or modified   
 * freely and without restriction, both in commercial and non commercial
 * applications, as long as this copyright notice is maintained.
 * 
 * This application is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

/* file:        src/osg/GLStaticLibrary.h
 * author:      Alok Priyadarshi 2010-04-27
*/

#ifndef OSG_GLSTATICLIBRARY
#define OSG_GLSTATICLIBRARY 1

namespace osg {

class GLStaticLibrary
{
public:
    static void* getProcAddress(const char* procName);
};

}

#endif
