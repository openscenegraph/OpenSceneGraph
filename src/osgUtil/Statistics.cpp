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

#include <osgUtil/Statistics>

#include <osg/PrimitiveSet>
#include <osg/Drawable>
#include <osg/NodeVisitor>
#include <osg/Geode>
#include <osg/LOD>
#include <osg/Switch>
#include <osg/Geometry>
#include <osg/Transform>

#include <map>
#include <set>
#include <ostream>
#include <iomanip>

using namespace osgUtil;

Statistics::Statistics()
{
    reset();
}

void Statistics::reset()
{
    numDrawables=0;
    numFastDrawables=0;
    nummat=0;
    depth=0;
    stattype=STAT_NONE;
    nlights=0;
    nbins=0;
    nimpostor=0;
    numStateGraphs=0;
    numOrderedLeaves=0;

    _vertexCount=0;
    _primitiveCount.clear();

    _currentPrimitiveFunctorMode=0;

    _primitives_count.clear();
    _total_primitives_count=0;
    _number_of_vertexes=0;
}

void Statistics::drawArrays(GLenum mode,GLint,GLsizei count)
{
    PrimitivePair& prim = _primitiveCount[mode];
    ++prim.first;
    prim.second+=count;
    _primitives_count[mode] += _calculate_primitives_number_by_mode(mode, count);
}

void Statistics::drawElements(GLenum mode,GLsizei count,const GLubyte*)
{
    PrimitivePair& prim = _primitiveCount[mode];
    ++prim.first;
    prim.second+=count;
    _primitives_count[mode] += _calculate_primitives_number_by_mode(mode, count);
}

void Statistics::drawElements(GLenum mode,GLsizei count,const GLushort*)
{
    PrimitivePair& prim = _primitiveCount[mode];
    ++prim.first;
    prim.second+=count;
    _primitives_count[mode] += _calculate_primitives_number_by_mode(mode, count);
}

void Statistics::drawElements(GLenum mode,GLsizei count,const GLuint*)
{
    PrimitivePair& prim = _primitiveCount[mode];
    ++prim.first;
    prim.second+=count;
    _primitives_count[mode] += _calculate_primitives_number_by_mode(mode, count);
}


void Statistics::begin(GLenum mode)
{
    _currentPrimitiveFunctorMode=mode;
    PrimitivePair& prim = _primitiveCount[mode];
    ++prim.first;
    _number_of_vertexes = 0;
}

void Statistics::end()
{
    _primitives_count[_currentPrimitiveFunctorMode] +=
        _calculate_primitives_number_by_mode(_currentPrimitiveFunctorMode, _number_of_vertexes);

    _vertexCount += _number_of_vertexes;
}

void Statistics::add(const Statistics& stats)
{
    numDrawables += stats.numDrawables;
    numFastDrawables += stats.numFastDrawables;
    nummat += stats.nummat;
    depth += stats.depth;
    nlights += stats.nlights;
    nbins += stats.nbins;
    nimpostor += stats.nimpostor;
    numStateGraphs += stats.numStateGraphs;
    numOrderedLeaves += stats.numOrderedLeaves;

    _vertexCount += stats._vertexCount;
    for(PrimitiveValueMap::const_iterator pitr = stats._primitiveCount.begin();
        pitr != stats._primitiveCount.end();
        ++pitr)
    {
        _primitiveCount[pitr->first].first += pitr->second.first;
        _primitiveCount[pitr->first].second += pitr->second.second;
    }

    _currentPrimitiveFunctorMode += stats._currentPrimitiveFunctorMode;

    for(PrimitiveCountMap::const_iterator citr = stats._primitives_count.begin();
        citr != stats._primitives_count.end();
        ++citr)
    {
        _primitives_count[citr->first] += citr->second;
    }

    _total_primitives_count += stats._total_primitives_count;
    _number_of_vertexes += stats._number_of_vertexes;
}

StatsVisitor::StatsVisitor():
    osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
    _numInstancedGroup(0),
    _numInstancedSwitch(0),
    _numInstancedLOD(0),
    _numInstancedTransform(0),
    _numInstancedGeode(0),
    _numInstancedDrawable(0),
    _numInstancedGeometry(0),
    _numInstancedFastGeometry(0),
    _numInstancedStateSet(0)
{}

void StatsVisitor::reset()
{
    _numInstancedGroup = 0;
    _numInstancedSwitch = 0;
    _numInstancedLOD = 0;
    _numInstancedTransform = 0;
    _numInstancedGeode = 0;
    _numInstancedDrawable = 0;
    _numInstancedGeometry = 0;
    _numInstancedFastGeometry = 0;
    _numInstancedStateSet = 0;

    _groupSet.clear();
    _transformSet.clear();
    _lodSet.clear();
    _switchSet.clear();
    _geodeSet.clear();
    _drawableSet.clear();
    _geometrySet.clear();
    _fastGeometrySet.clear();
    _statesetSet.clear();

    _uniqueStats.reset();
    _instancedStats.reset();
}

