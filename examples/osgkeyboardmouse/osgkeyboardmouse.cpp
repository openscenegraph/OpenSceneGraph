// C++ source file - (C) 2003 Robert Osfield, released under the OSGPL.
//
// Simple example of use of Producer::RenderSurface + KeyboardMouseCallback + SceneView
// example that provides the user with control over view position with basic picking.

#include <Producer/RenderSurface>
#include <Producer/KeyboardMouse>
#include <Producer/Trackball>

#include <osg/Timer>
#include <osg/io_utils>

#include <osgUtil/SceneView>
#include <osgUtil/IntersectVisitor>

#include <osgDB/ReadFile>

#include <osgFX/Scribe>


class MyKeyboardMouseCallback : public Producer::KeyboardMouseCallback
{
public:

    MyKeyboardMouseCallback(osgUtil::SceneView* sceneView) :
        Producer::KeyboardMouseCallback(),
        _mx(0.0f),_my(0.0f),_mbutton(0),
        _done(false),
        _trackBall(new Producer::Trackball),
        _sceneView(sceneView)
    {
        resetTrackball();
        _mouseMovingOnPreviousRelease = false;
    }

    virtual void specialKeyPress( Producer::KeyCharacter key )
    {
        if (key==Producer::KeyChar_Escape)
                    shutdown();
    }

    virtual void shutdown()
    {
        _done = true; 
    }

    virtual void keyPress( Producer::KeyCharacter key)
    {
        if (key==' ') resetTrackball();
    }

    virtual void mouseMotion( float mx, float my ) 
    {
        _mx = mx;
        _my = my;
    }

    virtual void buttonPress( float mx, float my, unsigned int mbutton ) 
    {
         _mx = mx;
         _my = my;
         _mbutton |= (1<<(mbutton-1));
         
         _mx_buttonPress = _mx;
         _my_buttonPress = _my;
    }
    virtual void buttonRelease( float mx, float my, unsigned int mbutton ) 
    {
        _mx = mx;
        _my = my;
        _mbutton &= ~(1<<(mbutton-1));
        
        if (_mx==_mx_buttonPress && _my_buttonPress==_my)
        {
            if (!_mouseMovingOnPreviousRelease)
            {
                // button press and release without moving so assume this means
                // the users wants to pick.
                pick(_mx,_my);
            }
            _mouseMovingOnPreviousRelease = false;
        }
        else
        {
            _mouseMovingOnPreviousRelease = true;
        }
    }

    bool done() { return _done; }
    float mx()  { return _mx; }
    float my()  { return _my; }
    unsigned int mbutton()  { return _mbutton; }
    
    void resetTrackball()
    {
        osg::Node* scene = _sceneView->getSceneData();
        if (scene)
        {
            const osg::BoundingSphere& bs = scene->getBound();
            if (bs.valid())
            {
                _trackBall->reset();
                _trackBall->setOrientation( Producer::Trackball::Z_UP );
                _trackBall->setDistance(bs.radius()*2.0f);
                _trackBall->translate(-bs.center().x(),-bs.center().y(),-bs.center().z());
            }
        }
    }
    
    osg::Matrixd getViewMatrix()
    {
        _trackBall->input( mx(), my(), mbutton() );
        return osg::Matrixd(_trackBall->getMatrix().ptr());
    }
    
