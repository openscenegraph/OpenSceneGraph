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
#include <osg/Referenced>
#include <osg/Notify>

#include <typeinfo>
#include <memory>

namespace osg
{

static std::auto_ptr<DeleteHandler> s_deleteHandler(0);

void Referenced::setDeleteHandler(DeleteHandler* handler)
{
#if defined(_MSC_VER) && (_MSC_VER <= 1200) // 1200 == VC++ 6.0
    s_deleteHandler = handler;
#else
    s_deleteHandler.reset(handler);
#endif
}

DeleteHandler* Referenced::getDeleteHandler()
{
    return s_deleteHandler.get();
}

Referenced::~Referenced()
{
    if (_refCount>0)
    {
        notify(WARN)<<"Warning: deleting still referenced object "<<this<<" of type '"<<typeid(this).name()<<"'"<<std::endl;
        notify(WARN)<<"         the final reference count was "<<_refCount<<", memory corruption possible."<<std::endl;
    }
}

}; // end of namespace osg
