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

//
// OpenFlight® loader for OpenSceneGraph
//
//  Copyright (C) 2005-2007  Brede Johansen
//

#include "Document.h"

using namespace flt;


Document::Document() :
    _replaceClampWithClampToEdge(false),
    _preserveFace(false),
    _preserveObject(false),
    _replaceDoubleSidedPolys(false),
    _defaultDOFAnimationState(false),
    _useTextureAlphaForTransparancyBinning(true),
    _useBillboardCenter(false),
    _doUnitsConversion(true),
    _readObjectRecordData(false),
    _desiredUnits(METERS),
    _done(false),
    _level(0),
    _subfaceLevel(0),
    _unitScale(1.0),
    _version(0),
    _colorPoolParent(false),
    _texturePoolParent(false),
    _materialPoolParent(false),
    _lightSourcePoolParent(false),
    _lightPointAppearancePoolParent(false),
    _lightPointAnimationPoolParent(false),
    _shaderPoolParent(false)
{
    _subsurfaceDepth = new osg::Depth(osg::Depth::LESS, 0.0, 1.0,false);
}

Document::~Document()
{
}

void Document::pushLevel()
{
    _levelStack.push_back(_currentPrimaryRecord.get());
    _level++;
}

void Document::popLevel()
{
    _levelStack.pop_back();

    if (!_levelStack.empty())
        _currentPrimaryRecord = _levelStack.back();

    if (--_level<=0)
        _done = true;
}

void Document::pushSubface()
{
    _subfaceLevel++;
}

void Document::popSubface()
{
    _subfaceLevel--;
}

void Document::pushExtension()
{
    if (!_currentPrimaryRecord.valid())
    {
        OSG_WARN << "No current primary in Document::pushExtension()." << std::endl;
        return;
    }

    _extensionStack.push_back(_currentPrimaryRecord.get());
}

void Document::popExtension()
{
    _currentPrimaryRecord=_extensionStack.back().get();
    if (!_currentPrimaryRecord.valid())
    {
        OSG_WARN << "Can't decide primary in Document::popExtension()." << std::endl;
        return;
    }

    _extensionStack.pop_back();
}

osg::Node* Document::getInstanceDefinition(int no)
{
    InstanceDefinitionMap::iterator itr = _instanceDefinitionMap.find(no);
    if (itr != _instanceDefinitionMap.end())
        return (*itr).second.get();

    return NULL;
}

void Document::setSubSurfacePolygonOffset(int level, osg::PolygonOffset* po)
{
    _subsurfacePolygonOffsets[level] = po;
}

osg::PolygonOffset* Document::getSubSurfacePolygonOffset(int level)
{
    OSG_DEBUG<<"Document::getSubSurfacePolygonOffset("<<level<<")"<<std::endl;
    osg::ref_ptr<osg::PolygonOffset>& po = _subsurfacePolygonOffsets[level];
    if (!po)
    {
        po = new osg::PolygonOffset(-1.0f*float(level), -1.0f);
    }
    return po.get();
}

double flt::unitsToMeters(CoordUnits unit)
{
    switch (unit)
    {
    case METERS:
        return 1.0;
    case KILOMETERS:
        return 1000.0;
    case FEET:
        return 0.3048;
    case INCHES:
        return 0.02540;
    case NAUTICAL_MILES:
        return 1852.0;
    }

    return 1.0;
}
