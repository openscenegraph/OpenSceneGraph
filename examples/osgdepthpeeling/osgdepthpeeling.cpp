/*
  Steffen Frey
  Fachpraktikum Graphik-Programmierung 2007
  Institut fuer Visualisierung und Interaktive Systeme
  Universitaet Stuttgart
 */

#include <osg/GLExtensions>
#include <osg/Node>
#include <osg/Geometry>
#include <osg/Notify>
#include <osg/MatrixTransform>
#include <osg/AnimationPath>

#include <osgDB/ReadFile>
#include <osgViewer/Viewer>
#include <osgGA/TrackballManipulator>

#include <iostream>

#include "DePee.h"

/*!
  Handles keyboard events.
  Maintains a copy of the DePee object and part of its internal state
  Used for example to set sketchiness, color, add or remove a depth peeling pass
 */
class KeyboardEventHandler : public osgGA::GUIEventHandler
{
public:

  KeyboardEventHandler(DePee* dePee)
  {
    _apc = 0;
    _dePee = dePee;
    _sketchy = false;
    _sketchiness = 1.0;
    _colored = false;
    _edgy = true;
    _crayon = false;
    _dePee->setSketchy(_sketchy);
    _dePee->setColored(_colored);
  }

  virtual bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter&)
  {
    switch(ea.getEventType())
      {

      case(osgGA::GUIEventAdapter::KEYDOWN):
	{
	  if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Space)
	    {
	      if(_apc)
		_apc->setPause(!_apc->getPause());
	      return true;
	    }
	  else if (ea.getKey() == 'a')
	    {
	      _dePee->addDePeePass();
	      return true;
	    }
	  else if (ea.getKey() == 'r')
	    {
	      _dePee->remDePeePass();
	      return true;
	    }
	  else if (ea.getKey() == 'c')
	    {
	      _colored = !_colored;
	      _dePee->setColored(_colored);
	      return true;
	    }
	  else if (ea.getKey() == 's')
	    {
	      _sketchy = !_sketchy;
	      _dePee->setSketchy(_sketchy);
	      return true;
	    }

	  else if (ea.getKey() == 'e')
	    {
	      _edgy = !_edgy;
	      _dePee->setEdgy(_edgy);
	      return true;
	    }
	  else if (ea.getKey() == 'f')
	    {
	      return true;
	    }
	  else if (ea.getKey() == '+')
	    {
	      _sketchiness += 0.5;
	      _dePee->setSketchiness(_sketchiness);
	    }
	  else if (ea.getKey() == '-')
	    {
	      _sketchiness -= 0.5;
	      if(_sketchiness < 0.0)
		_sketchiness = 0.0;
	      _dePee->setSketchiness(_sketchiness);
	    }

	  else if (ea.getKey() == 'y')
	    {
	      _crayon = !_crayon;
	      _dePee->setCrayon(_crayon);
	    }

	  break;
	}

      default:
	break;

      }
    return false;
  }
  void registerAnimationPathCallback(osg::AnimationPathCallback* apc)
  {
    _apc = apc;
  }
private:
  DePee* _dePee;
  bool _sketchy;
  bool _colored;
  bool _edgy;
  bool _crayon;
  double _sketchiness;
  osg::AnimationPathCallback* _apc;
};


/*!
  Handles mouse events.
  Maintains a copy of the DePee object and part of its internal state
  Used to rotate the object
 */
class MouseEventHandler : public osgGA::GUIEventHandler
{
public:

  MouseEventHandler(DePee* dePee)
  {
    _dePee = dePee;
  }

  virtual bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter&)
  {
    switch(ea.getEventType())
      {
	//mouse
      case(osgGA::GUIEventAdapter::DRAG):
	{
	  rotate(ea.getXnormalized(), ea.getYnormalized());
	  break;
	}
      case(osgGA::GUIEventAdapter::MOVE):
	_prevX = ea.getXnormalized();
	_prevY = ea.getYnormalized();
	break;

      default:
	break;

      }
    return false;
  }
  void registerModelGroupTransform(osg::MatrixTransform* modelGroupTransform)
  {
    _modelGroupTransform = modelGroupTransform;
    _rotCenter = modelGroupTransform->getBound().center();
  }
