/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield 
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

#include <osgProducer/Viewer>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgUtil/Optimizer>

#include <osg/Geode>
#include <osg/Notify>
#include <osg/MatrixTransform>
#include <osg/Switch>
#include <osg/TexMat>
#include <osg/Texture2D>

typedef std::vector<std::string> FileList;

class SlideEventHandler : public osgGA::GUIEventHandler
{
public:

    SlideEventHandler();
    
    META_Object(osgStereImageApp,SlideEventHandler);


    void set(osg::Switch* sw, float offsetX, float offsetY, osg::TexMat* texmatLeft, osg::TexMat* texmatRight, float timePerSlide, bool autoSteppingActive);

    virtual void accept(osgGA::GUIEventHandlerVisitor& v) { v.visit(*this); }

    virtual bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter&);
    
    virtual void getUsage(osg::ApplicationUsage& usage) const;

    virtual void operator()(osg::Node* node, osg::NodeVisitor* nv);
    
    void nextSlide();
    
    void previousSlide();
    
    void scaleImage(float s);
    
    void offsetImage(float ds,float dt);

    void rotateImage(float rx,float ry);

    void initTexMatrices();

protected:

    ~SlideEventHandler() {}
    SlideEventHandler(const SlideEventHandler&,const osg::CopyOp&) {}

    osg::ref_ptr<osg::Switch>   _switch;
    osg::ref_ptr<osg::TexMat>   _texmatLeft;
    osg::ref_ptr<osg::TexMat>   _texmatRight;
    bool                        _firstTraversal;
    unsigned int                _activeSlide;
    double                      _previousTime;
    double                      _timePerSlide;
    bool                        _autoSteppingActive;
    float                       _initSeperationX;
    float                       _currentSeperationX;
    float                       _initSeperationY;
    float                       _currentSeperationY;
        
};

SlideEventHandler::SlideEventHandler():
    _switch(0),
    _texmatLeft(0),
    _texmatRight(0),
    _firstTraversal(true),
    _activeSlide(0),
    _previousTime(-1.0f),
    _timePerSlide(5.0),
    _autoSteppingActive(false)
{
}

void SlideEventHandler::set(osg::Switch* sw, float offsetX, float offsetY, osg::TexMat* texmatLeft, osg::TexMat* texmatRight, float timePerSlide, bool autoSteppingActive)
{
    _switch = sw;
    _switch->setUpdateCallback(this);

    _texmatLeft = texmatLeft;
    _texmatRight = texmatRight;

    _timePerSlide = timePerSlide;
    _autoSteppingActive = autoSteppingActive;    
    
    _initSeperationX = offsetX;
    _currentSeperationX = _initSeperationX;

    _initSeperationY = offsetY;
    _currentSeperationY = _initSeperationY;

    initTexMatrices();

}

bool SlideEventHandler::handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter&)
{
    switch(ea.getEventType())
    {
        case(osgGA::GUIEventAdapter::KEYDOWN):
        {
            if (ea.getKey()=='a')
            {
                _autoSteppingActive = !_autoSteppingActive;
                _previousTime = ea.getTime();
                return true;
            }
            else if (ea.getKey()=='n')
            {
                nextSlide();
                return true;
            }
            else if (ea.getKey()=='p')
            {
                previousSlide();
                return true;
            }
            else if (ea.getKey()=='w')
            {
                scaleImage(0.99f);
                return true;
            }
            else if (ea.getKey()=='s')
            {
                scaleImage(1.01f);
                return true;
            }
            else if (ea.getKey()=='j')
            {
                offsetImage(-0.001f,0.0f);
                return true;
            }
            else if (ea.getKey()=='k')
            {
                offsetImage(0.001f,0.0f);
                return true;
            }
            else if (ea.getKey()=='i')
            {
                offsetImage(0.0f,-0.001f);
                return true;
            }
            else if (ea.getKey()=='m')
            {
                offsetImage(0.0f,0.001f);
                return true;
            }
            else if (ea.getKey()==' ')
            {
                initTexMatrices();
                return true;
            }
            return false;
        }
        case(osgGA::GUIEventAdapter::DRAG):
        case(osgGA::GUIEventAdapter::MOVE):
        {
            static float px = ea.getXnormalized();
            static float py = ea.getYnormalized();
            
            float dx = ea.getXnormalized()-px;
            float dy = ea.getYnormalized()-py;
            
            px = ea.getXnormalized();
            py = ea.getYnormalized();
            
            rotateImage(dx,dy);
            
            return true;
        }

        default:
            return false;
    }
}

