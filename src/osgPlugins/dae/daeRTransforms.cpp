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

#include <osg/MatrixTransform>
#include <osgSim/DOFTransform>

using namespace osgdae;

// Note <lookat>, <matrix>, <rotate>, <scale>, <skew> and <translate> may appear in any order
// These transformations can be combined in any number and ordering to produce the desired
// coordinate systemfor the parent <node> element. The COLLADA specificatin requires that the
// transformation elements are processed in order and accumulate the result as if they were 
// converted to column-order matrices and concatenated using matrix post-multiplication.
osg::Node* daeReader::processOsgMatrixTransform( domNode *node )
{
    osg::MatrixTransform* matNode = new osg::MatrixTransform;
    osg::Matrix matrix;

    // Process all coordinate system contributing elements in order!
    size_t count = node->getContents().getCount();
    for (size_t i = 0; i < count; i++ ) 
    {
        domRotate * rot = daeSafeCast< domRotate >( node->getContents()[i] );
        if (rot)
        {
            domFloat4& r = rot->getValue();
            if (r.getCount() != 4 ) 
            {
                osg::notify(osg::WARN)<<"Data is wrong size for rotate"<<std::endl;
                continue;
            }

            // Build rotation matrix
            osg::Matrix rotMat;
            rotMat.makeRotate(osg::DegreesToRadians(r[3]), r[0], r[1], r[2]);

            matrix = rotMat * matrix;
            continue;
        }

        domTranslate * trans = daeSafeCast< domTranslate >( node->getContents()[i] );
        if (trans != NULL)
        {
            domFloat3& t = trans->getValue();
            if (t.getCount() != 3 ) 
            {
                osg::notify(osg::WARN)<<"Data is wrong size for translate"<<std::endl;
                continue;
            }

            // Build translation matrix
            osg::Matrix transMat;
            transMat.makeTranslate(t[0], t[1], t[2]);

            matrix = transMat * matrix;
            continue;
        }

        domScale * scale = daeSafeCast< domScale >( node->getContents()[i] );
        if (scale != NULL)
        {
            domFloat3& s = scale->getValue();
            if (s.getCount() != 3 ) 
            {
                osg::notify(osg::WARN)<<"Data is wrong size for scale"<<std::endl;
                continue;
            }

            // Build scale matrix
            osg::Matrix scaleMat;
            scaleMat.makeScale(s[0], s[1], s[2]);

            matrix = scaleMat * matrix;
            continue;
        }

        domMatrix * mat = daeSafeCast< domMatrix >( node->getContents()[i] );
        if (mat != NULL)
        {
            if (mat->getValue().getCount() != 16 ) 
            {
                osg::notify(osg::WARN)<<"Data is wrong size for matrix"<<std::endl;
                continue;
            }

            // Build matrix
            osg::Matrix mMat(    mat->getValue()[0], mat->getValue()[4], mat->getValue()[8], mat->getValue()[12],
                                mat->getValue()[1], mat->getValue()[5], mat->getValue()[9], mat->getValue()[13],
                                mat->getValue()[2], mat->getValue()[6], mat->getValue()[10], mat->getValue()[14],
                                mat->getValue()[3], mat->getValue()[7], mat->getValue()[11], mat->getValue()[15] );

            matrix = mMat * matrix;
            continue;
        }

        domLookat * la = daeSafeCast< domLookat >( node->getContents()[i] );
        if (la != NULL)
        {
            if (la->getValue().getCount() != 9 ) 
            {
                osg::notify(osg::WARN)<<"Data is wrong size for lookat"<<std::endl;
                continue;
            }

            // Build lookat matrix
            osg::Matrix lookatMat;
            osg::Vec3 eye(la->getValue()[0], la->getValue()[1], la->getValue()[2]);
            osg::Vec3 center(la->getValue()[3], la->getValue()[4], la->getValue()[5] );
            osg::Vec3 up( la->getValue()[6], la->getValue()[7], la->getValue()[8] );
            lookatMat.makeLookAt( eye, center, up );
            
            matrix = lookatMat * matrix;
            continue;
        }

        domSkew * skew = daeSafeCast< domSkew >( node->getContents()[i] );
        if (skew != NULL)
        {
            if (skew->getValue().getCount() != 7 ) 
            {
                osg::notify(osg::WARN)<<"Data is wrong size for skew"<<std::endl;
                continue;
            }

            // Skew matrix building derived from GNURealistic ShaderMan GMANMatrix4 (LGPL) matrix class

            // Build skew matrix
            domFloat7& s = skew->getValue();

            float shear = sin(osg::DegreesToRadians(s[0]));
            // axis of rotation
            osg::Vec3f around(s[1],s[2],s[3]);
            // axis of translation
            osg::Vec3f along(s[4],s[5],s[6]);

            along.normalize();
            osg::Vec3f a = around - (along * (around * along));
            a.normalize();
            
            float an1 = around * a;
            float an2 = around * along;
            
            float rx = an1 * cos(shear) - an2 * sin(shear);
            float ry = an1 * sin(shear) + an2 * cos(shear);

            if (rx <= 0.0) 
            {
                osg::notify(osg::WARN)<<"skew angle too large"<<std::endl;
                continue;
            }
            
            float alpha;
            // A parallel to B??
            if (an1==0) 
            {
                alpha=0;
            } 
            else 
            {
                alpha=ry/rx-an2/an1;
            }


            osg::Matrix skewMat(a.x()*along.x()*alpha+1.0,    a.x()*along.y()*alpha,        a.x()*along.z()*alpha,        0,
                                a.y()*along.x()*alpha,        a.y()*along.y()*alpha+1.0,    a.y()*along.z()*alpha,        0,
                                a.z()*along.x()*alpha,        a.z()*along.y()*alpha,        a.z()*along.z()*alpha+1.0,    0,
                                0,                            0,                            0,                            1);


            matrix = skewMat * matrix;
            continue;
        }
    }

    matNode->setMatrix(matrix);

    osg::Vec3 scale = matrix.getScale();
    if ((scale.x() != 1) || (scale.y() != 1) || (scale.z() != 1))
    {
        osg::StateSet* ss = matNode->getOrCreateStateSet();
        ss->setMode(GL_RESCALE_NORMAL, osg::StateAttribute::ON|osg::StateAttribute::OVERRIDE);
    }

    return matNode;
}

