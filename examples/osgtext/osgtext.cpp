/* OpenSceneGraph example, osgtext.
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
*  THE SOFTWARE.
*/

#include <osgUtil/Optimizer>

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgDB/Registry>

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include <osg/Geode>
#include <osg/Camera>
#include <osg/ShapeDrawable>
#include <osg/Sequence>
#include <osg/PolygonMode>
#include <osg/io_utils>

#include <osgText/Font>
#include <osgText/Text>







class UpdateTextOperation : public osg::Operation
{
public:

  UpdateTextOperation(osg::Group* group):        
      Operation("UpdateTextOperation", true),
        _group(group),
        _maxNumChildren(200),
        _maxNumTextPerGeode(10)
      {
      }

      virtual void operator () (osg::Object* callingObject)
      {
        // decided which method to call according to whole has called me.
        osgViewer::Viewer* viewer = dynamic_cast<osgViewer::Viewer*>(callingObject);

        if (viewer) update();
        else load();
      }

      void update()
      {
        // osg::notify(osg::NOTICE)<<"*** Doing update"<<std::endl;

        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);

        if (_mergeSubgraph.valid())
        {
          _group->addChild(_mergeSubgraph.get());

          _mergeSubgraph = 0;

          if (_group->getNumChildren()>_maxNumChildren)
          {
            osg::Geode* geode = dynamic_cast<osg::Geode*>(_group->getChild(0));
            if (geode)
            {
              _availableSubgraph.push_back(geode);
              geode->removeDrawables(0,geode->getNumDrawables());
            }
            _group->removeChild(0,1);
          }

          _waitOnMergeBlock.release();
        }        
      }

      void load()
      {

        // osg::notify(osg::NOTICE)<<"Doing load"<<std::endl;

        osg::ref_ptr<osg::Geode> geode;
        {
          OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
          if (!_availableSubgraph.empty())
          {
            geode = _availableSubgraph.front();
            _availableSubgraph.pop_front();
          }
        }

        if (!geode) geode = new osg::Geode;

        for(unsigned int i=0; i<_maxNumTextPerGeode; ++i)
        {
          osg::Vec3 position(float(rand()) / float(RAND_MAX), float(rand()) / float(RAND_MAX), float(i)/float(_maxNumTextPerGeode));

          std::string str;
          unsigned int _numCharacters = 5;
          for(unsigned int ni=0; ni<_numCharacters;++ni)
          {
            str.push_back(char(32.0 + (float(rand())/float(RAND_MAX))*128.0f));
          }

          osgText::Text* text = new osgText::Text;
          text->setDataVariance(osg::Object::DYNAMIC);
          text->setPosition(position);
          text->setFont("times.ttf");
          text->setText(str);
          text->setCharacterSize(0.025f);
          text->setAxisAlignment(osgText::Text::SCREEN);

          geode->addDrawable(text);
        }


        {        
          OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
          _mergeSubgraph = geode;
        }

        // osg::notify(osg::NOTICE)<<"Waiting on merge"<<std::endl;

        _waitOnMergeBlock.block();

      }

      virtual void release()
      {
        _waitOnMergeBlock.release();
      }

      typedef std::list< osg::ref_ptr<osg::Geode> > AvailableList;

      unsigned int                _maxNumChildren;
      unsigned int                _maxNumTextPerGeode;

      OpenThreads::Mutex          _mutex;
      osg::ref_ptr<osg::Group>    _group;
      osg::ref_ptr<osg::Geode>    _mergeSubgraph;
      AvailableList               _availableSubgraph;
      OpenThreads::Block          _waitOnMergeBlock;

      unsigned int                _counter;

};


int main(int argc, char** argv)
{
  osg::ArgumentParser arguments(&argc, argv);


  osg::Referenced::setThreadSafeReferenceCounting(true);

  // construct the viewer.
  osgViewer::Viewer viewer(arguments);

  typedef std::list< osg::ref_ptr<osg::OperationThread> > Threads;

  Threads operationThreads;
  osg::ref_ptr<UpdateTextOperation> updateOperation;

  unsigned int numThreads = 0;
  if (arguments.read("--mt", numThreads) || arguments.read("--mt"))
  {
    // construct a multi-threaded text updating test.
    if (numThreads==0) numThreads = 1;

    // create a group to add everything into.
    osg::Group* mainGroup = new osg::Group;

    osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFiles(arguments);
    mainGroup->addChild(loadedModel.get());

    for(unsigned int i=0; i<numThreads; ++i)
    {
      osg::Group* textGroup = new osg::Group;
      mainGroup->addChild(textGroup);

      // create the background thread
      osg::OperationThread* operationThread = new osg::OperationThread;

      operationThreads.push_back(operationThread);

      // create the operation that will run in the background and
      // sync once per frame with the main viewer loop.
      updateOperation = new UpdateTextOperation(textGroup);

      // add the operation to the operation thread and start it.
      operationThread->add(updateOperation.get());
      operationThread->startThread();

      // add the operation to the viewer to sync once per frame.
      viewer.addUpdateOperation(updateOperation.get());


      // add a unit cube for the text to appear within.
      osg::Geode* geode = new osg::Geode;
      geode->getOrCreateStateSet()->setAttribute(new osg::PolygonMode(osg::PolygonMode::FRONT_AND_BACK,osg::PolygonMode::LINE));
      geode->addDrawable(new osg::ShapeDrawable(new osg::Box(osg::Vec3(0.5f,0.5f,0.5f),1.0)));

      mainGroup->addChild(geode);
    }

    viewer.setSceneData(mainGroup);        
  }
  

#if 0
  osgDB::writeNodeFile(*viewer.getSceneData(),"text.osg");
#endif

  viewer.addEventHandler(new osgViewer::StatsHandler());
  viewer.addEventHandler( new osgViewer::ThreadingHandler );
  viewer.addEventHandler( new osgViewer::WindowSizeHandler );


  viewer.run();

  if (!operationThreads.empty())
  {
    for(Threads::iterator itr = operationThreads.begin();
      itr != operationThreads.begin();
      ++itr)
    {
      (*itr)->cancel();
    }
  }

  return 0;
}