    void pick(float x, float y)
    {
        osg::Node* scene = _sceneView->getSceneData();
        if (scene)
        {
            std::cout<<"Picking "<<x<<"\t"<<y<<std::endl;

            int origX, origY, width, height;
            _sceneView->getViewport(origX,origY,width,height);

            // convert Producer's non dimensional x,y coords back into pixel coords.
            int pixel_x = (int)((x+1.0f)*0.5f*(float)width);
            int pixel_y = (int)((y+1.0f)*0.5f*(float)height);
            
            
            osgUtil::PickVisitor pick(_sceneView->getViewport(),
                                      _sceneView->getProjectionMatrix(),
                                      _sceneView->getViewMatrix(),
                                      pixel_x, pixel_y);

            scene->accept(pick);
            
            osgUtil::PickVisitor::LineSegmentHitListMap& segHitList = pick.getSegHitList();
            if (!segHitList.empty() && !segHitList.begin()->second.empty())
            {
                std::cout<<"Got hits"<<std::endl;
                
                // get the hits for the first segment
                osgUtil::PickVisitor::HitList& hits = segHitList.begin()->second;
                
                // just take the first hit - nearest the eye point.
                osgUtil::Hit& hit = hits.front();
                
                osg::NodePath& nodePath = hit._nodePath;
                osg::Node* node = (nodePath.size()>=1)?nodePath[nodePath.size()-1]:0;
                osg::Group* parent = (nodePath.size()>=2)?dynamic_cast<osg::Group*>(nodePath[nodePath.size()-2]):0;

                if (node) std::cout<<"  Hits "<<node->className()<<" nodePath size"<<nodePath.size()<<std::endl;

                // now we try to decorate the hit node by the osgFX::Scribe to show that its been "picked"
                if (parent && node)
                {
                
                    std::cout<<"  parent "<<parent->className()<<std::endl;
                    
                    osgFX::Scribe* parentAsScribe = dynamic_cast<osgFX::Scribe*>(parent);
                    if (!parentAsScribe)
                    {
                        // node not already picked, so highlight it with an osgFX::Scribe
                        osgFX::Scribe* scribe = new osgFX::Scribe();
                        scribe->addChild(node);
                        parent->replaceChild(node,scribe);
                    }
                    else
                    {
                        // node already picked so we want to remove scribe to unpick it.
                        osg::Node::ParentList parentList = parentAsScribe->getParents();
                        for(osg::Node::ParentList::iterator itr=parentList.begin();
                            itr!=parentList.end();
                            ++itr)
                        {
                            (*itr)->replaceChild(parentAsScribe,node);
                        }
                    }
                }

            }
            
        }        
        
    }

private:

    float                               _mx, _my;
    float                               _mx_buttonPress, _my_buttonPress;
    unsigned int                        _mbutton;
    bool                                _mouseMovingOnPreviousRelease;
    
    bool                                _done;
    
    osg::ref_ptr<Producer::Trackball>   _trackBall;
    osg::ref_ptr<osgUtil::SceneView>    _sceneView;
};

int main( int argc, char **argv )
{
    if (argc<2) 
    {
        std::cout << argv[0] <<": requires filename argument." << std::endl;
        return 1;
    }

    // load the scene.
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFile(argv[1]);
    if (!loadedModel) 
    {
        std::cout << argv[0] <<": No data loaded." << std::endl;
        return 1;
    }
    
    // create the window to draw to.
    osg::ref_ptr<Producer::RenderSurface> renderSurface = new Producer::RenderSurface;
    renderSurface->setWindowName("osgkeyboardmouse");
    renderSurface->setWindowRectangle(100,100,800,600);
    renderSurface->useBorder(true);
    renderSurface->realize();
    

    // create the view of the scene.
    osg::ref_ptr<osgUtil::SceneView> sceneView = new osgUtil::SceneView;
    sceneView->setDefaults();
    sceneView->setSceneData(loadedModel.get());


    // set up a KeyboardMouse to manage the events comming in from the RenderSurface
    osg::ref_ptr<Producer::KeyboardMouse>  kbm = new Producer::KeyboardMouse(renderSurface.get());

    // create a KeyboardMouseCallback to handle the mouse events within this applications
    osg::ref_ptr<MyKeyboardMouseCallback> kbmcb = new MyKeyboardMouseCallback(sceneView.get());


    // record the timer tick at the start of rendering.    
    osg::Timer_t start_tick = osg::Timer::instance()->tick();
    
    unsigned int frameNum = 0;

    // main loop (note, window toolkits which take control over the main loop will require a window redraw callback containing the code below.)
    while( renderSurface->isRealized() && !kbmcb->done())
    {
        // set up the frame stamp for current frame to record the current time and frame number so that animtion code can advance correctly
        osg::FrameStamp* frameStamp = new osg::FrameStamp;
        frameStamp->setReferenceTime(osg::Timer::instance()->delta_s(start_tick,osg::Timer::instance()->tick()));
        frameStamp->setFrameNumber(frameNum++);
        
        // pass frame stamp to the SceneView so that the update, cull and draw traversals all use the same FrameStamp
        sceneView->setFrameStamp(frameStamp);

        // pass any keyboard mouse events onto the local keyboard mouse callback.        
        kbm->update( *kbmcb );
        
        // set the view
        sceneView->setViewMatrix(kbmcb->getViewMatrix());

        // update the viewport dimensions, incase the window has been resized.
        sceneView->setViewport(0,0,renderSurface->getWindowWidth(),renderSurface->getWindowHeight());

        // do the update traversal the scene graph - such as updating animations
        sceneView->update();
        
        // do the cull traversal, collect all objects in the view frustum into a sorted set of rendering bins
        sceneView->cull();
        
        // draw the rendering bins.
        sceneView->draw();

        // Swap Buffers
        renderSurface->swapBuffers();
    }

    return 0;
}

