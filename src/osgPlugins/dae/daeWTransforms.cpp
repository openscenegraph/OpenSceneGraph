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

#include <osgAnimation/Bone>
#include <osgSim/DOFTransform>

#ifdef COLLADA_DOM_2_4_OR_LATER
#include <dom/domAny.h>
using namespace ColladaDOM141;
#endif

using namespace osgDAE;


void daeWriter::writeUpdateTransformElements(const osg::Vec3 &pos, const osg::Quat &q,    const osg::Vec3 &s)
{
    // Make a scale place element
    domScale *scale = daeSafeCast< domScale >( currentNode->add( COLLADA_ELEMENT_SCALE ) );
    scale->setSid("scale");
    scale->getValue().append3( s.x(), s.y(), s.z() );

    // Make a three rotate place elements for the euler angles
    // TODO decompose quaternion into three euler angles
    osg::Quat::value_type angle;
    osg::Vec3 axis;
    q.getRotate( angle, axis );

    domRotate *rot = daeSafeCast< domRotate >( currentNode->add( COLLADA_ELEMENT_ROTATE ) );
    rot->setSid("rotateZ");
    rot->getValue().append4( 0, 0, 1, osg::RadiansToDegrees(angle) );

    rot = daeSafeCast< domRotate >( currentNode->add( COLLADA_ELEMENT_ROTATE ) );
    rot->setSid("rotateY");
    rot->getValue().append4( 0, 1, 0, osg::RadiansToDegrees(angle) );

    rot = daeSafeCast< domRotate >( currentNode->add( COLLADA_ELEMENT_ROTATE ) );
    rot->setSid("rotateX");
    rot->getValue().append4( 1, 0, 0, osg::RadiansToDegrees(angle) );

    // Make a translate place element
    domTranslate *trans = daeSafeCast< domTranslate >( currentNode->add( COLLADA_ELEMENT_TRANSLATE ) );
    trans->setSid("translate");
    trans->getValue().append3( pos.x(), pos.y(), pos.z() );
}

//MATRIX
void daeWriter::apply( osg::MatrixTransform &node )
{
#ifdef _DEBUG
    debugPrint( node );
#endif
    updateCurrentDaeNode();
    currentNode = daeSafeCast< domNode >(currentNode->add( COLLADA_ELEMENT_NODE ) );
    std::string nodeName = getNodeName(node,"matrixTransform");
    currentNode->setId(nodeName.c_str());

    osg::Callback* ncb = node.getUpdateCallback();
    bool handled = false;
    if (ncb)
    {
        osgAnimation::UpdateMatrixTransform* ut = dynamic_cast<osgAnimation::UpdateMatrixTransform*>(ncb);
        // If targeted by an animation we split up the matrix into multiple place element so they can be targeted individually
        if (ut)
        {
            handled = true;

            const osg::Matrix &mat = node.getMatrix();

            // Note: though this is a generic matrix, based on the fact that it will be animated by and UpdateMatrixTransform,
            // we assume the initial matrix can be decomposed into translation, rotation and scale elements
            writeUpdateTransformElements(mat.getTrans(), mat.getRotate(), mat.getScale());
        }
    }

    // If not targeted by an animation simply write a single matrix place element
    if (!handled)
    {
        domMatrix *mat = daeSafeCast< domMatrix >(currentNode->add( COLLADA_ELEMENT_MATRIX ) );
        nodeName += "_matrix";
        mat->setSid(nodeName.c_str());

        const osg::Matrix::value_type *mat_vals = node.getMatrix().ptr();
        for ( int i = 0; i < 4; i++ )
        {
            for ( int j = 0; j < 4; j++ )
            {
                mat->getValue().append( mat_vals[i + j*4] );
            }
        }
    }

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
    updateCurrentDaeNode();
    currentNode = daeSafeCast< domNode >(currentNode->add( COLLADA_ELEMENT_NODE ) );
    std::string nodeName = getNodeName(node,"positionAttitudeTransform");
    currentNode->setId(nodeName.c_str());

    const osg::Vec3 &pos = node.getPosition();
    const osg::Quat &q = node.getAttitude();
    const osg::Vec3 &s = node.getScale();

    osg::Callback* ncb = node.getUpdateCallback();
    bool handled = false;
    if (ncb)
    {
        osgAnimation::UpdateMatrixTransform* ut = dynamic_cast<osgAnimation::UpdateMatrixTransform*>(ncb);
        // If targeted by an animation we split up the matrix into multiple place element so they can be targeted individually
        if (ut)
        {
            handled = true;

            writeUpdateTransformElements(pos, q, s);
        }
    }

    // If not targeted by an animation simply add the elements that actually contribute to placement
    if (!handled)
    {
        if ( s.x() != 1 || s.y() != 1 || s.z() != 1 )
        {
            // Make a scale place element
            domScale *scale = daeSafeCast< domScale >( currentNode->add( COLLADA_ELEMENT_SCALE ) );
            scale->setSid("scale");
            scale->getValue().append3( s.x(), s.y(), s.z() );
        }

        osg::Quat::value_type angle;
        osg::Vec3 axis;
        q.getRotate( angle, axis );
        if ( angle != 0 )
        {
            // Make a rotate place element
            domRotate *rot = daeSafeCast< domRotate >( currentNode->add( COLLADA_ELEMENT_ROTATE ) );
            rot->setSid("rotate");
            rot->getValue().append4( axis.x(), axis.y(), axis.z(), osg::RadiansToDegrees(angle) );
        }

    if ( s.x() != 1 || s.y() != 1 || s.z() != 1 )
        {
            // Make a translate place element
            domTranslate *trans = daeSafeCast< domTranslate >( currentNode->add( COLLADA_ELEMENT_TRANSLATE ) );
            trans->setSid("translate");
            trans->getValue().append3( pos.x(), pos.y(), pos.z() );
        }
    }

    writeNodeExtra(node);

    lastDepth = _nodePath.size();

    traverse( node );
}

