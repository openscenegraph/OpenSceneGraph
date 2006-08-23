/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the SCEA Shared Source License, Version 1.0 (the "License"); you may not use this 
 * file except in compliance with the License. You may obtain a copy of the License at:
 * http://research.scea.com/scea_shared_source_license.html
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License 
 * is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or 
 * implied. See the License for the specific language governing permissions and limitations under the 
 * License. 
 */

#include "daeWriter.h"

#include <dom/domCOLLADA.h>

#include <dom/domNode.h>
#include <dom/domConstants.h>

using namespace osgdae;

//MATRIX
void daeWriter::apply( osg::MatrixTransform &node )
{
#ifdef _DEBUG
    debugPrint( node );
#endif

    while ( lastDepth >= _nodePath.size() )
    {
        //We are not a child of previous node
        currentNode = daeSafeCast< domNode >( currentNode->getParentElement() );
        lastDepth--;
    }

    currentNode = daeSafeCast< domNode >(currentNode->createAndPlace( COLLADA_ELEMENT_NODE ) );
    currentNode->setId(getNodeName(node,"matrixTransform").c_str());
    
    domMatrix *mat = daeSafeCast< domMatrix >(currentNode->createAndPlace( COLLADA_ELEMENT_MATRIX ) );
    const osg::Matrix::value_type *mat_vals = node.getMatrix().ptr();
    //for ( int i = 0; i < 16; i++ )
    //{
    //  mat->getValue().append( mat_vals[i] );
    //}
    mat->getValue().append( mat_vals[0] );
    mat->getValue().append( mat_vals[4] );
    mat->getValue().append( mat_vals[8] );
    mat->getValue().append( mat_vals[12] );
    mat->getValue().append( mat_vals[1] );
    mat->getValue().append( mat_vals[5] );
    mat->getValue().append( mat_vals[9] );
    mat->getValue().append( mat_vals[13] );
    mat->getValue().append( mat_vals[2] );
    mat->getValue().append( mat_vals[6] );
    mat->getValue().append( mat_vals[10] );
    mat->getValue().append( mat_vals[14] );
    mat->getValue().append( mat_vals[3] );
    mat->getValue().append( mat_vals[7] );
    mat->getValue().append( mat_vals[11] );
    mat->getValue().append( mat_vals[15] );

    lastVisited = MATRIX;
    lastDepth = _nodePath.size();

    traverse( node );
}

//POSATT
void daeWriter::apply( osg::PositionAttitudeTransform &node )
{
#ifdef _DEBUG
    debugPrint( node );
#endif

    while ( lastDepth >= _nodePath.size() )
    {
        //We are not a child of previous node
        currentNode = daeSafeCast< domNode >( currentNode->getParentElement() );
        lastDepth--;
    }
    currentNode = daeSafeCast< domNode >(currentNode->createAndPlace( COLLADA_ELEMENT_NODE ) );
    currentNode->setId(getNodeName(node,"positionAttitudeTransform").c_str());
    
    const osg::Vec3 &pos = node.getPosition();
    const osg::Quat &q = node.getAttitude();
    const osg::Vec3 &s = node.getScale();

    if ( s.x() != 1 || s.y() != 1 || s.z() != 1 )
    {
        //make a scale
        domScale *scale = daeSafeCast< domScale >( currentNode->createAndPlace( COLLADA_ELEMENT_SCALE ) );
        scale->getValue().append( s.x() );
        scale->getValue().append( s.y() );
        scale->getValue().append( s.z() );
    }

    double angle;
    osg::Vec3 axis;
    q.getRotate( angle, axis );
    if ( angle != 0 )
    {
        //make a rotate
        domRotate *rot = daeSafeCast< domRotate >( currentNode->createAndPlace( COLLADA_ELEMENT_ROTATE ) );
        rot->getValue().append( axis.x() );
        rot->getValue().append( axis.y() );
        rot->getValue().append( axis.z() );
        rot->getValue().append( angle );
    }

    if ( pos.x() != 0 || pos.y() != 0 || pos.z() != 0 )
    {
        //make a translate
        domTranslate *trans = daeSafeCast< domTranslate >( currentNode->createAndPlace( COLLADA_ELEMENT_TRANSLATE ) );
        trans->getValue().append( pos.x() );
        trans->getValue().append( pos.y() );
        trans->getValue().append( pos.z() );
    }

    lastVisited = POSATT;
    lastDepth = _nodePath.size();

    traverse( node );
}

void daeWriter::apply( osg::Transform &node ) 
{
    osg::notify( osg::WARN ) << "some other transform type. Missing " << node.getNumChildren() << " children\n";
}

void daeWriter::apply( osg::CoordinateSystemNode &node ) 
{
    osg::notify( osg::WARN ) << "CoordinateSystemNode. Missing " << node.getNumChildren() << " children\n";
}
