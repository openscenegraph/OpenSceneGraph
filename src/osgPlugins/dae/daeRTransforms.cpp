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

#include "daeReader.h"
#include <dae.h>
#include <dae/domAny.h>
#include <dom/domCOLLADA.h>

#include <osgAnimation/UpdateMatrixTransform>
#include <osgAnimation/StackedMatrixElement>
#include <osgAnimation/StackedRotateAxisElement>
#include <osgAnimation/StackedScaleElement>
#include <osgAnimation/StackedTranslateElement>
#include <osg/MatrixTransform>
#include <osgSim/DOFTransform>

#ifdef COLLADA_DOM_2_4_OR_LATER
#include <dom/domAny.h>
using namespace ColladaDOM141;
#endif

using namespace osgDAE;

// Note <lookat>, <matrix>, <rotate>, <scale>, <skew> and <translate> may appear in any order
// These transformations can be combined in any number and ordering to produce the desired
// coordinate system for the parent <node> element. The COLLADA specificatin requires that the
// transformation elements are processed in order and accumulate the result as if they were
// converted to column-order matrices and concatenated using matrix post-multiplication.
osg::Transform* daeReader::processOsgMatrixTransform(domNode *node, bool isBone)
{
    osg::MatrixTransform* resultNode = NULL;

    if (isBone)
    {
        resultNode = getOrCreateBone(node);
    }
    else
    {
        resultNode = new osg::MatrixTransform;
    }

    osg::Callback* pNodeCallback = resultNode->getUpdateCallback();
    std::vector<osg::ref_ptr<osgAnimation::StackedTransformElement> > transformElements;
    osg::ref_ptr<osgAnimation::StackedTransformElement> pLastStaticTransformElement;

    // Process all coordinate system contributing elements in order!
    size_t count = node->getContents().getCount();
    for (size_t i = 0; i < count; i++ )
    {
        daeElement* pDaeElement = node->getContents()[i];
        osg::ref_ptr<osgAnimation::StackedTransformElement> pTransformElement = NULL;

        if (domRotate * pDomRotate = daeSafeCast< domRotate >( pDaeElement ))
        {
            const domFloat4& r = pDomRotate->getValue();
            if (r.getCount() != 4 )
            {
                OSG_WARN << "Data is wrong size for rotate" << std::endl;
                continue;
            }

            pTransformElement = new osgAnimation::StackedRotateAxisElement(pDomRotate->getSid() ? pDomRotate->getSid() : "", osg::Vec3(r[0], r[1], r[2]), osg::DegreesToRadians(r[3]));
        }
        else if (domTranslate * pDomTranslate = daeSafeCast< domTranslate >( pDaeElement ))
        {
            const domFloat3& t = pDomTranslate->getValue();
            if (t.getCount() != 3 )
            {
                OSG_WARN<<"Data is wrong size for translate"<<std::endl;
                continue;
            }

            pTransformElement = new osgAnimation::StackedTranslateElement(pDomTranslate->getSid() ?  pDomTranslate->getSid() : "", osg::Vec3(t[0], t[1], t[2]));
        }
        else if (domScale * pDomScale = daeSafeCast< domScale >( pDaeElement ))
        {
            const domFloat3& s = pDomScale->getValue();
            if (s.getCount() != 3 )
            {
                OSG_WARN<<"Data is wrong size for scale"<<std::endl;
                continue;
            }

            pTransformElement = new osgAnimation::StackedScaleElement(pDomScale->getSid() ? pDomScale->getSid() : "", osg::Vec3(s[0], s[1], s[2]));
        }
        else if (domMatrix * pDomMatrix = daeSafeCast< domMatrix >( pDaeElement ))
        {
            if (pDomMatrix->getValue().getCount() != 16 )
            {
                OSG_WARN<<"Data is wrong size for matrix"<<std::endl;
                continue;
            }

            pTransformElement = new osgAnimation::StackedMatrixElement(pDomMatrix->getSid() ? pDomMatrix->getSid() : "",
                osg::Matrix(    pDomMatrix->getValue()[0], pDomMatrix->getValue()[4], pDomMatrix->getValue()[8], pDomMatrix->getValue()[12],
                pDomMatrix->getValue()[1], pDomMatrix->getValue()[5], pDomMatrix->getValue()[9], pDomMatrix->getValue()[13],
                pDomMatrix->getValue()[2], pDomMatrix->getValue()[6], pDomMatrix->getValue()[10], pDomMatrix->getValue()[14],
                pDomMatrix->getValue()[3], pDomMatrix->getValue()[7], pDomMatrix->getValue()[11], pDomMatrix->getValue()[15]));
        }
        else if (domLookat * pDomLookat = daeSafeCast< domLookat >( pDaeElement ))
        {
            if (pDomLookat->getValue().getCount() != 9 )
            {
                OSG_WARN<<"Data is wrong size for lookat"<<std::endl;
                continue;
            }

            pTransformElement = new osgAnimation::StackedMatrixElement(pDomLookat->getSid() ? pDomLookat->getSid() : "",
                osg::Matrix::lookAt(
                osg::Vec3(pDomLookat->getValue()[0], pDomLookat->getValue()[1], pDomLookat->getValue()[2]),
                osg::Vec3(pDomLookat->getValue()[3], pDomLookat->getValue()[4], pDomLookat->getValue()[5]),
                osg::Vec3(pDomLookat->getValue()[6], pDomLookat->getValue()[7], pDomLookat->getValue()[8])));
        }
        else if (domSkew * pDomSkew = daeSafeCast< domSkew >( pDaeElement ))
        {
            if (pDomSkew->getValue().getCount() != 7 )
            {
                OSG_WARN<<"Data is wrong size for skew"<<std::endl;
                continue;
            }

            const domFloat7& s = pDomSkew->getValue();

            float shear = sin(osg::DegreesToRadians(s[0]));
            // axis of rotation
            osg::Vec3f around(s[1],s[2],s[3]);
            // axis of translation
            osg::Vec3f along(s[4],s[5],s[6]);

            //This maths is untested so may be transposed or negated or just completely wrong.
            osg::Vec3f normal = along ^ around;
            normal.normalize();
            around.normalize();
            along *= shear / along.length();

            pTransformElement = new osgAnimation::StackedMatrixElement(pDomSkew->getSid() ? pDomSkew->getSid() : "",
                osg::Matrix(
                normal.x() * along.x() + 1.0f, normal.x() * along.y(), normal.x() * along.z(), 0.0f,
                normal.y() * along.x(), normal.y() * along.y() + 1.0f, normal.y() * along.z(), 0.0f,
                normal.z() * along.x(), normal.z() * along.y(), normal.z() * along.z() + 1.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f));
        }

        if (pTransformElement)
        {
            daeElementDomChannelMap::iterator iter = _daeElementDomChannelMap.find(pDaeElement);
            if (iter != _daeElementDomChannelMap.end())
            {
                // The element is animated

                // First add single or collapsed transform element if any
                if (pLastStaticTransformElement)
                {
                    transformElements.push_back(pLastStaticTransformElement);
                    pLastStaticTransformElement = NULL;
                }
                transformElements.push_back(pTransformElement);

                // Animated element so we need an AnimationUpdateCallback
                if (!pNodeCallback)
                {
                    std::string name = node->getId() ? node->getId() : node->getSid() ? node->getSid() : "";
                    resultNode->setDataVariance(osg::Object::DYNAMIC);

                    pNodeCallback = new osgAnimation::UpdateMatrixTransform(name);
                    resultNode->setUpdateCallback(pNodeCallback);
                }

                do
                {
                    _domChannelOsgAnimationUpdateCallbackMap[iter->second] = pNodeCallback;
                    ++iter;
                } while (iter != _daeElementDomChannelMap.end() && iter->first == pDaeElement);
            }
            else if (pLastStaticTransformElement)
            {
                // Add transform element only if not identity
                if (!pTransformElement->isIdentity())
                {
                    // Collapse static transform elements
                    osg::Matrix matrix = pLastStaticTransformElement->getAsMatrix();
                    pTransformElement->applyToMatrix(matrix);
                    pLastStaticTransformElement = new osgAnimation::StackedMatrixElement("collapsed", matrix);
                }
            }
            else if (!pTransformElement->isIdentity())
            {
                // Store single static transform element only if not identity
                pLastStaticTransformElement = pTransformElement;
            }
        }
    }

    // Add final collapsed element (if any)
    if (pLastStaticTransformElement)
    {
        transformElements.push_back(pLastStaticTransformElement);
    }

    // Build a matrix for the MatrixTransform and add the elements to the updateCallback
    osg::Matrix matrix;

    osgAnimation::UpdateMatrixTransform* pUpdateStackedTransform =
        dynamic_cast<osgAnimation::UpdateMatrixTransform*>(pNodeCallback);

    for (size_t i=0; i < transformElements.size(); i++)
    {
        transformElements[i]->applyToMatrix(matrix);
        if (pUpdateStackedTransform)
        {
            pUpdateStackedTransform->getStackedTransforms().push_back(transformElements[i].get());
        }
    }

    resultNode->setMatrix(matrix);

    osg::Vec3 scale = matrix.getScale();
    if ((scale.x() != 1) || (scale.y() != 1) || (scale.z() != 1))
    {
        osg::StateSet* ss = resultNode->getOrCreateStateSet();
        if (scale.x() == scale.y() && scale.y() == scale.z())
        {
            // This mode may be quicker than GL_NORMALIZE, but ONLY works if x, y & z components of scale are the same.
            ss->setMode(GL_RESCALE_NORMAL, osg::StateAttribute::ON|osg::StateAttribute::OVERRIDE);
        }
        else
        {
            // This mode may be slower than GL_RESCALE_NORMAL, but does work if x, y & z components of scale are not the same.
            ss->setMode(GL_NORMALIZE, osg::StateAttribute::ON|osg::StateAttribute::OVERRIDE);
        }
    }

    return resultNode;
}

