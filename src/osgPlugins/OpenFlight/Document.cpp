//
// OpenFlight® loader for OpenSceneGraph
//
//  Copyright (C) 2005-2006  Brede Johansen
//

#include "Document.h"

using namespace flt;


Document::Document() :
    _preserveFace(false),
    _preserveObject(false),
    _defaultDOFAnimationState(false),
    _useTextureAlphaForTransparancyBinning(true),
    _doUnitsConversion(true),
    _desiredUnits(METERS),
    _done(false),
    _level(0),
    _subfaceLevel(0),
    _unitScale(1.0),
    _version(0),
    _colorPoolParent(false),
    _texturePoolParent(false),
    _materialPoolParent(false),
    _lightPointAppearancePoolParent(false),
    _shaderPoolParent(false)
{
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
        _currentPrimaryRecord = _levelStack.back().get();

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
        osg::notify(osg::WARN) << "No current primary in Document::pushExtension()." << std::endl;
        return;
    }

    _extensionStack.push_back(_currentPrimaryRecord.get());
}


void Document::popExtension()
{
    _currentPrimaryRecord=_extensionStack.back().get();
    if (!_currentPrimaryRecord.valid())
    {
        osg::notify(osg::WARN) << "Can't descide primary in Document::popExtension()." << std::endl;
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
