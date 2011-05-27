#include <stdio.h>

#include <osg/MatrixTransform>
#include <osgUtil/Optimizer>

#include "OrientationConverter.h"

using namespace osg;

OrientationConverter::OrientationConverter( void )
{
   R.makeIdentity();
   T.makeIdentity();
   _trans_set = false;
   _use_world_frame = false;
   S.makeIdentity();
}

void OrientationConverter::setRotation( const Vec3 &from, const Vec3 &to )
{
    R = Matrix::rotate( from, to );
}

void OrientationConverter::setRotation( float degrees, const Vec3 &axis )
{
    R = Matrix::rotate( osg::DegreesToRadians(degrees), axis );
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

void OrientationConverter::useWorldFrame( bool worldFrame )
{
   _use_world_frame = worldFrame;
}

Node* OrientationConverter::convert( Node *node )
{
    // Order of operations here is :
    // 1. If world frame option not set, translate to world origin (0,0,0)
    // 2. Rotate to new orientation
    // 3. Scale in new orientation coordinates
    // 4. If an absolute translation was specified then
    //        - translate to absolute translation in world coordinates
    //    else if world frame option not set,
    //        - translate back to model's original origin.
    BoundingSphere bs = node->getBound();
    Matrix C;

    if (_use_world_frame)
    {
        C.makeIdentity();
    }
    else
    {
        C = Matrix::translate( -bs.center() );

        if( !_trans_set )
            T = Matrix::translate( bs.center() );
    }

    osg::Group* root = new osg::Group;
    osg::MatrixTransform* transform = new osg::MatrixTransform;

    transform->setDataVariance(osg::Object::STATIC);
    transform->setMatrix( C * R * S * T );
    
    if (!S.isIdentity())
    {
        // Add a normalize state. This will be removed if the FlattenStaticTransformsVisitor works
        transform->getOrCreateStateSet()->setMode(GL_NORMALIZE, osg::StateAttribute::ON);
    }

    root->addChild(transform);
    transform->addChild(node);

    osgUtil::Optimizer::FlattenStaticTransformsVisitor fstv;
    root->accept(fstv);
    fstv.removeTransforms(root);
    
    return root->getChild(0);
}