void daeWriter::apply( osg::Transform &node )
{
    debugPrint( node );
    updateCurrentDaeNode();
    currentNode = daeSafeCast< domNode >(currentNode->add( COLLADA_ELEMENT_NODE ) );

    // If a DOFTransform node store it's data as extra "DOFTransform" data in the "OpenSceneGraph" technique
    osgSim::DOFTransform* dof = dynamic_cast<osgSim::DOFTransform*>(&node);
    if (_pluginOptions.writeExtras && dof)
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
        osgAnimation::Bone* bone = dynamic_cast<osgAnimation::Bone*>(&node);
        if (bone)
        {
            domNode *pDomNode = daeSafeCast< domNode >(currentNode->add( COLLADA_ELEMENT_NODE ));
            pDomNode->setType(NODETYPE_JOINT);
            pDomNode->setId(getNodeName(node, "bone").c_str());
        }
        else
        {
            std::string nodeName = getNodeName(node, "transform");
            currentNode->setId(nodeName.c_str());

            // Unknown transform type, just use local to world matrix
            osg::Matrix matrix;
            node.computeLocalToWorldMatrix(matrix, NULL);

            osg::Callback* ncb = node.getUpdateCallback();
            bool handled = false;
            if (ncb)
            {
                osgAnimation::UpdateMatrixTransform* ut = dynamic_cast<osgAnimation::UpdateMatrixTransform*>(ncb);
                // If targeted by an animation we split up the matrix into multiple place element so they can be targeted individually
                if (ut)
                {
                    handled = true;

                    // Note: though this is a generic matrix, based on the fact that it will be animated by and UpdateMatrixTransform,
                    // we assume the initial matrix can be decomposed into translation, rotation and scale elements
                    writeUpdateTransformElements(matrix.getTrans(), matrix.getRotate(), matrix.getScale());
                }
            }

            // If not targeted by an animation simply write a single matrix place element
            if (!handled)
            {
                domMatrix *mat = daeSafeCast< domMatrix >(currentNode->add( COLLADA_ELEMENT_MATRIX ) );
                nodeName += "_matrix";
                mat->setSid(nodeName.c_str());

                const osg::Matrix::value_type *mat_vals = matrix.ptr();
                for ( int i = 0; i < 4; i++ )
                {
                    for ( int j = 0; j < 4; j++ )
                    {
                        mat->getValue().append( mat_vals[i + j*4] );
                    }
                }
            }
        }
    }

    writeNodeExtra(node);

    lastDepth = _nodePath.size();

    traverse( node );
}

void daeWriter::apply( osg::CoordinateSystemNode &node )
{
    OSG_WARN << "CoordinateSystemNode. Missing " << node.getNumChildren() << " children" << std::endl;
}