void SlideEventHandler::getUsage(osg::ApplicationUsage& usage) const
{
    usage.addKeyboardMouseBinding("Space","Reset the image position to center");
    usage.addKeyboardMouseBinding("a","Toggle on/off the automatic advancement for image to image");
    usage.addKeyboardMouseBinding("n","Advance to next image");
    usage.addKeyboardMouseBinding("p","Move to previous image");
    usage.addKeyboardMouseBinding("q","Zoom into the image");
    usage.addKeyboardMouseBinding("a","Zoom out of the image");
    usage.addKeyboardMouseBinding("j","Reduce horizontal offset");
    usage.addKeyboardMouseBinding("k","Increase horizontal offset");
    usage.addKeyboardMouseBinding("m","Reduce vertical offset");
    usage.addKeyboardMouseBinding("i","Increase vertical offset");
}

void SlideEventHandler::operator()(osg::Node* node, osg::NodeVisitor* nv)
{
    if (_autoSteppingActive && nv->getFrameStamp())
    {
        double time = nv->getFrameStamp()->getReferenceTime();
        
        if (_firstTraversal)
        {
            _firstTraversal = false;
            _previousTime = time;
        }
        else if (time-_previousTime>_timePerSlide)
        {
            _previousTime = time;
            
            nextSlide();
        }
        
    }

    traverse(node,nv);
}

void SlideEventHandler::nextSlide()
{
    if (_switch->getNumChildren()==0) return;

    ++_activeSlide;
    if (_activeSlide>=_switch->getNumChildren()) _activeSlide = 0;

    _switch->setSingleChildOn(_activeSlide);
}

void SlideEventHandler::previousSlide()
{
    if (_switch->getNumChildren()==0) return;

    if (_activeSlide==0) _activeSlide = _switch->getNumChildren()-1;
    else --_activeSlide;

    _switch->setSingleChildOn(_activeSlide);
}

void SlideEventHandler::scaleImage(float s)
{
    _texmatLeft->setMatrix(_texmatLeft->getMatrix()*osg::Matrix::translate(-0.5f,-0.5f,0.0f)*osg::Matrix::scale(s,s,1.0f)*osg::Matrix::translate(0.5f,0.5f,0.0f));
    _texmatRight->setMatrix(_texmatRight->getMatrix()*osg::Matrix::translate(-0.5f,-0.5f,0.0f)*osg::Matrix::scale(s,s,1.0f)*osg::Matrix::translate(0.5f,0.5f,0.0f));
}

void SlideEventHandler::offsetImage(float ds,float dt)
{
    _currentSeperationX+=ds;
    _currentSeperationY+=dt;
    osg::notify(osg::NOTICE)<<"image offset x = "<<_currentSeperationX<<"  y ="<<_currentSeperationY<<std::endl;
    _texmatLeft->setMatrix(_texmatLeft->getMatrix()*osg::Matrix::translate(ds,dt,0.0f));
    _texmatRight->setMatrix(_texmatRight->getMatrix()*osg::Matrix::translate(-ds,-dt,0.0f));
}

void SlideEventHandler::rotateImage(float rx,float ry)
{
    const float scale = 0.5f;
    _texmatLeft->setMatrix(_texmatLeft->getMatrix()*osg::Matrix::translate(-rx*scale,-ry*scale,0.0f));
    _texmatRight->setMatrix(_texmatRight->getMatrix()*osg::Matrix::translate(-rx*scale,-ry*scale,0.0f));
}

void SlideEventHandler::initTexMatrices()
{
    _texmatLeft->setMatrix(osg::Matrix::translate(_initSeperationX,_initSeperationY,0.0f));
    _texmatRight->setMatrix(osg::Matrix::translate(-_initSeperationX,-_initSeperationY,0.0f));
}