osg::Node* daeReader::processOsgDOFTransform(domTechnique* teq)
{
    osgSim::DOFTransform* dof = new osgSim::DOFTransform;

    domAny* any = daeSafeCast< domAny >(teq->getChild("MinHPR"));
    if (any)
    {
        dof->setMinHPR(parseVec3String(any->getValue()));
    }
    else
    {
        osg::notify(osg::WARN) << "Expected element 'MinHPR' not found" << std::endl;
    }

    any = daeSafeCast< domAny >(teq->getChild("MaxHPR"));
    if (any)
    {
        dof->setMaxHPR(parseVec3String(any->getValue()));
    }
    else
    {
        osg::notify(osg::WARN) << "Expected element 'MaxHPR' not found" << std::endl;
    }

    any = daeSafeCast< domAny >(teq->getChild("IncrementHPR"));
    if (any)
    {
        dof->setIncrementHPR(parseVec3String(any->getValue()));
    }
    else
    {
        osg::notify(osg::WARN) << "Expected element 'IncrementHPR' not found" << std::endl;
    }

    any = daeSafeCast< domAny >(teq->getChild("CurrentHPR"));
    if (any)
    {
        dof->setCurrentHPR(parseVec3String(any->getValue()));
    }
    else
    {
        osg::notify(osg::WARN) << "Expected element 'CurrentHPR' not found" << std::endl;
    }

    any = daeSafeCast< domAny >(teq->getChild("MinTranslate"));
    if (any)
    {
        dof->setMinTranslate(parseVec3String(any->getValue()));
    }
    else
    {
        osg::notify(osg::WARN) << "Expected element 'MinTranslate' not found" << std::endl;
    }

    any = daeSafeCast< domAny >(teq->getChild("MaxTranslate"));
    if (any)
    {
        dof->setMaxTranslate(parseVec3String(any->getValue()));
    }
    else
    {
        osg::notify(osg::WARN) << "Expected element 'MaxTranslate' not found" << std::endl;
    }

    any = daeSafeCast< domAny >(teq->getChild("IncrementTranslate"));
    if (any)
    {
        dof->setIncrementTranslate(parseVec3String(any->getValue()));
    }
    else
    {
        osg::notify(osg::WARN) << "Expected element 'IncrementTranslate' not found" << std::endl;
    }

    any = daeSafeCast< domAny >(teq->getChild("CurrentTranslate"));
    if (any)
    {
        dof->setCurrentTranslate(parseVec3String(any->getValue()));
    }
    else
    {
        osg::notify(osg::WARN) << "Expected element 'CurrentTranslate' not found" << std::endl;
    }

    any = daeSafeCast< domAny >(teq->getChild("MinScale"));
    if (any)
    {
        dof->setMinScale(parseVec3String(any->getValue()));
    }
    else
    {
        osg::notify(osg::WARN) << "Expected element 'MinScale' not found" << std::endl;
    }

    any = daeSafeCast< domAny >(teq->getChild("MaxScale"));
    if (any)
    {
        dof->setMaxScale(parseVec3String(any->getValue()));
    }
    else
    {
        osg::notify(osg::WARN) << "Expected element 'MaxScale' not found" << std::endl;
    }

    any = daeSafeCast< domAny >(teq->getChild("IncrementScale"));
    if (any)
    {
        dof->setIncrementScale(parseVec3String(any->getValue()));
    }
    else
    {
        osg::notify(osg::WARN) << "Expected element 'IncrementScale' not found" << std::endl;
    }

    any = daeSafeCast< domAny >(teq->getChild("CurrentScale"));
    if (any)
    {
        dof->setCurrentScale(parseVec3String(any->getValue()));
    }
    else
    {
        osg::notify(osg::WARN) << "Expected element 'CurrentScale' not found" << std::endl;
    }

    any = daeSafeCast< domAny >(teq->getChild("MultOrder"));
    if (any)
    {
        dof->setHPRMultOrder((osgSim::DOFTransform::MultOrder)parseString<int>(any->getValue()));
    }
    else
    {
        osg::notify(osg::WARN) << "Expected element 'MultOrder' not found" << std::endl;
    }

    any = daeSafeCast< domAny >(teq->getChild("LimitationFlags"));
    if (any)
    {
        dof->setLimitationFlags(parseString<unsigned long>(any->getValue()));
    }
    else
    {
        osg::notify(osg::WARN) << "Expected element 'LimitationFlags' not found" << std::endl;
    }

    any = daeSafeCast< domAny >(teq->getChild("AnimationOn"));
    if (any)
    {
        dof->setAnimationOn(parseString<bool>(any->getValue()));
    }
    else
    {
        osg::notify(osg::WARN) << "Expected element 'AnimationOn' not found" << std::endl;
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
        osg::notify(osg::WARN) << "Expected element 'PutMatrix' not found" << std::endl;
    }

    return dof;
}
