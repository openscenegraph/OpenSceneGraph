/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2003 Robert Osfield 
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
#include <osgDB/ImageOptions>

#include <osgUtil/Optimizer>

#include <osg/Geode>
#include <osg/Notify>
#include <osg/MatrixTransform>
#include <osg/Switch>
#include <osg/TexMat>
#include <osg/Texture2D>
#include <osg/PolygonOffset>

#include <osgText/Text>

#include <sstream>

class ImageReaderWriter : public osgDB::ReaderWriter
{
    public:
        virtual const char* className() { return "Image Reader"; }
        
        
        struct DataReference
        {
            DataReference():
                _fileName(),
                _resolutionX(256),
                _resolutionY(256),
                _center(0.0f,0.0f,0.0f),
                _maximumWidth(1.25f,0.0f,0.0f),
                _maximumHeight(0.0f,0.0f,1.0f),
                _numPointsAcross(10), 
                _numPointsUp(10) {}

            DataReference(const std::string& fileName, unsigned int res):
                _fileName(fileName),
                _resolutionX(res),
                _resolutionY(res),
                _center(0.0f,0.0f,0.0f),
                _maximumWidth(1.25f,0.0f,0.0f),
                _maximumHeight(0.0f,0.0f,1.0f),
                _numPointsAcross(10), 
                _numPointsUp(10) {}
        
            DataReference(const DataReference& rhs):
                _fileName(rhs._fileName),
                _resolutionX(rhs._resolutionX),
                _resolutionY(rhs._resolutionY),
                _center(rhs._center),
                _maximumWidth(rhs._maximumWidth),
                _maximumHeight(rhs._maximumHeight),
                _numPointsAcross(rhs._numPointsAcross), 
                _numPointsUp(rhs._numPointsUp) {}

            std::string     _fileName;
            unsigned int    _resolutionX;
            unsigned int    _resolutionY;
            osg::Vec3       _center;
            osg::Vec3       _maximumWidth; 
            osg::Vec3       _maximumHeight;
            unsigned int    _numPointsAcross; 
            unsigned int    _numPointsUp;
        };
        
        typedef std::map<std::string,DataReference> DataReferenceMap;
        DataReferenceMap _dataReferences;
        
        std::string insertReference(const std::string& fileName, unsigned int res)
        {
	    std::stringstream ostr;
	    ostr<<"res_"<<res<<"_"<<fileName;

            std::string myReference = ostr.str();
            _dataReferences[myReference] = DataReference(fileName,res);
            return myReference;
        }
        
        

        virtual ReadResult readNode(const std::string& fileName, const Options* opt)
        {
            std::cout<<"Trying to read paged image "<<fileName<<std::endl;
            
            DataReferenceMap::iterator itr = _dataReferences.find(fileName);
            if (itr==_dataReferences.end()) return ReaderWriter::ReadResult::FILE_NOT_HANDLED;

            DataReference& dr = itr->second;
            
            // record previous options.
            osg::ref_ptr<osgDB::ReaderWriter::Options> previousOptions = osgDB::Registry::instance()->getOptions();

            osg::ref_ptr<osgDB::ImageOptions> options = new osgDB::ImageOptions;
            options->_destinationImageWindowMode = osgDB::ImageOptions::PIXEL_WINDOW;
            options->_destinationPixelWindow.set(0,0,dr._resolutionX,dr._resolutionY);

            osgDB::Registry::instance()->setOptions(options.get());
            
            osg::Image* image = osgDB::readImageFile(dr._fileName);
            
            // restore previous options.
            osgDB::Registry::instance()->setOptions(previousOptions.get());

            if (image)
            {
            
                float s = options.valid()?options->_sourcePixelWindow.windowWidth:1.0f;
                float t = options.valid()?options->_sourcePixelWindow.windowHeight:1.0f;
            
                osg::Geode* geode = osg::createGeodeForImage(image,s,t);
                return geode;
            
            }
            else
            {
                return ReaderWriter::ReadResult::FILE_NOT_HANDLED;
            }
            
                        
        }

};


// now register with Registry to instantiate the above
// reader/writer.
osgDB::RegisterReaderWriterProxy<ImageReaderWriter> g_ImageReaderWriter;



