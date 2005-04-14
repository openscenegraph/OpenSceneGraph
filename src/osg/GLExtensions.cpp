/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2005 Robert Osfield 
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

    bool extensionDisabled = false;

    if (result)
    {

        const std::string& disableString = getGLExtensionDisableString();
        if (!disableString.empty())
        {
        
            static const GLubyte* s_renderer = glGetString(GL_RENDERER);
            static std::string s_rendererString(s_renderer?(const char*)s_renderer:"");
            
            std::string::size_type pos=0;
            while ( pos!=std::string::npos && (pos=disableString.find(extension,pos))!=std::string::npos )
            {
                std::string::size_type previousColon = disableString.find_last_of(':',pos);
                std::string::size_type previousSemiColon = disableString.find_last_of(';',pos);
                
                std::string renderer = "";
                if (previousColon!=std::string::npos)
                {
                    if (previousSemiColon==std::string::npos) renderer = disableString.substr(0,previousColon);
                    else if (previousSemiColon<previousColon) renderer = disableString.substr(previousSemiColon+1,previousColon-previousSemiColon-1);
                }

                if (!renderer.empty())
                {
                
                    // remove leading spaces if they exist.
                    std::string::size_type leadingSpaces = renderer.find_first_not_of(' ');
                    if (leadingSpaces==std::string::npos) renderer = ""; // nothing but spaces
                    else if (leadingSpaces!=0) renderer.erase(0,leadingSpaces);

                    // remove trailing spaces if they exist.
                    std::string::size_type trailingSpaces = renderer.find_last_not_of(' ');
                    if (trailingSpaces!=std::string::npos) renderer.erase(trailingSpaces+1,std::string::npos);

                }
                            
                if (renderer.empty())
                {
                    extensionDisabled = true;
                    break;
                }
                
                if (s_rendererString.find(renderer)!=std::string::npos)
                {
                    extensionDisabled = true;
                    break;
                    
                }
                
                // move the position in the disable string along so that the same extension is found multiple times
                ++pos;
            }

        }
    }

    if (result)
    {
        if (!extensionDisabled) osg::notify(INFO)<<"OpenGL extension '"<<extension<<"' is supported."<<std::endl;
        else osg::notify(INFO)<<"OpenGL extension '"<<extension<<"' is supported by OpenGL\ndriver but has been disabled by osg::getGLExtensionDisableString()."<<std::endl;
    }
    else osg::notify(INFO)<<"OpenGL extension '"<<extension<<"' is not supported."<<std::endl;


    return result && !extensionDisabled;
}

void osg::setGLExtensionDisableString(const std::string& disableString)
{
    getGLExtensionDisableString() = disableString;
}

std::string& osg::getGLExtensionDisableString()
{
    static const char* envVar = getenv("OSG_GL_EXTENSION_DISABLE");
    static std::string s_GLExtensionDisableString(envVar?envVar:"Nothing defined");
    return s_GLExtensionDisableString;
}


bool osg::isGLUExtensionSupported(const char *extension)
{
    typedef std::set<std::string>  ExtensionSet;
    static ExtensionSet s_extensionSet;
    static const char* s_extensions = NULL;
    if (s_extensions==NULL)
    {
        // get the extension list from OpenGL.
        s_extensions = (const char*)gluGetString(GLU_EXTENSIONS);
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
