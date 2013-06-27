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
#include <osg/Shape>
#include <osg/ShapeDrawable>
#include <osgViewer/Viewer>
#include <osgGA/TrackballManipulator>
#include <osg/MatrixTransform>
#include <osg/Material>
#include <osgAnimation/Sampler>

class AnimtkUpdateCallback : public osg::NodeCallback
{
public:
    META_Object(osgAnimation, AnimtkUpdateCallback);

    AnimtkUpdateCallback()
    {
        _sampler = new osgAnimation::Vec3CubicBezierSampler;
        _playing = false;
        _lastUpdate = 0;
    }
    AnimtkUpdateCallback(const AnimtkUpdateCallback& val, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY):
        osg::Object(val, copyop),
        osg::NodeCallback(val, copyop),
        _sampler(val._sampler),
        _startTime(val._startTime),
        _currentTime(val._currentTime),
        _playing(val._playing),
        _lastUpdate(val._lastUpdate)
    {
    }

    /** Callback method called by the NodeVisitor when visiting a node.*/
    virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
    {
        if (nv->getVisitorType() == osg::NodeVisitor::UPDATE_VISITOR &&
            nv->getFrameStamp() &&
            nv->getFrameStamp()->getFrameNumber() != _lastUpdate) {

            _lastUpdate = nv->getFrameStamp()->getFrameNumber();
            _currentTime = osg::Timer::instance()->tick();

            if (_playing && _sampler.get() && _sampler->getKeyframeContainer()) {
                osg::MatrixTransform* transform = dynamic_cast<osg::MatrixTransform*>(node);
                if (transform) {
                    osg::Vec3 result;
                    float t = osg::Timer::instance()->delta_s(_startTime, _currentTime);
                    float duration = _sampler->getEndTime() - _sampler->getStartTime();
                    t = fmod(t, duration);
                    t += _sampler->getStartTime();
                    _sampler->getValueAt(t, result);
                    transform->setMatrix(osg::Matrix::translate(result));
                }
            }
        }
        // note, callback is responsible for scenegraph traversal so
        // they must call traverse(node,nv) to ensure that the
        // scene graph subtree (and associated callbacks) are traversed.
        traverse(node,nv);
    }

    void start() { _startTime = osg::Timer::instance()->tick(); _currentTime = _startTime; _playing = true;}
    void stop() { _currentTime = _startTime; _playing = false;}

    osg::ref_ptr<osgAnimation::Vec3CubicBezierSampler> _sampler;
    osg::Timer_t _startTime;
    osg::Timer_t _currentTime;
    bool _playing;
    unsigned int _lastUpdate;
};


class AnimtkStateSetUpdateCallback : public osg::StateSet::Callback
{
public:
    META_Object(osgAnimation, AnimtkStateSetUpdateCallback);

    AnimtkStateSetUpdateCallback()
    {
        _sampler = new osgAnimation::Vec4LinearSampler;
        _playing = false;
        _lastUpdate = 0;
    }

    AnimtkStateSetUpdateCallback(const AnimtkStateSetUpdateCallback& val, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY):
        osg::Object(val, copyop),
        osg::StateSet::Callback(val, copyop),
        _sampler(val._sampler),
        _startTime(val._startTime),
        _currentTime(val._currentTime),
        _playing(val._playing),
        _lastUpdate(val._lastUpdate)
    {
    }

    /** Callback method called by the NodeVisitor when visiting a node.*/
    virtual void operator()(osg::StateSet* state, osg::NodeVisitor* nv)
    {
        if (state &&
            nv->getVisitorType() == osg::NodeVisitor::UPDATE_VISITOR &&
            nv->getFrameStamp() &&
            nv->getFrameStamp()->getFrameNumber() != _lastUpdate)
        {

            _lastUpdate = nv->getFrameStamp()->getFrameNumber();
            _currentTime = osg::Timer::instance()->tick();

            if (_playing && _sampler.get() && _sampler->getKeyframeContainer())
            {
                osg::Material* material = dynamic_cast<osg::Material*>(state->getAttribute(osg::StateAttribute::MATERIAL));
                if (material)
                {
                    osg::Vec4 result;
                    float t = osg::Timer::instance()->delta_s(_startTime, _currentTime);
                    float duration = _sampler->getEndTime() - _sampler->getStartTime();
                    t = fmod(t, duration);
                    t += _sampler->getStartTime();
                    _sampler->getValueAt(t, result);
                    material->setDiffuse(osg::Material::FRONT_AND_BACK, result);
                }
            }
        }
    }

    void start() { _startTime = osg::Timer::instance()->tick(); _currentTime = _startTime; _playing = true;}
    void stop() { _currentTime = _startTime; _playing = false;}

