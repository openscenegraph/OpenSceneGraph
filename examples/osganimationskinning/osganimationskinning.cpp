/*  -*-c++-*-
 *  Copyright (C) 2008 Cedric Pinson <cedric.pinson@plopbyte.net>
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
#include <osgDB/WriteFile>
#include <osgUtil/SmoothingVisitor>
#include <osg/io_utils>

#include <osgAnimation/Bone>
#include <osgAnimation/Skeleton>
#include <osgAnimation/RigGeometry>
#include <osgAnimation/BasicAnimationManager>
#include <osgAnimation/UpdateMatrixTransform>
#include <osgAnimation/UpdateBone>
#include <osgAnimation/StackedTransform>
#include <osgAnimation/StackedTranslateElement>
#include <osgAnimation/StackedRotateAxisElement>

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
    geometry->setColorArray (colors, osg::Array::BIND_PER_VERTEX);
    geometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES,0,6));

    geode->addDrawable( geometry );
    return geode;
}

osgAnimation::RigGeometry* createTesselatedBox(int nsplit, float size)
{
    osgAnimation::RigGeometry* riggeometry = new osgAnimation::RigGeometry;

    osg::Geometry* geometry = new osg::Geometry;
    osg::ref_ptr<osg::Vec3Array> vertices (new osg::Vec3Array());
    osg::ref_ptr<osg::Vec3Array> colors (new osg::Vec3Array());
    geometry->setVertexArray (vertices.get());
    geometry->setColorArray (colors.get(), osg::Array::BIND_PER_VERTEX);

    float step = size / static_cast<float>(nsplit);
    float s = 0.5f/4.0f;
    for (int i = 0; i < nsplit; i++)
    {
        float x = -1.0f + static_cast<float>(i) * step;
        std::cout << x << std::endl;
        vertices->push_back (osg::Vec3 ( x, s, s));
        vertices->push_back (osg::Vec3 ( x, -s, s));
        vertices->push_back (osg::Vec3 ( x, -s, -s));
        vertices->push_back (osg::Vec3 ( x, s, -s));
        osg::Vec3 c (0.0f,0.0f,0.0f);
        c[i%3] = 1.0f;
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
    riggeometry->setSourceGeometry(geometry);
    return riggeometry;
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
        if (val >= -1.0f && val <= 0.0f)
            (*vim)[b0->getName()].push_back(osgAnimation::VertexIndexWeight(i,1.0f));
        else if ( val > 0.0f && val <= 1.0f)
            (*vim)[b1->getName()].push_back(osgAnimation::VertexIndexWeight(i,1.0f));
        else if ( val > 1.0f)
            (*vim)[b2->getName()].push_back(osgAnimation::VertexIndexWeight(i,1.0f));
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
    root->setInvBindMatrixInSkeletonSpace(osg::Matrix::inverse(osg::Matrix::translate(-1.0,0.0,0.0)));
    root->setName("root");
    osgAnimation::UpdateBone* pRootUpdate = new osgAnimation::UpdateBone("root");
    pRootUpdate->getStackedTransforms().push_back(new osgAnimation::StackedTranslateElement("translate",osg::Vec3(-1.0f,0.0f,0.0f)));
    root->setUpdateCallback(pRootUpdate);

    osg::ref_ptr<osgAnimation::Bone> right0 = new osgAnimation::Bone;
    right0->setInvBindMatrixInSkeletonSpace(osg::Matrix::inverse(osg::Matrix::translate(0.0,0.0,0.0)));
    right0->setName("right0");
    osgAnimation::UpdateBone* pRight0Update = new osgAnimation::UpdateBone("right0");
    pRight0Update->getStackedTransforms().push_back(new osgAnimation::StackedTranslateElement("translate", osg::Vec3(1.0f,0.0f,0.0f)));
    pRight0Update->getStackedTransforms().push_back(new osgAnimation::StackedRotateAxisElement("rotate", osg::Vec3(0.0f,0.0f,1.0f), 0.0));
    right0->setUpdateCallback(pRight0Update);

    osg::ref_ptr<osgAnimation::Bone> right1 = new osgAnimation::Bone;
    right1->setInvBindMatrixInSkeletonSpace(osg::Matrix::inverse(osg::Matrix::translate(1.0,0.0,0.0)));
    right1->setName("right1");
    osgAnimation::UpdateBone* pRight1Update = new osgAnimation::UpdateBone("right1");
    pRight1Update->getStackedTransforms().push_back(new osgAnimation::StackedTranslateElement("translate", osg::Vec3(1.0f,0.0f,0.0f)));
    pRight1Update->getStackedTransforms().push_back(new osgAnimation::StackedRotateAxisElement("rotate", osg::Vec3(0.0f,0.0f,1.0f), 0.0));
    right1->setUpdateCallback(pRight1Update);

    root->addChild(right0.get());
    right0->addChild(right1.get());
    skelroot->addChild(root.get());

    osg::Group* scene = new osg::Group;
    osg::ref_ptr<osgAnimation::BasicAnimationManager> manager = new osgAnimation::BasicAnimationManager;
    scene->setUpdateCallback(manager.get());

    osgAnimation::Animation* anim = new osgAnimation::Animation;
    {
        osgAnimation::FloatKeyframeContainer* keys0 = new osgAnimation::FloatKeyframeContainer;
        keys0->push_back(osgAnimation::FloatKeyframe(0.0,0.0f));
        keys0->push_back(osgAnimation::FloatKeyframe(3.0,osg::PI_2));
        keys0->push_back(osgAnimation::FloatKeyframe(6.0,osg::PI_2));
        osgAnimation::FloatLinearSampler* sampler = new osgAnimation::FloatLinearSampler;
        sampler->setKeyframeContainer(keys0);
        osgAnimation::FloatLinearChannel* channel = new osgAnimation::FloatLinearChannel(sampler);
        channel->setName("rotate");
        channel->setTargetName("right0");
        anim->addChannel(channel);
    }

    {
        osgAnimation::FloatKeyframeContainer* keys1 = new osgAnimation::FloatKeyframeContainer;
        keys1->push_back(osgAnimation::FloatKeyframe(0.0,0.0f));
        keys1->push_back(osgAnimation::FloatKeyframe(3.0,0.0f));
        keys1->push_back(osgAnimation::FloatKeyframe(6.0,osg::PI_2));
        osgAnimation::FloatLinearSampler* sampler = new osgAnimation::FloatLinearSampler;
        sampler->setKeyframeContainer(keys1);
        osgAnimation::FloatLinearChannel* channel = new osgAnimation::FloatLinearChannel(sampler);
        channel->setName("rotate");
        channel->setTargetName("right1");
        anim->addChannel(channel);
    }
    manager->registerAnimation(anim);
    manager->buildTargetReference();

    // let's start !
    manager->playAnimation(anim);

    // we will use local data from the skeleton
    osg::MatrixTransform* rootTransform = new osg::MatrixTransform;
    rootTransform->setMatrix(osg::Matrix::rotate(osg::PI_2,osg::Vec3(1.0f,0.0f,0.0f)));
    right0->addChild(createAxis());
    right0->setDataVariance(osg::Object::DYNAMIC);
    right1->addChild(createAxis());
    right1->setDataVariance(osg::Object::DYNAMIC);
    osg::MatrixTransform* trueroot = new osg::MatrixTransform;
    trueroot->setMatrix(osg::Matrix(root->getMatrixInBoneSpace().ptr()));
    trueroot->addChild(createAxis());
    trueroot->addChild(skelroot.get());
    trueroot->setDataVariance(osg::Object::DYNAMIC);
    rootTransform->addChild(trueroot);
    scene->addChild(rootTransform);

    osgAnimation::RigGeometry* geom = createTesselatedBox(4, 4.0f);
    osg::Geode* geode = new osg::Geode;
    geode->addDrawable(geom);
    skelroot->addChild(geode);
    osg::ref_ptr<osg::Vec3Array> src = dynamic_cast<osg::Vec3Array*>(geom->getSourceGeometry()->getVertexArray());
    geom->getOrCreateStateSet()->setMode(GL_LIGHTING, false);
    geom->setDataVariance(osg::Object::DYNAMIC);

    initVertexMap(root.get(), right0.get(), right1.get(), geom, src.get());

    // let's run !
    viewer.setSceneData( scene );
    viewer.realize();

    while (!viewer.done())
    {
        viewer.frame();
    }

    osgDB::writeNodeFile(*scene, "skinning.osg");
    return 0;
}


