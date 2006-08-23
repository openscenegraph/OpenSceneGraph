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
#include <dom/domCOLLADA.h>

#include <osg/PositionAttitudeTransform>
#include <osg/MatrixTransform>

using namespace osgdae;

osg::Transform* daeReader::processMatrix( domMatrix *mat )
{
    osg::Transform* xform = new osg::MatrixTransform();
    xform->setDataVariance(osg::Object::STATIC);

    xform->setName( mat->getSid() ? mat->getSid() : "" );

    osg::Matrix m;

    if (mat->getValue().getCount() != 16 ) {
        osg::notify(osg::WARN)<<"Data is wrong size for matrix"<<std::endl;
        return NULL;
    }
    
    //m.set((daeDouble*)mat->getValue().getRawData());
    m.set(  mat->getValue()[0], mat->getValue()[4], mat->getValue()[8], mat->getValue()[12],
            mat->getValue()[1], mat->getValue()[5], mat->getValue()[9], mat->getValue()[13],
            mat->getValue()[2], mat->getValue()[6], mat->getValue()[10], mat->getValue()[14],
            mat->getValue()[3], mat->getValue()[7], mat->getValue()[11], mat->getValue()[15] );

    xform->asMatrixTransform()->setMatrix(m);

    return xform;
}

osg::Transform* daeReader::processTranslate( domTranslate *trans )
{
    osg::Transform* xform = new osg::PositionAttitudeTransform();
    //xform->setDataVariance(osg::Object::STATIC);

    xform->setName( trans->getSid() ? trans->getSid() : "" );

    if (trans->getValue().getCount() != 3 ) {
        osg::notify(osg::WARN)<<"Data is wrong size for translate"<<std::endl;
        return NULL;
    }

    domFloat3& t = trans->getValue();

    xform->asPositionAttitudeTransform()->setPosition(
        osg::Vec3(t[0],t[1],t[2]));

    return xform;
}

osg::Transform* daeReader::processRotate( domRotate *rot )
{
    osg::Transform* xform = new osg::PositionAttitudeTransform();
    //xform->setDataVariance(osg::Object::STATIC);

    xform->setName( rot->getSid() ? rot->getSid() : "" );

    if (rot->getValue().getCount() != 4 ) {
        osg::notify(osg::WARN)<<"Data is wrong size for rotate"<<std::endl;
        return NULL;
    }
    domFloat4& r = rot->getValue();

    osg::Vec3 axis;
    axis.set(r[0],r[1],r[2]);
    xform->asPositionAttitudeTransform()->setAttitude(
        osg::Quat(osg::DegreesToRadians(r[3]),axis));

    return xform;
}

osg::Transform* daeReader::processScale( domScale *scale )
{
    osg::Transform* xform = new osg::PositionAttitudeTransform();
    //xform->setDataVariance(osg::Object::STATIC);

    xform->setName( scale->getSid() ? scale->getSid() : "" );

    if (scale->getValue().getCount() != 3 ) {
        osg::notify(osg::WARN)<<"Data is wrong size for scale"<<std::endl;
        return NULL;
    }
    domFloat3& s = scale->getValue();

    xform->asPositionAttitudeTransform()->setScale(
        osg::Vec3(s[0],s[1],s[2]));

    return xform;
}

osg::Transform* daeReader::processLookat( domLookat *la )
{
    osg::Transform* xform = new osg::MatrixTransform();
    xform->setDataVariance(osg::Object::STATIC);

    xform->setName( la->getSid() ? la->getSid() : "" );

    if (la->getValue().getCount() != 9 ) {
        osg::notify(osg::WARN)<<"Data is wrong size for lookat"<<std::endl;
        return NULL;
    }

    osg::Matrix m;

    osg::Vec3 eye;
    osg::Vec3 center;
    osg::Vec3 up;

    eye.set( la->getValue()[0], la->getValue()[1], la->getValue()[2] );
    center.set( la->getValue()[3], la->getValue()[4], la->getValue()[5] );
    up.set( la->getValue()[6], la->getValue()[7], la->getValue()[8] );

    m.makeLookAt( eye, center, up );

    xform->asMatrixTransform()->setMatrix(m);

    return xform;
}

osg::Transform* daeReader::processSkew( domSkew *skew )
{
    osg::Transform* xform = new osg::MatrixTransform();
    xform->setDataVariance(osg::Object::STATIC);

    xform->setName( skew->getSid() ? skew->getSid() : "" );

    if (skew->getValue().getCount() != 9 ) {
        osg::notify(osg::WARN)<<"Data is wrong size for skew"<<std::endl;
        return NULL;
    }
    domFloat7& s = skew->getValue();

    float angle = s[0];
    float shear = sin(osg::DegreesToRadians(angle));
    osg::Vec3 around(s[1],s[2],s[3]);
    osg::Vec3 along(s[4],s[5],s[6]);

    osg::Vec3 const x(1,0,0);
    osg::Vec3 const y(0,1,0);
    osg::Vec3 const z(0,0,1);

    osg::Matrix m;

    if ( along == x ) {
        if ( around == y ) {
            m(2,0) = shear;
        } else if ( around == z ) {
            m(1,0) = -shear;
        } else {
            //osg::notify(osg::WARN)<<"Unsupported skew around "<<around<<std::endl;
        }
    } else if ( along == y ) {
            if ( around == x ) {
                m(2,1) = -shear;
            } else if ( around == z ) {
                m(0,1) = shear;
            } else {
                //osg::notify(osg::WARN)<<"Unsupported skew around "<<around<<std::endl;
            }
    } else if ( along == z ) {
        if ( around == x ) {
            m(1,2) = shear;
        } else if ( around == y ) {
            m(0,2) = -shear;
        } else {
            //osg::notify(osg::WARN)<<"Unsupported skew around "<<around<<std::endl;
        }
    } else {
        //osg::notify(osg::WARN)<<"Unsupported skew along "<<along<<std::endl;
    }


    if (angle > 0) {
        //osg::notify(osg::NOTICE)<<"Skew: angle("<<angle<<") around("<<around<<") along("<<along<<")"<<std::endl;
    }

    xform->asMatrixTransform()->setMatrix(m);

    return xform;
}