osg::Geode* createSectorForImage(osg::Image* image, osg::TexMat* texmat, float s,float t, float radius, float height, float length)
{

    int numSegments = 20;
    float Theta = length/radius;
    float dTheta = Theta/(float)(numSegments-1);
    
    float ThetaZero = height*s/(t*radius);
    
    // set up the texture.
    osg::Texture2D* texture = new osg::Texture2D;
    texture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
    texture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);
    texture->setImage(image);

    // set up the drawstate.
    osg::StateSet* dstate = new osg::StateSet;
    dstate->setMode(GL_CULL_FACE,osg::StateAttribute::OFF);
    dstate->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
    dstate->setTextureAttributeAndModes(0, texture,osg::StateAttribute::ON);
    dstate->setTextureAttribute(0, texmat);

    // set up the geoset.
    osg::Geometry* geom = new osg::Geometry;
    geom->setStateSet(dstate);

    osg::Vec3Array* coords = new osg::Vec3Array();
    osg::Vec2Array* tcoords = new osg::Vec2Array();
   
    int i;
    float angle = -Theta/2.0f;
    for(i=0;
        i<numSegments;
        ++i, angle+=dTheta)
    {
        coords->push_back(osg::Vec3(sinf(angle)*radius,cosf(angle)*radius,height*0.5f)); // top
        coords->push_back(osg::Vec3(sinf(angle)*radius,cosf(angle)*radius,-height*0.5f)); // bottom.
        
        tcoords->push_back(osg::Vec2(angle/ThetaZero+0.5f,1.0f)); // top
        tcoords->push_back(osg::Vec2(angle/ThetaZero+0.5f,0.0f)); // bottom.

    }

    osg::Vec4Array* colors = new osg::Vec4Array();
    colors->push_back(osg::Vec4(1.0f,1.0f,1.0f,1.0f));

    osg::DrawArrays* elements = new osg::DrawArrays(osg::PrimitiveSet::QUAD_STRIP,0,coords->size());

    

    geom->setVertexArray(coords);
    geom->setTexCoordArray(0,tcoords);
    geom->setColorArray(colors);
    geom->setColorBinding(osg::Geometry::BIND_OVERALL);

    geom->addPrimitiveSet(elements);

    // set up the geode.
    osg::Geode* geode = new osg::Geode;
    geode->addDrawable(geom);

    return geode;

}

// create a switch containing a set of child each containing a 
// stereo image pair.
osg::Switch* createScene(const FileList& fileList, osg::TexMat* texmatLeft, osg::TexMat* texmatRight, float radius, float height, float length)
{
    osg::Switch* sw = new osg::Switch;

    // load the images.
    for(unsigned int i=0;i+1<fileList.size();i+=2)
    {
        osg::ref_ptr<osg::Image> imageLeft = osgDB::readImageFile(fileList[i]);
        osg::ref_ptr<osg::Image> imageRight = osgDB::readImageFile(fileList[i+1]);
        if (imageLeft.valid() && imageRight.valid())
        {
            float average_s = (imageLeft->s()+imageRight->s())*0.5f;
            float average_t = (imageLeft->t()+imageRight->t())*0.5f;

            osg::Geode* geodeLeft = createSectorForImage(imageLeft.get(),texmatLeft,average_s,average_t, radius, height, length);
            geodeLeft->setNodeMask(0x01);

            osg::Geode* geodeRight = createSectorForImage(imageRight.get(),texmatRight,average_s,average_t, radius, height, length);
            geodeRight->setNodeMask(0x02);

            osg::ref_ptr<osg::Group> imageGroup = new osg::Group;
            
            imageGroup->addChild(geodeLeft);
            imageGroup->addChild(geodeRight);
            
            sw->addChild(imageGroup.get());
        }
        else
        {
            std::cout << "Warning: Unable to load both image files, '"<<fileList[i]<<"' & '"<<fileList[i+1]<<"', required for stereo imaging."<<std::endl;
        }
    }


    if (sw->getNumChildren()>0)
    {
        // select first child.
        sw->setSingleChildOn(0);
    }

    return sw;
}