typedef std::vector<std::string> FileList;

class SlideEventHandler : public osgGA::GUIEventHandler, public osg::NodeCallback
{
public:

    SlideEventHandler();
    
    META_Object(osgStereImageApp,SlideEventHandler);

    void set(osg::Switch* sw, float timePerSlide, bool autoSteppingActive);

    virtual void accept(osgGA::GUIEventHandlerVisitor& v) { v.visit(*this); }

    virtual bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter&);
    
    virtual void getUsage(osg::ApplicationUsage& usage) const;

    virtual void operator()(osg::Node* node, osg::NodeVisitor* nv);
    
    void nextSlide();
    
    void previousSlide();

protected:

    ~SlideEventHandler() {}
    SlideEventHandler(const SlideEventHandler&,const osg::CopyOp&) {}

    osg::ref_ptr<osg::Switch>   _switch;
    bool                        _firstTraversal;
    unsigned int                _activeSlide;
    double                      _previousTime;
    double                      _timePerSlide;
    bool                        _autoSteppingActive;
};

SlideEventHandler::SlideEventHandler():
    _switch(0),
    _firstTraversal(true),
    _activeSlide(0),
    _previousTime(-1.0f),
    _timePerSlide(5.0),
    _autoSteppingActive(false)
{
}

void SlideEventHandler::set(osg::Switch* sw, float timePerSlide, bool autoSteppingActive)
{
    _switch = sw;
    _switch->setUpdateCallback(this);

    _timePerSlide = timePerSlide;
    _autoSteppingActive = autoSteppingActive;    
    
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
                _previousTime = ea.time();
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
            return false;
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

// create a switch containing a set of child each containing a 
// stereo image pair.
osg::Switch* createScene(const FileList& fileList, float height, float length)
{
    osgDB::ReaderWriter* readerWriter = osgDB::Registry::instance()->getReaderWriterForExtension("gdal");
    if (!readerWriter)
    {
        std::cout<<"Error: GDAL plugin not available, cannot preceed with database creation"<<std::endl;
        return 0;
    }

    ImageReaderWriter* rw = g_ImageReaderWriter.get();

    osg::Switch* sw = new osg::Switch;

    typedef std::vector< osg::ref_ptr<osg::Node> > NodeList;
    NodeList nodes;

    // load the images.
    unsigned int i;
    for(i=0;i<fileList.size();++i)
    {
        float cut_off_distance = 8.0f;
        float max_visible_distance = 300.0f;
        
        osg::Vec3 center(0.0f,0.0f,0.0f);

        osgText::Text* text = new osgText::Text;
        text->setFont("fonts/arial.ttf");
        text->setPosition(center);
        text->setCharacterSize(0.1f);
        text->setAlignment(osgText::Text::CENTER_CENTER);
        text->setAxisAlignment(osgText::Text::XZ_PLANE);
        text->setText(fileList[i]);

        osg::Geode* geode = new osg::Geode;
        geode->addDrawable(text);
        
        osg::PagedLOD* pagedlod = new osg::PagedLOD;
        pagedlod->setCenter(center);
        pagedlod->setRadius(1.6f);
        pagedlod->setNumChildrenThatCannotBeExpired(2);
        
        pagedlod->setRange(0,max_visible_distance,1e7);
        pagedlod->addChild(geode);
        
        pagedlod->setRange(1,cut_off_distance,max_visible_distance);
        pagedlod->setFileName(1,rw->insertReference(fileList[i],128));

        pagedlod->setRange(2,0.0f,cut_off_distance);
        pagedlod->setFileName(2,rw->insertReference(fileList[i],1024));

        nodes.push_back(pagedlod);
    }


    if (nodes.empty()) return 0;
    
    osg::Group* front = new osg::Group;
    sw->addChild(front);
    
    unsigned int nodes_across = (unsigned int)ceilf(sqrtf((float)nodes.size()*1.25));
    unsigned int nodes_down = (unsigned int)ceilf((float)nodes.size()/(float)nodes_across);
    
    float scale = 1.0f/(float)nodes_down;
    
    osg::Vec3 down_delta(0.0f,0.0f,-scale);
    osg::Vec3 across_delta(scale*1.25,0.0f,0.0f);
    osg::Vec3 leftMargin(-down_delta*((float)nodes_down*0.5f)-across_delta*((float)nodes_across*0.5f));

    osg::Vec3 pos = leftMargin;
    i=0;
    
    // front cover background
    {
        osg::Geometry* geometry = createTexturedQuadGeometry(osg::Vec3(-1.25f,0.0f,-1.0f),osg::Vec3(2.5f,0.0f,0.0f),osg::Vec3(0.0f,0.0f,2.0f));
        osg::Geode* background = new osg::Geode;
        background->addDrawable(geometry);
        front->addChild(background);
        
        osg::StateSet* stateset = geometry->getOrCreateStateSet();
        stateset->setAttributeAndModes(new osg::PolygonOffset(2.0f,2.0f),osg::StateAttribute::ON);
        stateset->setTextureAttributeAndModes(0,new osg::Texture2D(osgDB::readImageFile("lz.rgb")),osg::StateAttribute::ON);
    }
    
    NodeList::iterator itr;
    for(itr=nodes.begin();
        itr!=nodes.end();
        ++itr)
    {
        osg::MatrixTransform* mt = new osg::MatrixTransform;
        mt->setMatrix(osg::Matrix::scale(scale*0.45f,scale*0.45f,scale*0.45f)*osg::Matrix::translate(pos));
        mt->addChild(itr->get());
        front->addChild(mt);

        i++;
        if ((i%nodes_across)==0)
        {
            leftMargin += down_delta;
            pos = leftMargin;
        }
        else pos += across_delta;
    }
    
    
    for(itr=nodes.begin();
        itr!=nodes.end();
        ++itr)
    {
        sw->addChild(itr->get());
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
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] image_file [image_file]");
    arguments.getApplicationUsage()->addCommandLineOption("-d <float>","Time delay in sceonds between the display of successive image pairs when in auto advance mode.");
    arguments.getApplicationUsage()->addCommandLineOption("-a","Enter auto advance of image pairs on start up.");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
    

    // construct the viewer.
    osgProducer::Viewer viewer(arguments);

    // set up the value with sensible default event handlers.
    //viewer.setUpViewer(osgProducer::Viewer::ESCAPE_SETS_DONE);
    viewer.setUpViewer();

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


    // now the windows have been realized we switch off the cursor to prevent it
    // distracting the people seeing the stereo images.
    float fovy = 1.0f;
    for( unsigned int i = 0; i < viewer.getCameraConfig()->getNumberOfCameras(); i++ )
    {
        Producer::Camera* cam = viewer.getCameraConfig()->getCamera(i);
        Producer::RenderSurface* rs = cam->getRenderSurface();
        rs->useCursor(false);
        fovy = cam->getLensVerticalFov();
    }

    float radius = 1.0f;
    float height = 2*radius*tan(fovy*0.5f);
    float length = osg::PI*radius;  // half a cylinder.

    // creat the scene from the file list.
    osg::ref_ptr<osg::Switch> rootNode = createScene(fileList,height,length);
    
    if (!rootNode) return 0;


    //osgDB::writeNodeFile(*rootNode,"test.osg");

    // set the scene to render
    viewer.setSceneData(rootNode.get());


    // set up the SlideEventHandler.
    seh->set(rootNode.get(),timeDelayBetweenSlides,autoSteppingActive);
    

    // create the windows and run the threads.
    viewer.realize();
    
    osg::Matrix homePosition;
    homePosition.makeLookAt(osg::Vec3(0.0f,0.0f,0.0f),osg::Vec3(0.0f,1.0f,0.0f),osg::Vec3(0.0f,0.0f,1.0f));
        
    while( !viewer.done() )
    {
        // wait for all cull and draw threads to complete.
        viewer.sync();

        // update the scene by traversing it with the the update visitor which will
        // call all node update callbacks and animations.
        viewer.update();
         
        //viewer.setView(homePosition);

        // fire off the cull and draw traversals of the scene.
        viewer.frame();
        
    }
    
    // wait for all cull and draw threads to complete before exit.
    viewer.sync();
    
    return 0;
}

