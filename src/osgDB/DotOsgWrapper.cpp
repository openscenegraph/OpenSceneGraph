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
#include <osgDB/DotOsgWrapper>

using namespace osgDB;

DotOsgWrapper::DotOsgWrapper(osg::Object* proto,
              const std::string& name,
              const std::string& associates,
              ReadFunc readFunc,
              WriteFunc writeFunc,
              ReadWriteMode readWriteMode)
{


    _prototype = proto;
    _name = name;
    

    // copy the names in the space delimited associates input into
    // a vector of separated names.    
    std::string::size_type start_of_name = associates.find_first_not_of(' ');
    while (start_of_name!=std::string::npos)
    {
        std::string::size_type end_of_name = associates.find_first_of(' ',start_of_name);
        if (end_of_name!=std::string::npos)
        {
            _associates.push_back(std::string(associates,start_of_name,end_of_name-start_of_name));
            start_of_name = associates.find_first_not_of(' ',end_of_name);
        }
        else
        {
            _associates.push_back(std::string(associates,start_of_name,associates.size()-start_of_name));
            start_of_name = end_of_name;
        }
    }
    
    _readFunc = readFunc;
    _writeFunc = writeFunc;
    
    _readWriteMode = readWriteMode;
}
