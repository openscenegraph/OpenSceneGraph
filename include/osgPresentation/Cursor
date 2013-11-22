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

#ifndef OSGPRESENTATION_CURSOR
#define OSGPRESENTATION_CURSOR 1

#include <osg/AutoTransform>
#include <osg/Camera>
#include <osgPresentation/Export>

namespace osgPresentation {

class OSGPRESENTATION_EXPORT Cursor : public osg::Group
{
    public:

        Cursor();

        Cursor(const std::string& filename, float size);

        /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
        Cursor(const Cursor& rhs,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);

        META_Node(osgPresentation, Cursor)

        void setFilename(const std::string& filename) { _filename = filename; _cursorDirty=true; }
        const std::string& getFilename() const { return _filename; }

        void setSize(float size) { _size = size; _cursorDirty=true; }
        float getSize() const { return _size; }

        virtual void traverse(osg::NodeVisitor& nv);

    protected:

        virtual ~Cursor();

        void initializeCursor();
        void updatePosition();

        std::string _filename;
        float       _size;

        bool _cursorDirty;

        osg::ref_ptr<osg::AutoTransform> _transform;

        osg::Vec2                       _cursorXY;
        osg::observer_ptr<osg::Camera>  _camera;

};

}

#endif
