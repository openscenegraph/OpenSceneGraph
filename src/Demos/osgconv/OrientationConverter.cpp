#include <stdio.h>

#include <osg/GeoSet>
#include <osg/Geode>
#include <osg/Billboard>
#include <osg/LOD>

#include "OrientationConverter.h"

using namespace osg;

OrientationConverter::OrientationConverter( void )
{
   R.makeIdent();
   T.makeIdent();
   _trans_set = false;
   S.makeIdent();
}

void OrientationConverter::setRotation( const Vec3 &from, const Vec3 &to )
{
    R = Matrix::rotate( from, to );
}

void OrientationConverter::setTranslation( const Vec3 &trans )
{
    T = Matrix::translate(trans);
    _trans_set = true;
}

void OrientationConverter::setScale( const Vec3 &scale )
{
    S = Matrix::scale(scale);
}


void OrientationConverter::convert( Node &node )
{
    // Order of operations here is :
    // 1. Translate to world origin (0,0,0)
    // 2. Rotate to new orientation
    // 3. Scale in new orientation coordinates
    // 4. If an absolute translation was specified then
    //        - translate to absolute translation in world coordinates
    //    else
    //        - translate back to model's original origin. 
    BoundingSphere bs = node.getBound();
    Matrix C = Matrix::translate( Vec3(0,0,0) - bs.center() );
    if( _trans_set == false )
        T = Matrix::translate( bs.center() );
    _cv.setMatrix( C * R * S * T );

    node.accept(_cv);
}


void OrientationConverter::ConvertVisitor::apply(osg::Geode& geode)
{
    for(int i=0;i<geode.getNumDrawables();++i)
    {
        geode.getDrawable(i)->applyAttributeOperation(_tf);
    }
}

void OrientationConverter::ConvertVisitor::apply(osg::Billboard& billboard)
{
    osg::Vec3 axis = osg::Matrix::transform3x3(_tf._im,billboard.getAxis());
    billboard.setAxis(axis);

    for(int i=0;i<billboard.getNumDrawables();++i)
    {
        billboard.setPos(i,billboard.getPos(i)*_tf._m);
        billboard.getDrawable(i)->applyAttributeOperation(_tf);
    }
}

void OrientationConverter::ConvertVisitor::apply(osg::LOD& lod)
{
    lod.setCenter(lod.getCenter()*_tf._m);
    traverse(lod);
}