osg::Group* daeReader::processOsgDOFTransform(domTechnique* teq)
{
    osgSim::DOFTransform* dof = new osgSim::DOFTransform;

    domAny* any = daeSafeCast< domAny >(teq->getChild("MinHPR"));
    if (any)
    {
        dof->setMinHPR(parseVec3String(any->getValue()));
    }
    else
    {
        OSG_WARN << "Expected element 'MinHPR' not found" << std::endl;
    }

    any = daeSafeCast< domAny >(teq->getChild("MaxHPR"));
    if (any)
    {
        dof->setMaxHPR(parseVec3String(any->getValue()));
    }
    else
    {
        OSG_WARN << "Expected element 'MaxHPR' not found" << std::endl;
    }

    any = daeSafeCast< domAny >(teq->getChild("IncrementHPR"));
    if (any)
    {
        dof->setIncrementHPR(parseVec3String(any->getValue()));
    }
    else
    {
        OSG_WARN << "Expected element 'IncrementHPR' not found" << std::endl;
    }

    any = daeSafeCast< domAny >(teq->getChild("CurrentHPR"));
    if (any)
    {
        dof->setCurrentHPR(parseVec3String(any->getValue()));
    }
    else
    {
        OSG_WARN << "Expected element 'CurrentHPR' not found" << std::endl;
    }

    any = daeSafeCast< domAny >(teq->getChild("MinTranslate"));
    if (any)
    {
        dof->setMinTranslate(parseVec3String(any->getValue()));
    }
    else
    {
        OSG_WARN << "Expected element 'MinTranslate' not found" << std::endl;
    }

    any = daeSafeCast< domAny >(teq->getChild("MaxTranslate"));
    if (any)
    {
        dof->setMaxTranslate(parseVec3String(any->getValue()));
    }
    else
    {
        OSG_WARN << "Expected element 'MaxTranslate' not found" << std::endl;
    }

    any = daeSafeCast< domAny >(teq->getChild("IncrementTranslate"));
    if (any)
    {
        dof->setIncrementTranslate(parseVec3String(any->getValue()));
    }
    else
    {
        OSG_WARN << "Expected element 'IncrementTranslate' not found" << std::endl;
    }

    any = daeSafeCast< domAny >(teq->getChild("CurrentTranslate"));
    if (any)
    {
        dof->setCurrentTranslate(parseVec3String(any->getValue()));
    }
    else
    {
        OSG_WARN << "Expected element 'CurrentTranslate' not found" << std::endl;
    }

    any = daeSafeCast< domAny >(teq->getChild("MinScale"));
    if (any)
    {
        dof->setMinScale(parseVec3String(any->getValue()));
    }
    else
    {
        OSG_WARN << "Expected element 'MinScale' not found" << std::endl;
    }

    any = daeSafeCast< domAny >(teq->getChild("MaxScale"));
    if (any)
    {
        dof->setMaxScale(parseVec3String(any->getValue()));
    }
    else
    {
        OSG_WARN << "Expected element 'MaxScale' not found" << std::endl;
    }

    any = daeSafeCast< domAny >(teq->getChild("IncrementScale"));
    if (any)
    {
        dof->setIncrementScale(parseVec3String(any->getValue()));
    }
    else
    {
        OSG_WARN << "Expected element 'IncrementScale' not found" << std::endl;
    }

    any = daeSafeCast< domAny >(teq->getChild("CurrentScale"));
    if (any)
    {
        dof->setCurrentScale(parseVec3String(any->getValue()));
    }
    else
    {
        OSG_WARN << "Expected element 'CurrentScale' not found" << std::endl;
    }

    any = daeSafeCast< domAny >(teq->getChild("MultOrder"));
    if (any)
    {
        dof->setHPRMultOrder((osgSim::DOFTransform::MultOrder)parseString<int>(any->getValue()));
    }
    else
    {
        OSG_WARN << "Expected element 'MultOrder' not found" << std::endl;
    }

    any = daeSafeCast< domAny >(teq->getChild("LimitationFlags"));
    if (any)
    {
        dof->setLimitationFlags(parseString<unsigned long>(any->getValue()));
    }
    else
    {
        OSG_WARN << "Expected element 'LimitationFlags' not found" << std::endl;
    }

    any = daeSafeCast< domAny >(teq->getChild("AnimationOn"));
    if (any)
    {
        dof->setAnimationOn(parseString<bool>(any->getValue()));
    }
    else
    {
        OSG_WARN << "Expected element 'AnimationOn' not found" << std::endl;
    }

    any = daeSafeCast< domAny >(teq->getChild("PutMatrix"));
    if (any)
    {
        osg::Matrix mat = parseMatrixString(any->getValue());
        dof->setPutMatrix(mat);
        dof->setInversePutMatrix( osg::Matrixd::inverse( mat ) );
    }
    else
    {
        OSG_WARN << "Expected element 'PutMatrix' not found" << std::endl;
    }

    return dof;
}
