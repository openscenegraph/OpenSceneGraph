/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2014 Robert Osfield
 *  Copyright (C) 2014 Pawel Ksiezopolski
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
 *
*/
#include "AggregateGeometryVisitor.h"

AggregateGeometryVisitor::AggregateGeometryVisitor( ConvertTrianglesOperator* ctOperator )
    : osg::NodeVisitor( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
{
    _ctOperator.setConverter(ctOperator);
    init();
}

void AggregateGeometryVisitor::init()
{
    _aggregatedGeometry = new osg::Geometry;
    _ctOperator.initGeometry( _aggregatedGeometry.get() );
    _matrixStack.clear();
}

AggregateGeometryVisitor::AddObjectResult AggregateGeometryVisitor::addObject( osg::Node* object, unsigned int typeID, unsigned int lodNumber )
{
    unsigned int currentVertexFirst = _aggregatedGeometry->getVertexArray()->getNumElements();
    _currentTypeID                  = typeID;
    _currentLodNumber               = lodNumber;
    object->accept(*this);
    unsigned int currentVertexCount = _aggregatedGeometry->getVertexArray()->getNumElements() - currentVertexFirst;
    _aggregatedGeometry->addPrimitiveSet( new osg::DrawArrays( osg::PrimitiveSet::TRIANGLES, currentVertexFirst, currentVertexCount ) );
    _matrixStack.clear();
    return AddObjectResult( currentVertexFirst, currentVertexCount, _aggregatedGeometry->getNumPrimitiveSets()-1 );
}

void AggregateGeometryVisitor::apply( osg::Node& node )
{
    bool pushed = _ctOperator.pushNode(&node);
    traverse(node);
    if( pushed ) _ctOperator.popNode();
}

void AggregateGeometryVisitor::apply( osg::Transform& transform )
{
    bool pushed = _ctOperator.pushNode(&transform);
    osg::Matrix matrix;
    if( !_matrixStack.empty() )
        matrix = _matrixStack.back();
    transform.computeLocalToWorldMatrix( matrix, this );
    _matrixStack.push_back( matrix );
    
    traverse(transform);    
    
    _matrixStack.pop_back();
    if( pushed ) _ctOperator.popNode();
}

void AggregateGeometryVisitor::apply( osg::Geode& geode )
{
    bool pushed = _ctOperator.pushNode(&geode);
    
    osg::Matrix matrix;
    if(! _matrixStack.empty() )
        matrix = _matrixStack.back();
    for( unsigned int i=0; i<geode.getNumDrawables(); ++i)
    {
        osg::Geometry* geom = geode.getDrawable(i)->asGeometry();
        if ( geom != NULL )
        {
            _ctOperator.setGeometryData( matrix, geom, _aggregatedGeometry.get(), (float) _currentTypeID, (float) _currentLodNumber  );
            geom->accept( _ctOperator );
        }
    }
    
    traverse(geode);
    if( pushed ) _ctOperator.popNode();
}
