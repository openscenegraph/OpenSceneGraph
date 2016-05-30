/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2014 Robert Osfield
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

#ifndef OSG_IDENTIFER_H
#define OSG_IDENTIFER_H

#include <osg/Referenced>
#include <osg/Observer>

#include <string>
#include <cctype>

#define OSG_HAS_IDENTIFIER

namespace osg
{

/** helper function for doing a case insenstive compare of two strings.*/
inline bool iequals(const std::string& lhs, const std::string& rhs)
{
    if (lhs.size()!=rhs.size()) return false;

    for(std::string::size_type i=0; i<lhs.size(); ++i)
    {
        if (std::tolower(lhs[i])!=std::tolower(rhs[i])) return false;
    }

    return true;
}


/** Unique Identifier class to help with efficiently comparing
  * road classification or region via pointers.*/
class OSG_EXPORT Identifier : public osg::Referenced, public osg::Observer
{
public:
    static Identifier* get(const std::string& name, int number=0, osg::Referenced* first=0, osg::Referenced* second=0);
    static Identifier* get(int number, osg::Referenced* first=0, osg::Referenced* second=0);
    static Identifier* get(osg::Referenced* first, osg::Referenced* second=0);

    const std::string& name() const { return _name; }
    const int& number() const { return _number; }

protected:
    Identifier(const std::string& name, int number, osg::Referenced* f, osg::Referenced* s);
    virtual ~Identifier();

    virtual void objectDeleted(void* ptr);

    std::string _name;
    int         _number;
    osg::Referenced* _first;
    osg::Referenced* _second;
};

}

#endif