void StatsVisitor::apply(osg::Node& node)
{
    if (node.getStateSet())
    {
        apply(*node.getStateSet());
    }

    traverse(node);
}

void StatsVisitor::apply(osg::Group& node)
{
    if (node.getStateSet())
    {
        apply(*node.getStateSet());
    }

    ++_numInstancedGroup;
    _groupSet.insert(&node);

    traverse(node);
}

void StatsVisitor::apply(osg::Transform& node)
{
    if (node.getStateSet())
    {
        apply(*node.getStateSet());
    }

    ++_numInstancedTransform;
    _transformSet.insert(&node);

    traverse(node);
}

void StatsVisitor::apply(osg::LOD& node)
{
    if (node.getStateSet())
    {
        apply(*node.getStateSet());
    }

    ++_numInstancedLOD;
    _lodSet.insert(&node);

    traverse(node);
}

void StatsVisitor::apply(osg::Switch& node)
{
    if (node.getStateSet())
    {
        apply(*node.getStateSet());
    }

    ++_numInstancedSwitch;
    _switchSet.insert(&node);

    traverse(node);
}

void StatsVisitor::apply(osg::Geode& node)
{
    if (node.getStateSet())
    {
        apply(*node.getStateSet());
    }

    ++_numInstancedGeode;
    _geodeSet.insert(&node);

    traverse(node);
}

void StatsVisitor::apply(osg::Drawable& drawable)
{
    if (drawable.getStateSet())
    {
        apply(*drawable.getStateSet());
    }

    ++_numInstancedDrawable;

    drawable.accept(_instancedStats);

    _drawableSet.insert(&drawable);

    osg::Geometry* geometry = drawable.asGeometry();
    if (geometry)
    {
        ++_numInstancedGeometry;
        _geometrySet.insert(geometry);

        ++_numInstancedFastGeometry;
        _fastGeometrySet.insert(geometry);
    }
}

void StatsVisitor::apply(osg::StateSet& stateSet)
{
   ++_numInstancedStateSet;
   _statesetSet.insert(&stateSet);
}

void StatsVisitor::totalUpStats()
{
    _uniqueStats.reset();

    for(DrawableSet::iterator itr = _drawableSet.begin();
        itr != _drawableSet.end();
        ++itr)
    {
        (*itr)->accept(_uniqueStats);
    }
}

void StatsVisitor::print(std::ostream& out)
{
    unsigned int unique_primitives = 0;
    osgUtil::Statistics::PrimitiveCountMap::iterator pcmitr;
    for(pcmitr = _uniqueStats.GetPrimitivesBegin();
        pcmitr != _uniqueStats.GetPrimitivesEnd();
        ++pcmitr)
    {
        unique_primitives += pcmitr->second;
    }

    unsigned int instanced_primitives = 0;
    for(pcmitr = _instancedStats.GetPrimitivesBegin();
        pcmitr != _instancedStats.GetPrimitivesEnd();
        ++pcmitr)
    {
        instanced_primitives += pcmitr->second;
    }


    out << std::setw(12) << "Object Type" << std::setw(10) << "Unique"                  << std::setw(10) << "Instanced" << std::endl;
    out << std::setw(12) << "-----------" << std::setw(10) << "------"                  << std::setw(10) << "---------" << std::endl;
    out << std::setw(12) << "StateSet   " << std::setw(10) << _statesetSet.size()       << std::setw(10) << _numInstancedStateSet << std::endl;
    out << std::setw(12) << "Group      " << std::setw(10) << _groupSet.size()          << std::setw(10) << _numInstancedGroup << std::endl;
    out << std::setw(12) << "Transform  " << std::setw(10) << _transformSet.size()      << std::setw(10) << _numInstancedTransform << std::endl;
    out << std::setw(12) << "LOD        " << std::setw(10) << _lodSet.size()            << std::setw(10) << _numInstancedLOD << std::endl;
    out << std::setw(12) << "Switch     " << std::setw(10) << _switchSet.size()         << std::setw(10) << _numInstancedSwitch << std::endl;
    out << std::setw(12) << "Geode      " << std::setw(10) << _geodeSet.size()          << std::setw(10) << _numInstancedGeode << std::endl;
    out << std::setw(12) << "Drawable   " << std::setw(10) << _drawableSet.size()       << std::setw(10) << _numInstancedDrawable << std::endl;
    out << std::setw(12) << "Geometry   " << std::setw(10) << _geometrySet.size()       << std::setw(10) << _numInstancedGeometry << std::endl;
    out << std::setw(12) << "Fast geom. " << std::setw(10) << _fastGeometrySet.size()   << std::setw(10) << _numInstancedFastGeometry << std::endl;
    out << std::setw(12) << "Vertices   " << std::setw(10) << _uniqueStats._vertexCount << std::setw(10) << _instancedStats._vertexCount << std::endl;
    out << std::setw(12) << "Primitives " << std::setw(10) << unique_primitives         << std::setw(10) << instanced_primitives << std::endl;
}

