/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2018 Robert Osfield
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

#ifndef OSG_COMPILESLIDECALLBACK
#define OSG_COMPILESLIDECALLBACK 1

#include <osgViewer/Viewer>
#include <osgPresentation/Export>

namespace osgPresentation {

class OSGPRESENTATION_EXPORT CompileSlideCallback : public osg::Camera::DrawCallback
{
    public:

        CompileSlideCallback():
            _needCompile(false),
            _frameNumber(0) {}

        virtual void operator()(const osg::Camera& camera) const;

        void needCompile(osg::Node* node) { _needCompile=true; _sceneToCompile = node; }

    protected:

        virtual ~CompileSlideCallback() {}

        mutable bool                _needCompile;
        mutable unsigned int        _frameNumber;
        osg::ref_ptr<osg::Node>     _sceneToCompile;

};

}

#endif
