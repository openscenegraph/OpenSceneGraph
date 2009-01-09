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
#include <dae/domAny.h>

#include <osgSim/DOFTransform>

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

    currentNode = daeSafeCast< domNode >(currentNode->add( COLLADA_ELEMENT_NODE ) );
    currentNode->setId(getNodeName(node,"matrixTransform").c_str());
    
    domMatrix *mat = daeSafeCast< domMatrix >(currentNode->add( COLLADA_ELEMENT_MATRIX ) );
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

    lastDepth = _nodePath.size();

    writeNodeExtra(node);

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
    currentNode = daeSafeCast< domNode >(currentNode->add( COLLADA_ELEMENT_NODE ) );
    currentNode->setId(getNodeName(node,"positionAttitudeTransform").c_str());
    
    const osg::Vec3 &pos = node.getPosition();
    const osg::Quat &q = node.getAttitude();
    const osg::Vec3 &s = node.getScale();

    if ( pos.x() != 0 || pos.y() != 0 || pos.z() != 0 )
    {
        //make a translate
        domTranslate *trans = daeSafeCast< domTranslate >( currentNode->add( COLLADA_ELEMENT_TRANSLATE ) );
        trans->getValue().append( pos.x() );
        trans->getValue().append( pos.y() );
        trans->getValue().append( pos.z() );
    }

    double angle;
    osg::Vec3 axis;
    q.getRotate( angle, axis );
    if ( angle != 0 )
    {
        //make a rotate
        domRotate *rot = daeSafeCast< domRotate >( currentNode->add( COLLADA_ELEMENT_ROTATE ) );
        rot->getValue().append( axis.x() );
        rot->getValue().append( axis.y() );
        rot->getValue().append( axis.z() );
        rot->getValue().append( osg::RadiansToDegrees(angle) );
    }

    if ( s.x() != 1 || s.y() != 1 || s.z() != 1 )
    {
        //make a scale
        domScale *scale = daeSafeCast< domScale >( currentNode->add( COLLADA_ELEMENT_SCALE ) );
        scale->getValue().append( s.x() );
        scale->getValue().append( s.y() );
        scale->getValue().append( s.z() );
    }

    writeNodeExtra(node);

    lastDepth = _nodePath.size();

    traverse( node );
}

void daeWriter::apply( osg::Transform &node ) 
{
    debugPrint( node );

    while ( lastDepth >= _nodePath.size() )
    {
        // We are not a child of previous node
        currentNode = daeSafeCast< domNode >( currentNode->getParentElement() );
        lastDepth--;
    }
    currentNode = daeSafeCast< domNode >(currentNode->add( COLLADA_ELEMENT_NODE ) );
    
    // If a DOFTransform node store it's data as extra "DOFTransform" data in the "OpenSceneGraph" technique
    osgSim::DOFTransform* dof = dynamic_cast<osgSim::DOFTransform*>(&node);
    if (writeExtras && dof)
    {
        // Adds the following to a node

        //<extra type="DOFTransform">
        //    <technique profile="OpenSceneGraph">
        //        <MinHPR>0 -0.174533 0</MinHPR>
        //        <MaxHPR>0 0.872665 0</MaxHPR>
        //        <IncrementHPR>0 0.0174533 0</IncrementHPR>
        //        <CurrentHPR>0 0 0</CurrentHPR>
        //        <MinTranslate>0 0 0</MinTranslate>
        //        <MaxTranslate>0 0 0</MaxTranslate>
        //        <IncrementTranslate>0 0 0</IncrementTranslate>
        //        <CurrentTranslate>0 0 0</CurrentTranslate>
        //        <MinScale>0 0 0</MinScale>
        //        <MaxScale>1 1 1</MaxScale>
        //        <IncrementScale>0 0 0</IncrementScale>
        //        <CurrentScale>1 1 1</CurrentScale>
        //        <MultOrder>0</MultOrder>
        //        <LimitationFlags>269964960</LimitationFlags>
        //        <AnimationOn>0</AnimationOn>
        //        <PutMatrix>
        //            1 0 0 0
        //            0 1 0 0
        //            0 0 1 0
        //            0 0 0 1
        //        </PutMatrix>
        //    </technique>
        //</extra>

        domExtra *extra = daeSafeCast<domExtra>(currentNode->add( COLLADA_ELEMENT_EXTRA ));
        extra->setType("DOFTransform");
        domTechnique *teq = daeSafeCast<domTechnique>(extra->add( COLLADA_ELEMENT_TECHNIQUE ) );
        teq->setProfile( "OpenSceneGraph" );

        domAny *minHPR = (domAny*)teq->add("MinHPR" );
        minHPR->setValue(toString(dof->getMinHPR()).c_str());

        domAny *maxHPR = (domAny*)teq->add("MaxHPR" );
        maxHPR->setValue(toString(dof->getMaxHPR()).c_str());

        domAny *incrementHPR = (domAny*)teq->add("IncrementHPR" );
        incrementHPR->setValue(toString(dof->getIncrementHPR()).c_str());

        domAny *currentHPR = (domAny*)teq->add("CurrentHPR" );
        currentHPR->setValue(toString(dof->getCurrentHPR()).c_str());

        domAny *minTranslate = (domAny*)teq->add("MinTranslate" );
        minTranslate->setValue(toString(dof->getMinTranslate()).c_str());

        domAny *maxTranslate = (domAny*)teq->add("MaxTranslate" );
        maxTranslate->setValue(toString(dof->getMaxTranslate()).c_str());

        domAny *incrementTranslate = (domAny*)teq->add("IncrementTranslate" );
        incrementTranslate->setValue(toString(dof->getIncrementTranslate()).c_str());

        domAny *currentTranslate = (domAny*)teq->add("CurrentTranslate" );
        currentTranslate->setValue(toString(dof->getCurrentTranslate()).c_str());

        domAny *minScale = (domAny*)teq->add("MinScale" );
        minScale->setValue(toString(dof->getMinScale()).c_str());

        domAny *maxScale = (domAny*)teq->add("MaxScale" );
        maxScale->setValue(toString(dof->getMaxScale()).c_str());

        domAny *incrementScale = (domAny*)teq->add("IncrementScale" );
        incrementScale->setValue(toString(dof->getIncrementScale()).c_str());

        domAny *currentScale = (domAny*)teq->add("CurrentScale" );
        currentScale->setValue(toString(dof->getCurrentScale()).c_str());

        domAny *multOrder = (domAny*)teq->add("MultOrder" );
        multOrder->setValue(toString<int>(dof->getHPRMultOrder()).c_str());

        domAny *limitationFlags = (domAny*)teq->add("LimitationFlags" );
        limitationFlags->setValue(toString<unsigned long>(dof->getLimitationFlags()).c_str());

        domAny *animationOn = (domAny*)teq->add("AnimationOn" );
        animationOn->setValue(toString<bool>(dof->getAnimationOn()).c_str());

        domAny *putMatrix = (domAny*)teq->add("PutMatrix" );
        putMatrix->setValue(toString(dof->getPutMatrix()).c_str());        

        currentNode->setId(getNodeName(node, "doftransform").c_str());
    }
    else
    {
        currentNode->setId(getNodeName(node, "transform").c_str());
        osg::notify( osg::WARN ) << "some other transform type. Missing " << node.getNumChildren() << " children" << std::endl;
    }

    writeNodeExtra(node);

    lastDepth = _nodePath.size();

    traverse( node );
}

void daeWriter::apply( osg::CoordinateSystemNode &node ) 
{
    osg::notify( osg::WARN ) << "CoordinateSystemNode. Missing " << node.getNumChildren() << " children" << std::endl;
}
