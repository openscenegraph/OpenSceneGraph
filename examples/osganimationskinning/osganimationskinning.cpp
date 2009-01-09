/*  -*-c++-*- 
 *  Copyright (C) 2008 Cedric Pinson <mornifle@plopbyte.net>
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

#include <iostream>
#include <osg/Geometry>
#include <osg/MatrixTransform>
#include <osg/Geode>
#include <osgViewer/Viewer>
#include <osgGA/TrackballManipulator>
#include <osgUtil/SmoothingVisitor>
#include <osg/io_utils>

#include <osgAnimation/Bone>
#include <osgAnimation/Skeleton>
#include <osgAnimation/RigGeometry>
#include <osgAnimation/Skinning>
#include <osgAnimation/BasicAnimationManager>

osg::Geode* createAxis()
{
    osg::Geode* geode (new osg::Geode());  
    osg::Geometry* geometry (new osg::Geometry());

    osg::Vec3Array* vertices (new osg::Vec3Array());
    vertices->push_back (osg::Vec3 ( 0.0, 0.0, 0.0));
    vertices->push_back (osg::Vec3 ( 1.0, 0.0, 0.0));
    vertices->push_back (osg::Vec3 ( 0.0, 0.0, 0.0));
    vertices->push_back (osg::Vec3 ( 0.0, 1.0, 0.0));
    vertices->push_back (osg::Vec3 ( 0.0, 0.0, 0.0));
    vertices->push_back (osg::Vec3 ( 0.0, 0.0, 1.0));
    geometry->setVertexArray (vertices);

    osg::Vec4Array* colors (new osg::Vec4Array());
    colors->push_back (osg::Vec4 (1.0f, 0.0f, 0.0f, 1.0f));
    colors->push_back (osg::Vec4 (1.0f, 0.0f, 0.0f, 1.0f));
    colors->push_back (osg::Vec4 (0.0f, 1.0f, 0.0f, 1.0f));
    colors->push_back (osg::Vec4 (0.0f, 1.0f, 0.0f, 1.0f));
    colors->push_back (osg::Vec4 (0.0f, 0.0f, 1.0f, 1.0f));
    colors->push_back (osg::Vec4 (0.0f, 0.0f, 1.0f, 1.0f));
    geometry->setColorArray (colors);

    geometry->setColorBinding (osg::Geometry::BIND_PER_VERTEX);    
    geometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES,0,6));

    geode->addDrawable( geometry );
    return geode;
}

osgAnimation::RigGeometry* createTesselatedBox(int nsplit, float size)
{
    osgAnimation::RigGeometry* geometry = new osgAnimation::RigGeometry;

    osg::ref_ptr<osg::Vec3Array> vertices (new osg::Vec3Array());
    osg::ref_ptr<osg::Vec3Array> colors (new osg::Vec3Array());
    geometry->setVertexArray (vertices.get());
    geometry->setColorArray (colors.get());
    geometry->setColorBinding (osg::Geometry::BIND_PER_VERTEX);    
  
    float step = size / nsplit;
    float s = 0.5/4.0;
    for (int i = 0; i < nsplit; i++) 
    {
        float x = -1 + i * step;
        std::cout << x << std::endl;
        vertices->push_back (osg::Vec3 ( x, s, s));
        vertices->push_back (osg::Vec3 ( x, -s, s));
        vertices->push_back (osg::Vec3 ( x, -s, -s));
        vertices->push_back (osg::Vec3 ( x, s, -s));
        osg::Vec3 c (0,0,0);
        c[i%3] = 1;
        colors->push_back (c);
        colors->push_back (c);
        colors->push_back (c);
        colors->push_back (c);
    }

    osg::ref_ptr<osg::UIntArray> array = new osg::UIntArray;
    for (int i = 0; i < nsplit - 1; i++) 
    {
        int base = i * 4;
        array->push_back(base);
        array->push_back(base+1);
        array->push_back(base+4);
        array->push_back(base+1);
        array->push_back(base+5);
        array->push_back(base+4);

        array->push_back(base+3);
        array->push_back(base);
        array->push_back(base+4);
        array->push_back(base+7);
        array->push_back(base+3);
        array->push_back(base+4);

        array->push_back(base+5);
        array->push_back(base+1);
        array->push_back(base+2);
        array->push_back(base+2);
        array->push_back(base+6);
        array->push_back(base+5);

        array->push_back(base+2);
        array->push_back(base+3);
        array->push_back(base+7);
        array->push_back(base+6);
        array->push_back(base+2);
        array->push_back(base+7);
    }
  
    geometry->addPrimitiveSet(new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, array->size(), &array->front()));
    geometry->setUseDisplayList( false );
    return geometry;
}


void initVertexMap(osgAnimation::Bone* b0,
                   osgAnimation::Bone* b1,
                   osgAnimation::Bone* b2,
                   osgAnimation::RigGeometry* geom,
                   osg::Vec3Array* array)
{
    osgAnimation::VertexInfluenceSet vertexesInfluences;
    osgAnimation::VertexInfluenceMap* vim = new osgAnimation::VertexInfluenceMap;

    (*vim)[b0->getName()].setName(b0->getName());
    (*vim)[b1->getName()].setName(b1->getName());
    (*vim)[b2->getName()].setName(b2->getName());

    for (int i = 0; i < (int)array->size(); i++) 
    {
        float val = (*array)[i][0];
        std::cout << val << std::endl;
        if (val >= -1 && val <= 0)
            (*vim)[b0->getName()].push_back(osgAnimation::VertexIndexWeight(i,1));
        else if ( val > 0 && val <= 1)
            (*vim)[b1->getName()].push_back(osgAnimation::VertexIndexWeight(i,1));
        else if ( val > 1)
            (*vim)[b2->getName()].push_back(osgAnimation::VertexIndexWeight(i,1));
    }

    geom->setInfluenceMap(vim);
}



int main (int argc, char* argv[])
{
    osg::ArgumentParser arguments(&argc, argv);
    osgViewer::Viewer viewer(arguments);

    viewer.setCameraManipulator(new osgGA::TrackballManipulator());

    osg::ref_ptr<osgAnimation::Skeleton> skelroot = new osgAnimation::Skeleton;
    skelroot->setDefaultUpdateCallback();
    osg::ref_ptr<osgAnimation::Bone> root = new osgAnimation::Bone;
    {
        root->setBindMatrixInBoneSpace(osg::Matrix::identity());
        root->setBindMatrixInBoneSpace(osg::Matrix::translate(-1,0,0));
        root->setName("root");
        root->setDefaultUpdateCallback();
    }

    osg::ref_ptr<osgAnimation::Bone> right0 = new osgAnimation::Bone;
    right0->setBindMatrixInBoneSpace(osg::Matrix::translate(1,0,0));
    right0->setName("right0");
    right0->setDefaultUpdateCallback("right0");

    osg::ref_ptr<osgAnimation::Bone> right1 = new osgAnimation::Bone;
    right1->setBindMatrixInBoneSpace(osg::Matrix::translate(1,0,0));
    right1->setName("right1");
    right1->setDefaultUpdateCallback("right1");

    root->addChild(right0.get());
    right0->addChild(right1.get());
    skelroot->addChild(root.get());

    osg::Group* scene = new osg::Group;
    osg::ref_ptr<osgAnimation::BasicAnimationManager> manager = new osgAnimation::BasicAnimationManager;
    scene->setUpdateCallback(manager.get());

    osgAnimation::Animation* anim = new osgAnimation::Animation;
    {
        osgAnimation::QuatKeyframeContainer* keys0 = new osgAnimation::QuatKeyframeContainer;
        osg::Quat rotate;
        rotate.makeRotate(osg::PI_2, osg::Vec3(0,0,1));
        keys0->push_back(osgAnimation::QuatKeyframe(0,osg::Quat(0,0,0,1)));
        keys0->push_back(osgAnimation::QuatKeyframe(3,rotate));
        keys0->push_back(osgAnimation::QuatKeyframe(6,rotate));
        osgAnimation::QuatSphericalLinearSampler* sampler = new osgAnimation::QuatSphericalLinearSampler;
        sampler->setKeyframeContainer(keys0);
        // osgAnimation::AnimationUpdateCallback* cb = dynamic_cast<osgAnimation::AnimationUpdateCallback*>(right0->getUpdateCallback());
        osgAnimation::QuatSphericalLinearChannel* channel = new osgAnimation::QuatSphericalLinearChannel(sampler);
        channel->setName("quaternion");
        channel->setTargetName("right0");
        anim->addChannel(channel);
    }

    {
        osgAnimation::QuatKeyframeContainer* keys1 = new osgAnimation::QuatKeyframeContainer;
        osg::Quat rotate;
        rotate.makeRotate(osg::PI_2, osg::Vec3(0,0,1));
        keys1->push_back(osgAnimation::QuatKeyframe(0,osg::Quat(0,0,0,1)));
        keys1->push_back(osgAnimation::QuatKeyframe(3,osg::Quat(0,0,0,1)));
        keys1->push_back(osgAnimation::QuatKeyframe(6,rotate));
        osgAnimation::QuatSphericalLinearSampler* sampler = new osgAnimation::QuatSphericalLinearSampler;
        sampler->setKeyframeContainer(keys1);
        osgAnimation::QuatSphericalLinearChannel* channel = new osgAnimation::QuatSphericalLinearChannel(sampler);
        //osgAnimation::AnimationUpdateCallback* cb = dynamic_cast<osgAnimation::AnimationUpdateCallback*>(right1->getUpdateCallback());
        channel->setName("quaternion");
        channel->setTargetName("right1");
        anim->addChannel(channel);
    }
    manager->registerAnimation(anim);
    manager->buildTargetReference();
  
    // let's start !
    manager->playAnimation(anim);

    // we will use local data from the skeleton
    osg::MatrixTransform* rootTransform = new osg::MatrixTransform;
    rootTransform->setMatrix(osg::Matrix::rotate(osg::PI_2,osg::Vec3(1,0,0)));
    right0->addChild(createAxis());
    right0->setDataVariance(osg::Object::DYNAMIC);
    right1->addChild(createAxis());
    right1->setDataVariance(osg::Object::DYNAMIC);
    osg::MatrixTransform* trueroot = new osg::MatrixTransform;
    trueroot->setMatrix(osg::Matrix(root->getMatrixInBoneSpace().ptr()));
    trueroot->addChild(createAxis());
    trueroot->addChild(skelroot);
    trueroot->setDataVariance(osg::Object::DYNAMIC);
    rootTransform->addChild(trueroot);
    scene->addChild(rootTransform);
  
    osgAnimation::RigGeometry* geom = createTesselatedBox(4, 4.0);
    osg::Geode* geode = new osg::Geode;
    geode->addDrawable(geom);
    skelroot->addChild(geode);
    osg::ref_ptr<osg::Vec3Array> src = dynamic_cast<osg::Vec3Array*>(geom->getVertexArray());
    geom->getOrCreateStateSet()->setMode(GL_LIGHTING, false);
    geom->setDataVariance(osg::Object::DYNAMIC);

    initVertexMap(root.get(), right0.get(), right1.get(), geom, src.get());

    geom->buildVertexSet();
    geom->buildTransformer(skelroot.get());

    // let's run !
    viewer.setSceneData( scene );
    viewer.realize();

    while (!viewer.done())
    {
        viewer.frame();
    }

    return 0;
}


