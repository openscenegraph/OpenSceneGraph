/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2003 Robert Osfield 
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
#include <osg/GLExtensions>
#include <osg/GL>
#include <osg/GLU>
#include <osg/Notify>


#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <string>
#include <vector>
#include <set>

bool osg::isGLExtensionSupported(const char *extension)
{
    typedef std::set<std::string>  ExtensionSet;
    static ExtensionSet s_extensionSet;
    static const char* s_extensions = NULL;
    if (s_extensions==NULL)
    {
        // get the extension list from OpenGL.
        s_extensions = (const char*)glGetString(GL_EXTENSIONS);
        if (s_extensions==NULL) return false;

        // insert the ' ' delimiated extensions words into the extensionSet.
        const char *startOfWord = s_extensions;
        const char *endOfWord;
        while ((endOfWord = strchr(startOfWord,' '))!=NULL)
        {
            s_extensionSet.insert(std::string(startOfWord,endOfWord));
            startOfWord = endOfWord+1;
        }
        if (*startOfWord!=0) s_extensionSet.insert(std::string(startOfWord));
        
        osg::notify(INFO)<<"OpenGL extensions supported by installed OpenGL drivers are:"<<std::endl;
        for(ExtensionSet::iterator itr=s_extensionSet.begin();
            itr!=s_extensionSet.end();
            ++itr)
        {
            osg::notify(INFO)<<"    "<<*itr<<std::endl;
        }
            
    }

    // true if extension found in extensionSet.
    bool result = s_extensionSet.find(extension)!=s_extensionSet.end();

    if (result) osg::notify(INFO)<<"OpenGL extension '"<<extension<<"' is supported."<<std::endl;
    else osg::notify(INFO)<<"OpenGL extension '"<<extension<<"' is not supported."<<std::endl;

    return result;
}

bool osg::isGLUExtensionSupported(const char *extension)
{
    typedef std::set<std::string>  ExtensionSet;
    static ExtensionSet s_extensionSet;
    static const char* s_extensions = NULL;
    if (s_extensions==NULL)
    {
        // get the extension list from OpenGL.
        s_extensions = (const char*)gluGetString(GL_EXTENSIONS);
        if (s_extensions==NULL) return false;

        // insert the ' ' delimiated extensions words into the extensionSet.
        const char *startOfWord = s_extensions;
        const char *endOfWord;
        while ((endOfWord = strchr(startOfWord,' '))!=NULL)
        {
            s_extensionSet.insert(std::string(startOfWord,endOfWord));
            startOfWord = endOfWord+1;
        }
        if (*startOfWord!=0) s_extensionSet.insert(std::string(startOfWord));
        
        osg::notify(INFO)<<"OpenGL extensions supported by installed OpenGL drivers are:"<<std::endl;
        for(ExtensionSet::iterator itr=s_extensionSet.begin();
            itr!=s_extensionSet.end();
            ++itr)
        {
            osg::notify(INFO)<<"    "<<*itr<<std::endl;
        }
            
    }

    // true if extension found in extensionSet.
    bool result = s_extensionSet.find(extension)!=s_extensionSet.end();

    if (result) osg::notify(INFO)<<"OpenGL utility library extension '"<<extension<<"' is supported."<<std::endl;
    else osg::notify(INFO)<<"OpenGL utility library extension '"<<extension<<"' is not supported."<<std::endl;

    return result;
}