int main( int argc, char **argv )
{

    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);
    
    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is the example which demonstrates use node masks to create stereo images.");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] image_file_left_eye image_file_right_eye");
    arguments.getApplicationUsage()->addCommandLineOption("-d <float>","Time delay in sceonds between the display of successive image pairs when in auto advance mode.");
    arguments.getApplicationUsage()->addCommandLineOption("-a","Enter auto advance of image pairs on start up.");
    arguments.getApplicationUsage()->addCommandLineOption("-x <float>","Horizontal offset of left and right images.");
    arguments.getApplicationUsage()->addCommandLineOption("-y <float>","Vertical offset of left and right images.");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
    

    // construct the viewer.
    osgProducer::Viewer viewer(arguments);

    // set up the value with sensible default event handlers.
    viewer.setUpViewer(osgProducer::Viewer::ESCAPE_SETS_DONE);

    // register the handler to add keyboard and mosue handling.
    SlideEventHandler* seh = new SlideEventHandler();
    viewer.getEventHandlerList().push_front(seh);


    // get details on keyboard and mouse bindings used by the viewer.
    viewer.getUsage(*arguments.getApplicationUsage());

    // read any time delay argument.
    float timeDelayBetweenSlides = 5.0f;
    while (arguments.read("-d",timeDelayBetweenSlides)) {}

    bool autoSteppingActive = false;
    while (arguments.read("-a")) autoSteppingActive = true;

    float offsetX=0.0f;
    while (arguments.read("-x",offsetX)) {}

    float offsetY=0.0f;
    while (arguments.read("-y",offsetY)) {}

    // if user request help write it out to cout.
    if (arguments.read("-h") || arguments.read("--help"))
    {
        arguments.getApplicationUsage()->write(std::cout);
        return 1;
    }

    // any option left unread are converted into errors to write out later.
    arguments.reportRemainingOptionsAsUnrecognized();

    // report any errors if they have occured when parsing the program aguments.
    if (arguments.errors())
    {
        arguments.writeErrorMessages(std::cout);
        return 1;
    }
    
    if (arguments.argc()<=1)
    {
        arguments.getApplicationUsage()->write(std::cout,osg::ApplicationUsage::COMMAND_LINE_OPTION);
        return 1;
    }

    // extract the filenames from the arguments list.
    FileList fileList;
    for(int pos=1;pos<arguments.argc();++pos)
    {
        if (arguments.isString(pos)) fileList.push_back(arguments[pos]);
    }

    if (fileList.size()<2)
    {
        return 1;
    }

    // set up the use of stereo by default.
    osg::DisplaySettings* ds = viewer.getDisplaySettings();
    if (!ds) ds = osg::DisplaySettings::instance();
    if (ds) ds->setStereo(true);

    // create the windows and run the threads.
    viewer.realize();

    // now the windows have been realized we switch off the cursor to prevent it
    // distracting the people seeing the stereo images.
    float fovy = 1.0f;
    for( unsigned int i = 0; i < viewer.getNumberOfCameras(); i++ )
    {
        Producer::Camera* cam = viewer.getCamera(i);
        Producer::RenderSurface* rs = cam->getRenderSurface();
        rs->useCursor(false);
        fovy = osg::DegreesToRadians(cam->getLensVerticalFov());
    }

    float radius = 1.0f;
    float height = 2*radius*tan(fovy*0.5f);
    float length = osg::PI*radius;  // half a cylinder.

    // use a texure matrix to control the placement of the image.
    osg::TexMat* texmatLeft = new osg::TexMat;
    osg::TexMat* texmatRight = new osg::TexMat;

    // creat the scene from the file list.
    osg::ref_ptr<osg::Switch> rootNode = createScene(fileList,texmatLeft,texmatRight,radius,height,length);


    //osgDB::writeNodeFile(*rootNode,"test.osg");

    // set the scene to render
    viewer.setSceneData(rootNode.get());


    viewer.getCullSettings().setCullMask(0xffffffff);
    viewer.getCullSettings().setCullMaskLeft(0x00000001);
    viewer.getCullSettings().setCullMaskRight(0x00000002);

    // set all the sceneview's up so that their left and right add cull masks are set up.
    for(osgProducer::OsgCameraGroup::SceneHandlerList::iterator itr=viewer.getSceneHandlerList().begin();
        itr!=viewer.getSceneHandlerList().end();
        ++itr)
    {
        osgUtil::SceneView* sceneview = (*itr)->getSceneView();
        sceneview->setFusionDistance(osgUtil::SceneView::USE_FUSION_DISTANCE_VALUE,radius);
    }


    // set up the SlideEventHandler.
    seh->set(rootNode.get(),offsetX,offsetY,texmatLeft,texmatRight,timeDelayBetweenSlides,autoSteppingActive);
    
    
    osg::Matrix homePosition;
    homePosition.makeLookAt(osg::Vec3(0.0f,0.0f,0.0f),osg::Vec3(0.0f,1.0f,0.0f),osg::Vec3(0.0f,0.0f,1.0f));
        
    while( !viewer.done() )
    {
        // wait for all cull and draw threads to complete.
        viewer.sync();

        // update the scene by traversing it with the the update visitor which will
        // call all node update callbacks and animations.
        viewer.update();
         
        viewer.setView(homePosition);

        // fire off the cull and draw traversals of the scene.
        viewer.frame();
        
    }
    
    // wait for all cull and draw threads to complete.
    viewer.sync();

    // run a clean up frame to delete all OpenGL objects.
    viewer.cleanup_frame();

    // wait for all the clean up frame to complete.
    viewer.sync();
    
    return 0;
}