    osg::ref_ptr<osgAnimation::Vec4LinearSampler> _sampler;
    osg::Timer_t _startTime;
    osg::Timer_t _currentTime;
    bool _playing;
    unsigned int _lastUpdate;
};


osg::Geode* createAxis()
{
    osg::Geode* geode  = new osg::Geode;
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

osg::StateSet* setupStateSet()
{
    osg::StateSet* st = new osg::StateSet;
    st->setAttributeAndModes(new osg::Material, true);
    st->setMode(GL_BLEND, true);
    AnimtkStateSetUpdateCallback* callback = new AnimtkStateSetUpdateCallback;
    osgAnimation::Vec4KeyframeContainer* keys = callback->_sampler->getOrCreateKeyframeContainer();
    keys->push_back(osgAnimation::Vec4Keyframe(0, osg::Vec4(0,0,0,0)));
    keys->push_back(osgAnimation::Vec4Keyframe(2, osg::Vec4(0.5,0,0,0.5)));
    keys->push_back(osgAnimation::Vec4Keyframe(4, osg::Vec4(0,0.5,0,1)));
    keys->push_back(osgAnimation::Vec4Keyframe(6, osg::Vec4(0,0,0.5,1)));
    keys->push_back(osgAnimation::Vec4Keyframe(8, osg::Vec4(1,1,1,0.5)));
    keys->push_back(osgAnimation::Vec4Keyframe(10, osg::Vec4(0,0,0,0)));
    callback->start();
    st->setUpdateCallback(callback);
    return st;
}

osg::Node* setupCube()
{
    osg::Geode* geode = new osg::Geode;
    geode->addDrawable(new osg::ShapeDrawable(new osg::Box(osg::Vec3(0.0f,0.0f,0.0f),2)));
    geode->setStateSet(setupStateSet());
    return geode;
}

osg::MatrixTransform* setupAnimtkNode()
{
    osg::Vec3 v[5];
    v[0] = osg::Vec3(0,0,0);
    v[1] = osg::Vec3(10,-50,0);
    v[2] = osg::Vec3(30,-10,20);
    v[3] = osg::Vec3(-10,20,-20);
    v[4] = osg::Vec3(0,0,0);
    osg::MatrixTransform* node = new osg::MatrixTransform;
    AnimtkUpdateCallback* callback = new AnimtkUpdateCallback;
    osgAnimation::Vec3CubicBezierKeyframeContainer* keys = callback->_sampler->getOrCreateKeyframeContainer();
    keys->push_back(osgAnimation::Vec3CubicBezierKeyframe(0, osgAnimation::Vec3CubicBezier(
                                                        v[0], // pos
                                                        v[0] + (v[0] - v[3]), // p1
                                                        v[1] - (v[1] - v[0]) // p2
                                                        )));
    keys->push_back(osgAnimation::Vec3CubicBezierKeyframe(2, osgAnimation::Vec3CubicBezier(
                                                        v[1], // pos
                                                        v[1] + (v[1] - v[0]),
                                                        v[2] - (v[2] - v[1])
                                                        )));
    keys->push_back(osgAnimation::Vec3CubicBezierKeyframe(4, osgAnimation::Vec3CubicBezier(
                                                        v[2], // pos
                                                        v[2] + (v[2] - v[1]),
                                                        v[3] - (v[3] - v[2])
                                                        )));
    keys->push_back(osgAnimation::Vec3CubicBezierKeyframe(6, osgAnimation::Vec3CubicBezier(
                                                        v[3], // pos
                                                        v[3] + (v[3] - v[2]),
                                                        v[4] - (v[4] - v[3])
                                                        )));
    keys->push_back(osgAnimation::Vec3CubicBezierKeyframe(8, osgAnimation::Vec3CubicBezier(
                                                        v[4], // pos
                                                        v[4] + (v[4] - v[3]),
                                                        v[0] - (v[0] - v[4])
                                                        )));

    callback->start();
    node->setUpdateCallback(callback);
    node->addChild(setupCube());
    return node;
}

int main (int argc, char* argv[])
{
    osg::ArgumentParser arguments(&argc, argv);
    osgViewer::Viewer viewer(arguments);

    osgGA::TrackballManipulator* manipulator = new osgGA::TrackballManipulator();
    viewer.setCameraManipulator(manipulator);

    osg::Group* root = new osg::Group;
    root->setInitialBound(osg::BoundingSphere(osg::Vec3(10,0,10), 30));
    root->addChild(createAxis());

    osg::MatrixTransform* node = setupAnimtkNode();
    node->addChild(createAxis());
    root->addChild(node);

    viewer.setSceneData( root );
    viewer.realize();

    while (!viewer.done())
    {
        viewer.frame();
    }

}
