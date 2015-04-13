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
#include <osg/Shape>
#include <osg/ShapeDrawable>
#include <osgViewer/Viewer>
#include <osgGA/TrackballManipulator>
#include <osg/MatrixTransform>

#include <osgAnimation/BasicAnimationManager>
#include <osgAnimation/Channel>
#include <osgAnimation/UpdateMatrixTransform>
#include <osgAnimation/StackedTranslateElement>
#include <osgAnimation/StackedRotateAxisElement>

using namespace osgAnimation;

osg::ref_ptr<osg::Geode> createAxis()
{
    osg::ref_ptr<osg::Geode> geode (new osg::Geode());
    osg::ref_ptr<osg::Geometry> geometry (new osg::Geometry());

    osg::ref_ptr<osg::Vec3Array> vertices (new osg::Vec3Array());
    vertices->push_back (osg::Vec3 ( 0.0, 0.0, 0.0));
    vertices->push_back (osg::Vec3 ( 10.0, 0.0, 0.0));
    vertices->push_back (osg::Vec3 ( 0.0, 0.0, 0.0));
    vertices->push_back (osg::Vec3 ( 0.0, 10.0, 0.0));
    vertices->push_back (osg::Vec3 ( 0.0, 0.0, 0.0));
    vertices->push_back (osg::Vec3 ( 0.0, 0.0, 10.0));
    geometry->setVertexArray (vertices.get());

    osg::ref_ptr<osg::Vec4Array> colors (new osg::Vec4Array());
    colors->push_back (osg::Vec4 (1.0f, 0.0f, 0.0f, 1.0f));
    colors->push_back (osg::Vec4 (1.0f, 0.0f, 0.0f, 1.0f));
    colors->push_back (osg::Vec4 (0.0f, 1.0f, 0.0f, 1.0f));
    colors->push_back (osg::Vec4 (0.0f, 1.0f, 0.0f, 1.0f));
    colors->push_back (osg::Vec4 (0.0f, 0.0f, 1.0f, 1.0f));
    colors->push_back (osg::Vec4 (0.0f, 0.0f, 1.0f, 1.0f));
    geometry->setColorArray (colors.get(), osg::Array::BIND_PER_VERTEX);
    geometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES,0,6));

    geode->addDrawable( geometry.get() );
    geode->getOrCreateStateSet()->setMode(GL_LIGHTING, false);
    return geode;
}


int main (int argc, char* argv[])
{
    osg::ArgumentParser arguments(&argc, argv);
    osgViewer::Viewer viewer(arguments);

    viewer.setCameraManipulator(new osgGA::TrackballManipulator());

    osg::Group* root = new osg::Group;

    osg::ref_ptr<osg::Geode> axe = createAxis();
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    geode->addDrawable(new osg::ShapeDrawable(new osg::Box(osg::Vec3(0.0f,0.0f,0.0f),0.5)));

    //Transformation to be manipulated by the animation
    osg::ref_ptr<osg::MatrixTransform> trans = new osg::MatrixTransform();
    trans->setName("AnimatedNode");
    //Dynamic object, has to be updated during update traversal
    trans->setDataVariance(osg::Object::DYNAMIC);
    //Animation callback for Matrix transforms, name is targetName for Channels
    osgAnimation::UpdateMatrixTransform* updatecb = new osgAnimation::UpdateMatrixTransform("AnimatedCallback");
    //add manipulator Stack, names must match with channel names
    //elements are applied in LIFO order
    //The first element modifies the position component of the matrix
    //The second element modifies the rotation around x-axis
    updatecb->getStackedTransforms().push_back(new osgAnimation::StackedTranslateElement("position"));
    updatecb->getStackedTransforms().push_back(new osgAnimation::StackedRotateAxisElement("euler",osg::Vec3(1,0,0),0));
    //connect the UpdateMatrixTransform callback to the MatrixTransform
    trans->setUpdateCallback(updatecb);
    //initialize MatrixTranform
    trans->setMatrix(osg::Matrix::identity());
    //append geometry node
    trans->addChild (geode.get());

    root->addChild (axe.get());
    root->addChild (trans.get());

    // Define a scheduler for our animations
    osg::Group* grp = new osg::Group;
    //add the animation manager to the scene graph to get it called during update traversals
    osgAnimation::BasicAnimationManager* mng = new osgAnimation::BasicAnimationManager();
    grp->setUpdateCallback(mng);
    //add the rest of the scene to the grp node
    grp->addChild(root);

    // And we finally define our channel for linear Vector interpolation
    osgAnimation::Vec3LinearChannel* channelAnimation1 = new osgAnimation::Vec3LinearChannel;
    //name of the AnimationUpdateCallback
    channelAnimation1->setTargetName("AnimatedCallback");
    //name of the StackedElementTransform for position modification
    channelAnimation1->setName("position");
    //Create keyframes for (in this case linear) interpolation of a osg::Vec3
    channelAnimation1->getOrCreateSampler()->getOrCreateKeyframeContainer()->push_back(osgAnimation::Vec3Keyframe(0, osg::Vec3(0,0,0)));
    channelAnimation1->getOrCreateSampler()->getOrCreateKeyframeContainer()->push_back(osgAnimation::Vec3Keyframe(2, osg::Vec3(1,1,0)));
    osgAnimation::Animation* anim1 = new osgAnimation::Animation;
    anim1->addChannel(channelAnimation1);
    anim1->setPlayMode(osgAnimation::Animation::PPONG);


    //define the channel for interpolation of a float angle value
    osgAnimation::FloatLinearChannel* channelAnimation2 = new osgAnimation::FloatLinearChannel;
    //name of the AnimationUpdateCallback
    channelAnimation2->setTargetName("AnimatedCallback");
    //name of the StackedElementTransform for position modification
    channelAnimation2->setName("euler");
    //Create keyframes for (in this case linear) interpolation of a osg::Vec3
    channelAnimation2->getOrCreateSampler()->getOrCreateKeyframeContainer()->push_back(osgAnimation::FloatKeyframe(0, 0));
    channelAnimation2->getOrCreateSampler()->getOrCreateKeyframeContainer()->push_back(osgAnimation::FloatKeyframe(1.5, 2*osg::PI));
    osgAnimation::Animation* anim2 = new osgAnimation::Animation;
    anim2->addChannel(channelAnimation2);
    anim2->setPlayMode(osgAnimation::Animation::LOOP);


    // We register all animation inside the scheduler
    mng->registerAnimation(anim1);
    mng->registerAnimation(anim2);

    //start the animation
    mng->playAnimation(anim1);
    mng->playAnimation(anim2);

    //set the grp-Group with the scene and the AnimationManager as viewer's scene data
    viewer.setSceneData( grp );
    return viewer.run();
}
