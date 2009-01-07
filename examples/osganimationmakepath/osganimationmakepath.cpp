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
 *
 *  Authors:
 *   Jeremy Moles  <jeremy@emperorlinux.com>
 *   Cedric Pinson <mornifle@plopbyte.net>
*/

#include <iostream>
#include <osg/io_utils>
#include <osg/Geometry>
#include <osg/Shape>
#include <osg/ShapeDrawable>
#include <osg/Material>
#include <osg/MatrixTransform>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgGA/TrackballManipulator>
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
              nv->getFrameStamp()->getFrameNumber() != _lastUpdate) 
          {

              _lastUpdate = nv->getFrameStamp()->getFrameNumber();
              _currentTime = osg::Timer::instance()->tick();

              if (_playing && _sampler.get() && _sampler->getKeyframeContainer()) 
              {
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
      int _lastUpdate;
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
            nv->getFrameStamp()->getFrameNumber() != _lastUpdate) {

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
    int _lastUpdate;
};

// This won't really give good results in any situation, but it does demonstrate
// on possible "fast" usage...
class MakePathTimeCallback: public AnimtkUpdateCallback 
{
    osg::ref_ptr<osg::Geode> _geode;
    float _lastAdd;
    float _addSeconds;

public:
    MakePathTimeCallback(osg::Geode* geode):
        _geode(geode),
        _lastAdd(0.0f),
        _addSeconds(0.08f) {
    }

    virtual void operator()(osg::Node* node, osg::NodeVisitor* nv) 
    {
        float t = osg::Timer::instance()->delta_s(_startTime, _currentTime);

        if(_lastAdd + _addSeconds <= t && t <= 8.0f) 
        {
            osg::Vec3 pos;

            _sampler->getValueAt(t, pos);

            _geode->addDrawable(new osg::ShapeDrawable(new osg::Sphere(pos, 0.5f)));
            _geode->dirtyBound();

            _lastAdd += _addSeconds;
        }

        AnimtkUpdateCallback::operator()(node, nv);
    }
};

// This will give great results if you DO NOT have VSYNC enabled and can generate
// decent FPS.
class MakePathDistanceCallback: public AnimtkUpdateCallback 
{
    osg::ref_ptr<osg::Geode> _geode;
    osg::Vec3 _lastAdd;
    float _threshold;
    unsigned int _count;

public:
    MakePathDistanceCallback(osg::Geode* geode):
        _geode(geode),
        _threshold(0.5f),
        _count(0) {}

    virtual void operator()(osg::Node* node, osg::NodeVisitor* nv) 
    {
        static bool countReported = false;

        float t = osg::Timer::instance()->delta_s(_startTime, _currentTime);

        osg::Vec3 pos;

        _sampler->getValueAt(t, pos);

        osg::Vec3 distance = _lastAdd - pos;

        if(t <= 8.0f && distance.length() >= _threshold) 
        {
            _geode->addDrawable(new osg::ShapeDrawable(new osg::Sphere(pos, 0.25f)));
            _lastAdd = pos;
            _count++;
        }
        else if(t > 8.0f) 
        {
            if(!countReported) std::cout << "Created " << _count << " nodes." << std::endl;
            countReported = true;
        }

        AnimtkUpdateCallback::operator()(node, nv);
    }
};

osg::StateSet* setupStateSet() 
{
    osg::StateSet* st = new osg::StateSet();
    
    st->setAttributeAndModes(new osg::Material(), true);
    st->setMode(GL_BLEND, true);
    
    AnimtkStateSetUpdateCallback* callback = new AnimtkStateSetUpdateCallback();
    osgAnimation::Vec4KeyframeContainer* keys = callback->_sampler->getOrCreateKeyframeContainer();
    keys->push_back(osgAnimation::Vec4Keyframe(0, osg::Vec4(1,0,0,1)));
    keys->push_back(osgAnimation::Vec4Keyframe(2, osg::Vec4(0.,1,0,1)));
    keys->push_back(osgAnimation::Vec4Keyframe(4, osg::Vec4(0,0,1,1)));
    keys->push_back(osgAnimation::Vec4Keyframe(6, osg::Vec4(0,0,1,1)));
    keys->push_back(osgAnimation::Vec4Keyframe(8, osg::Vec4(0,1,0,1)));
    keys->push_back(osgAnimation::Vec4Keyframe(10, osg::Vec4(1,0,0,1)));
    callback->start();
    st->setUpdateCallback(callback);
    
    return st;
}

osg::MatrixTransform* setupAnimtkNode(osg::Geode* staticGeode) 
{
    osg::Vec3 v[5];

    v[0] = osg::Vec3(  0,   0,   0);
    v[1] = osg::Vec3(20, 40, 60);
    v[2] = osg::Vec3(40, 60, 20);
    v[3] = osg::Vec3(60, 20, 40);
    v[4] = osg::Vec3( 0,  0,  0);

    osg::MatrixTransform* node = new osg::MatrixTransform();
    AnimtkUpdateCallback* callback = new MakePathDistanceCallback(staticGeode);
    osgAnimation::Vec3CubicBezierKeyframeContainer* keys = callback->_sampler->getOrCreateKeyframeContainer();

    keys->push_back(osgAnimation::Vec3CubicBezierKeyframe(0, osgAnimation::Vec3CubicBezier(
                                                        v[0],
                                                        v[0] + (v[0] - v[3]),
                                                        v[1] - (v[1] - v[0])
                                                        )));

    keys->push_back(osgAnimation::Vec3CubicBezierKeyframe(2, osgAnimation::Vec3CubicBezier(
                                                        v[1],
                                                        v[1] + (v[1] - v[0]),
                                                        v[2] - (v[2] - v[1])
                                                        )));

    keys->push_back(osgAnimation::Vec3CubicBezierKeyframe(4, osgAnimation::Vec3CubicBezier(
                                                        v[2],
                                                        v[2] + (v[2] - v[1]),
                                                        v[3] - (v[3] - v[2])
                                                        )));

    keys->push_back(osgAnimation::Vec3CubicBezierKeyframe(6, osgAnimation::Vec3CubicBezier(
                                                        v[3],
                                                        v[3] + (v[3] - v[2]),
                                                        v[4] - (v[4] - v[3])
                                                        )));

    keys->push_back(osgAnimation::Vec3CubicBezierKeyframe(8, osgAnimation::Vec3CubicBezier(
                                                        v[4],
                                                        v[4] + (v[4] - v[3]),
                                                        v[0] - (v[0] - v[4])
                                                        )));

    callback->start();
    node->setUpdateCallback(callback);

    osg::Geode* geode = new osg::Geode();
    
    geode->setStateSet(setupStateSet());
    geode->addDrawable(new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(0.0f, 0.0f, 0.0f), 2)));
    
    node->addChild(geode);

    return node;
}

int main(int argc, char** argv) 
{
    osg::ArgumentParser arguments(&argc, argv);
    osgViewer::Viewer viewer(arguments);
    
    osgGA::TrackballManipulator* tbm = new osgGA::TrackballManipulator();

    viewer.setCameraManipulator(tbm);

    viewer.addEventHandler(new osgViewer::StatsHandler());
    viewer.addEventHandler(new osgViewer::WindowSizeHandler());

    osg::Group* root = new osg::Group();
    osg::Geode* geode = new osg::Geode();

    geode->setStateSet(setupStateSet());

    root->setInitialBound(osg::BoundingSphere(osg::Vec3(10,0,20), 50));
    root->addChild(setupAnimtkNode(geode));
    root->addChild(geode);

    viewer.setSceneData(root);

    // tbm->setDistance(150);

    return viewer.run();
}