private:
  void rotate(float x, float y)
  {
    osg::Matrix baseMatrix = _modelGroupTransform->getMatrix();

    baseMatrix.preMultTranslate(_rotCenter);
    baseMatrix.preMultRotate(osg::Quat((x - _prevX) * 3, osg::Vec3d(0.0, 0.0, 1.0)));
    baseMatrix.preMultRotate(osg::Quat(-(y - _prevY) * 3, (baseMatrix * osg::Vec3d(1.0, 0.0, 0.0))));
    baseMatrix.preMultTranslate(-_rotCenter);

    _modelGroupTransform->setMatrix(baseMatrix);

    _prevX = x;
    _prevY = y;
  };

  DePee* _dePee;

  float _prevX;
  float _prevY;

  osg::Vec3 _rotCenter;
  osg::MatrixTransform* _modelGroupTransform;
};



int main( int argc, char **argv )
{
  // use an ArgumentParser object to manage the program arguments.
  osg::ArgumentParser arguments(&argc,argv);

  // set up the usage document, in case we need to print out how to use this program.
  arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is the example which demonstrates Depth Peeling");
  arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" filename");


  // construct the viewer
  osgViewer::Viewer viewer(arguments);

  // any option left unread are converted into errors to write out later.
  arguments.reportRemainingOptionsAsUnrecognized();

  // report any errors if they have occurred when parsing the program arguments.
  if (arguments.errors())
  {
      arguments.writeErrorMessages(std::cout);
      return 1;
  }

  if (arguments.argc()<=1 || arguments.argc() > 3)
  {
        arguments.getApplicationUsage()->write(std::cout,osg::ApplicationUsage::COMMAND_LINE_OPTION);
        return 1;
  }


  //only displays a textured quad
  viewer.getCamera()->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);

  // read the model to do depth peeling with
  osg::ref_ptr<osg::Node> loadedModel = osgDB::readRefNodeFile(arguments.argv()[1]);

  if (!loadedModel)
    return 1;

  // create a transform to spin the model.
  osg::MatrixTransform* modelGroupTransform = new osg::MatrixTransform;
  osg::Group* modelGroup = new osg::Group;
  modelGroupTransform->addChild(modelGroup);
  modelGroup->addChild(loadedModel);

  osg::Group* rootNode = new osg::Group();

  // add model to the viewer.
  viewer.setSceneData(rootNode);

  // Depth peel example only works on a single graphics context right now
  // so open up viewer on single screen to prevent problems
  viewer.setUpViewOnSingleScreen(0);

  // create the windows and run the threads.
  viewer.realize();

  unsigned int width = 1280;
  unsigned int height = 1280;
  osgViewer::Viewer::Windows windows;
  viewer.getWindows(windows);
  if (!windows.empty())
  {
    width = windows.front()->getTraits()->width;
    height = windows.front()->getTraits()->height;
  }


  osg::ref_ptr<DePee> dePee = new DePee(rootNode,
			        modelGroupTransform,
			        width,
			        height);

  //create event handlers
  KeyboardEventHandler* keyboardEventHandler = new KeyboardEventHandler(dePee.get());
  MouseEventHandler* mouseEventHandler = new MouseEventHandler(dePee.get());
  viewer.addEventHandler(keyboardEventHandler);
  viewer.addEventHandler(mouseEventHandler);

  //viewer.setCameraManipulator(new osgGA::TrackballManipulator);

  osg::StateSet* stateset = modelGroupTransform->getOrCreateStateSet();

  stateset->setMode(GL_BLEND, osg::StateAttribute::OFF);

  //create new animation callback for autmatic object rotation
  osg::AnimationPathCallback* apc = new osg::AnimationPathCallback(modelGroupTransform->getBound().center(),osg::Vec3(0.0f,0.0f,1.0f),osg::inDegrees(45.0f));
  apc->setPause(true);
  modelGroupTransform->setUpdateCallback(apc);

  keyboardEventHandler->registerAnimationPathCallback(apc);
  mouseEventHandler->registerModelGroupTransform(modelGroupTransform);

  //setup stuff that is necessary for measuring fps
  osg::Timer_t current_tick, previous_tick = 1;
  double* fps = new double;
  dePee->setFPS(fps);

  while(!viewer.done())
  {
    current_tick = osg::Timer::instance()->tick();

    *fps = 1.0/osg::Timer::instance()->delta_s(previous_tick,current_tick);
    dePee->updateHUDText();

    previous_tick = current_tick;

    // fire off the cull and draw traversals of the scene.
    viewer.frame();
  }

  return 0;
}
