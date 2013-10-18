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
//osgManipulator - Copyright (C) 2007 Fugro-Jason B.V.

#ifndef OSGMANIPULATOR_TRANSLATEAXISDRAGGER
#define OSGMANIPULATOR_TRANSLATEAXISDRAGGER 1

#include <osgManipulator/Translate1DDragger>
#include <osg/ShapeDrawable>
#include <osg/Geometry>
#include <osg/LineWidth>

namespace osgManipulator {

/**
 * Dragger for performing translation in all three axes.
 */
class OSGMANIPULATOR_EXPORT TranslateAxisDragger : public CompositeDragger
{
    public:

        TranslateAxisDragger();

        META_OSGMANIPULATOR_Object(osgManipulator,TranslateAxisDragger)

        /** Setup default geometry for dragger. */
        void setupDefaultGeometry();

        /** Sets the width of the axis lines in pixels. */
        void setAxisLineWidth(float linePixelWidth);

        /** Retrieves the width of the axis lines in pixels. */
        float getAxisLineWidth() const { return _axisLineWidth; }

        /** Sets the radius of the cylinders representing the axis lines for picking. */
        void setPickCylinderRadius(float pickCylinderRadius);

        /** Retrieves the radius of the cylinders representing the axis lines for picking. */
        float getPickCylinderRadius() const { return _pickCylinderRadius; }

        /** Sets the height of the cones. */
        void setConeHeight(float radius);

        /** Retrieves the height of the cones. */
        float getConeHeight() const { return _coneHeight; }

    protected:

        virtual ~TranslateAxisDragger();

        osg::ref_ptr< Translate1DDragger >  _xDragger;
        osg::ref_ptr< Translate1DDragger >  _yDragger;
        osg::ref_ptr< Translate1DDragger >  _zDragger;

        float _coneHeight;
        float _axisLineWidth;
        float _pickCylinderRadius;

        osg::ref_ptr<osg::Geode> _lineGeode;
        osg::ref_ptr<osg::Cylinder> _cylinder;
        osg::ref_ptr<osg::LineWidth> _lineWidth;
        osg::ref_ptr<osg::Cone> _cone;
};


}

#endif
