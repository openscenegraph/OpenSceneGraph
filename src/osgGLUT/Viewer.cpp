#if defined(_MSC_VER)
    #pragma warning( disable : 4786 )
#endif

#include <osg/GL>
#include <osgGLUT/glut>

#include <stdlib.h>
#if (!defined(WIN32) && !defined(macintosh)) || defined(__CYGWIN__)
#include <unistd.h>
#include <sys/time.h>
#endif
#include <stdio.h>

#include <osg/Math>
#include <osg/Statistics>

#include <string>

#include <osgGLUT/Viewer>
#include <osgGLUT/GLUTEventAdapter>

#include <osg/ApplicationUsage>
#include <osg/Switch>
#include <osg/Billboard>
#include <osg/LOD>
#include <osg/Light>
#include <osg/LightSource>
#include <osg/Geode>
#include <osg/Group>
#include <osg/NodeVisitor>
#include <osg/LineSegment>
#include <osg/PolygonMode>
#include <osg/Texture2D>
#include <osg/LightModel>
#include <osg/ShadeModel>
#include <osg/Notify>
#include <osg/CollectOccludersVisitor>

#include <osg/Fog>

#include <osgDB/WriteFile>
#include <osgDB/Registry>

#include <osgUtil/IntersectVisitor>
#include <osgUtil/DisplayListVisitor>
#include <osgUtil/SmoothingVisitor>
#include <osgUtil/TriStripVisitor>
#include <osgUtil/DisplayRequirementsVisitor>

#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>

#include <osg/Version>
#include <osgUtil/Version>

#ifdef WIN32
#define USE_FLTK
#define USE_GLUT
#endif

/*
#if defined(WIN32) && !defined(__CYGWIN__)
#include <sys/timeb.h>
#else
#endif
*/

#include <osg/Timer>
osg::Timer g_timer;
osg::Timer_t g_initTime;

//static GLenum polymodes [] = { GL_FILL, GL_LINE, GL_POINT };
static osg::PolygonMode::Mode polymodes [] = { osg::PolygonMode::FILL, osg::PolygonMode::LINE, osg::PolygonMode::POINT };

// forward declare functions to be used in stats.
GLuint makeRasterFont(void);
void displaytext(int x, int y, char *s);
int writePrims( const int ypos, osg::Statistics& stats);

using namespace osg;
using namespace osgUtil;
using namespace osgGLUT;
using namespace osgGA;

using namespace std;


Viewer* Viewer::s_theViewer = 0;

Viewer::Viewer()
{
    initialize();
}

Viewer::Viewer(osg::ArgumentParser& arguments)
{
    initialize();

    glutInit( &arguments.argc(), arguments.argv() );

    setWindowTitle(arguments.getProgramName().c_str());
    readCommandLine(arguments);
}

void Viewer::initialize()

{
    s_theViewer = this;

    polymode = 0;
    texture = 1;
    backface = 1;
    lighting = 1;
    flat_shade = 0;

    _printStats = 0; // gwm change from bool was : false;

    #ifdef SGV_USE_RTFS
    fs = new RTfs( RTFS_MODE_RTC_SPIN );
    frame_rate = 16;
    fs->setUpdateRate( frame_rate );
    #endif

    _viewFrustumCullingActive = true;
    _smallFeatureCullingActive = true;

    _useDisplayLists = true;

    _saveFileName = "saved_model.osg";

    osg::notify(osg::INFO)<<"Scene Graph Viewer (sgv)"<< std::endl;

//    osg::notify(osg::INFO)<<"   '"<<osgGetLibraryName()<<"' Version "<<osgGetVersion()<< std::endl;
//    osg::notify(osg::INFO)<<"   '"<<osgUtilGetLibraryName()<<"' Version "<<osgUtilGetVersion()<< std::endl;

    _initialTick = _timer.tick();
    _frameTick = _initialTick;
    _lastFrameTick = _initialTick;
    frRate=0;

    _focusedViewport = 0;         // The viewport with mouse/keyboard focus
    
    
    _frameStamp = new osg::FrameStamp;
    
    _displaySettings = new osg::DisplaySettings;

    _recordingAnimationPath = false;

}


Viewer::~Viewer()
{
}

void Viewer::clear()
{
    _viewportList.clear();
    _frameStamp = 0L;
    _displaySettings = 0L;
    
    Window::clear();
}

void Viewer::readCommandLine(osg::ArgumentParser& arguments)
{
    // report the usage options.
    if (arguments.getApplicationUsage())
    {
        arguments.getApplicationUsage()->addCommandLineOption("-f","open the application in fullscreen mode");
    }

    while(arguments.read("-f"))
    {
        _fullscreen = true;
    }
    
    _displaySettings->readCommandLine(arguments);

    osgDB::readCommandLine(arguments);

}


void Viewer::toggleFullScreen()
{
    _fullscreen = !_fullscreen;
    if (_fullscreen)
    {
        _saved_ww = _ww;
        _saved_wh = _wh;
        glutFullScreen();
    } else
    {
        //glutPositionWindow(wx,wy);
        glutReshapeWindow(_saved_ww,_saved_wh);
    }
}

/**
  * Configure and open the GLUT window for this Viewer
  * 
  */
bool Viewer::open()
{
    if ( getNumViewports() <= 0 ) {
        osg::notify(osg::FATAL)<<"osgGLUT::Viewer::open() called with no Viewports registered."<< std::endl;
        return false;
    }

    // Verify all viewports have an active camera manipulator 
    unsigned int index = 0;
    ViewportList::iterator itr;
    for(itr=_viewportList.begin();
        itr!=_viewportList.end();
        ++itr, ++index)
    {
        if (itr->_cameraManipList.empty())
        {
            osg::notify(osg::INFO)<<"osgGLUT::Viewer::open() called without any camara manipulators registered for a viewport,"<< std::endl;
            osg::notify(osg::INFO)<<"automatically registering trackball,flight and drive manipulators."<< std::endl;
            registerCameraManipulator(new osgGA::TrackballManipulator, index);
            registerCameraManipulator(new osgGA::FlightManipulator, index);
            registerCameraManipulator(new osgGA::DriveManipulator, index);
        }

        if (!itr->_cameraManipulator.valid())
            selectCameraManipulator(0,index);
    }

    GLUTEventAdapter::setWindowSize( _wx, _wy, _wx+_ww, _wx+_wh );
    GLUTEventAdapter::setButtonMask(0);


    bool needQuadBufferStereo = false;

    // do we need quad buffer stereo?

    // Set the absolute viewport for each SceneView based on the
    //   relative viewport coordinates given to us
    for(itr=_viewportList.begin();
        itr!=_viewportList.end();
        ++itr)
    {
        osgUtil::SceneView* sceneView = itr->sceneView.get();
        int view[4] = { int(itr->viewport[0]*_ww), int(itr->viewport[1]*_wh),
                        int(itr->viewport[2]*_ww), int(itr->viewport[3]*_wh) };

        sceneView->setViewport(view[0], view[1], view[2], view[3]);

        osg::ref_ptr<GLUTEventAdapter> ea = new GLUTEventAdapter;
        ea->adaptResize(clockSeconds(), 
                        view[0], view[1], 
                        view[0]+view[2], view[1]+view[3]);

        if (itr->_cameraManipulator->handle(*ea,*this))
        {
            //        osg::notify(osg::INFO) << "Handled reshape "<< std::endl;
        }

        const osg::DisplaySettings* vs = sceneView->getDisplaySettings();
        if (vs)
        {
            _displaySettings->merge(*vs);
        }
        else
        {
            // one does not already exist so attach the viewers visualsSettins to the SceneView
            sceneView->setDisplaySettings(_displaySettings.get());
        }

    }
    
    if (_displaySettings->getStereo() && 
        _displaySettings->getStereoMode()==osg::DisplaySettings::QUAD_BUFFER) needQuadBufferStereo = true;

    // traverse the scene graphs gathering the requirements of the OpenGL buffers.
    osgUtil::DisplayRequirementsVisitor drv;
    drv.setDisplaySettings(_displaySettings.get());
    for(itr=_viewportList.begin();
        itr!=_viewportList.end();
        ++itr)
    {
        Node* node = itr->sceneView->getSceneData();
        if (node) node->accept(drv);
    }

    // set up each render stage to clear the appropriate buffers.
    GLbitfield clear_mask=0;
    if (_displaySettings->getRGB())              clear_mask |= GL_COLOR_BUFFER_BIT;
    if (_displaySettings->getDepthBuffer())      clear_mask |= GL_DEPTH_BUFFER_BIT;
    if (_displaySettings->getStencilBuffer())    clear_mask |= GL_STENCIL_BUFFER_BIT;

    for(itr=_viewportList.begin();
        itr!=_viewportList.end();
        ++itr)
    {
        osgUtil::RenderStage *stage = itr->sceneView->getRenderStage();
        if (stage) stage->setClearMask(clear_mask);
    }


    // set the GLUT display mode bit mask up to handle it.
    unsigned int displayMode=0;
    if (_displaySettings->getDoubleBuffer())     displayMode |= GLUT_DOUBLE;
    else                                         displayMode |= GLUT_SINGLE;
    if (_displaySettings->getRGB())              displayMode |= GLUT_RGB;
    if (_displaySettings->getDepthBuffer())      displayMode |= GLUT_DEPTH;
    if (_displaySettings->getAlphaBuffer())      displayMode |= GLUT_ALPHA;
    if (_displaySettings->getStencilBuffer())    displayMode |= GLUT_STENCIL;
    if (needQuadBufferStereo)                    displayMode |= GLUT_STEREO;

    // and we'll add in multisample so that on systems like Onyx's can
    // go ahead and use there loverly anti-aliasing.  This is ignored
    // by other systems I've come across so not need to worry about it.
    displayMode |= GLUT_MULTISAMPLE;
    
    osg::notify(osg::INFO)                                    <<"osgGLUT::Viewer::open() requesting displayMode = "<<displayMode<< std::endl;
    if (displayMode & GLUT_DOUBLE)      osg::notify(osg::INFO)<<"                        requesting GLUT_DOUBLE."<< std::endl;
    if (displayMode & GLUT_SINGLE)      osg::notify(osg::INFO)<<"                        requesting GLUT_SINGLE."<< std::endl; 
    if (displayMode & GLUT_RGB)         osg::notify(osg::INFO)<<"                        requesting GLUT_RGB."<< std::endl; 
    if (displayMode & GLUT_DEPTH)       osg::notify(osg::INFO)<<"                        requesting GLUT_DEPTH."<< std::endl; 
    if (displayMode & GLUT_ALPHA)       osg::notify(osg::INFO)<<"                        requesting GLUT_ALPHA."<< std::endl; 
    if (displayMode & GLUT_STENCIL)     osg::notify(osg::INFO)<<"                        requesting GLUT_STENCIL."<< std::endl; 
    if (displayMode & GLUT_MULTISAMPLE) osg::notify(osg::INFO)<<"                        requesting GLUT_MULTISAMPLE."<< std::endl; 
    if (displayMode & GLUT_STEREO)      osg::notify(osg::INFO)<<"                        requesting GLUT_STEREO."<< std::endl; 

    _displayMode = displayMode;

    return Window::open();
}



unsigned int Viewer::registerCameraManipulator(osgGA::CameraManipulator* cm,
                                               unsigned int viewport)
{
    ViewportDef &viewp = _viewportList[viewport];
    unsigned int pos   = viewp._cameraManipList.size();
    viewp._cameraManipList.push_back(cm);
    return pos;
}

void Viewer::prependEventHandler(osgGA::GUIEventHandler* handler,unsigned int viewport)
{
    ViewportDef &viewp = _viewportList[viewport];
    viewp._eventHandlerList.push_front( handler );
}

void Viewer::appendEventHandler(osgGA::GUIEventHandler* handler,unsigned int viewport)
{
    ViewportDef &viewp = _viewportList[viewport];
    viewp._eventHandlerList.push_back( handler );
}

void Viewer::setFocusedViewport(unsigned int pos)
{
    if (pos>=_viewportList.size()) return;

    _focusedViewport = pos;
}


void Viewer::selectCameraManipulator(unsigned int pos, unsigned int viewport)
{
    if (viewport>=_viewportList.size()) return;

    ViewportDef &viewp = _viewportList[viewport];
    if (pos>=viewp._cameraManipList.size()) return;

    viewp._cameraManipulator = viewp._cameraManipList[pos];

    osgUtil::SceneView *sceneView = viewp.sceneView.get();
    viewp._cameraManipulator->setCamera(sceneView->getCamera());
    viewp._cameraManipulator->setNode(sceneView->getSceneData());

    osg::ref_ptr<GLUTEventAdapter> ea = new GLUTEventAdapter;
    viewp._cameraManipulator->init(*ea,*this);
}


void Viewer::requestWarpPointer(int x,int y)
{
    // glutWarpPointer core dumps if invoked before a GLUT window is open
    if ( !_is_open ) {
        osg::notify(osg::INFO)<<"osgGLUT::Viewer::requestWarpPointer() called with window closed; ignored."<< std::endl;
        return;
    }
    glutWarpPointer(x,y);
}


float Viewer::update(unsigned int viewport)
{
    osg::Timer_t beforeApp = _timer.tick();


    // update the camera manipulator.
    osg::ref_ptr<GLUTEventAdapter> ea = new GLUTEventAdapter;
    ea->adaptFrame(_frameStamp->getReferenceTime());

    for (EventHandlerList::iterator eh = _viewportList[viewport]._eventHandlerList.begin();
         eh != _viewportList[viewport]._eventHandlerList.end(); 
         eh++ )
    {
        if (eh->valid()) (*eh)->handle(*ea,*this);
    }
    _viewportList[viewport]._cameraManipulator->handle(*ea,*this);


    if (getRecordingAnimationPath() && getAnimationPath())
    {
        osg::Camera* camera = getViewportSceneView(viewport)->getCamera();
        osg::Matrix matrix;
        matrix.invert(camera->getModelViewMatrix());
        osg::Quat quat;
        quat.set(matrix);
        getAnimationPath()->insert(_frameStamp->getReferenceTime(),osg::AnimationPath::ControlPoint(matrix.getTrans(),quat));
    }

    // do app traversal.
    
    getViewportSceneView(viewport)->setFrameStamp(_frameStamp.get());
    getViewportSceneView(viewport)->update();

    osg::Timer_t beforeCull = _timer.tick();

    return  _timer.delta_m(beforeApp,beforeCull);
}


float Viewer::cull(unsigned int viewport)
{
    osg::Timer_t beforeCull = _timer.tick();

    // do cull traversal.
    getViewportSceneView(viewport)->cull();

    osg::Timer_t beforeDraw = _timer.tick();

    return _timer.delta_m(beforeCull,beforeDraw);
}


float Viewer::draw(unsigned int viewport)
{
    osg::Timer_t beforeDraw = _timer.tick();

    // do draw traversal.
    getViewportSceneView(viewport)->draw();

    osg::Timer_t afterDraw = _timer.tick();

    return _timer.delta_m(beforeDraw,afterDraw);
}


int Viewer::mapWindowXYToViewport(int x, int y)
{
    int ogl_y = _wh-y;

    int index = 0;
    for(ViewportList::iterator itr=_viewportList.begin();
        itr!=_viewportList.end();
        ++itr, ++index)
    {
      if ( x >= int( itr->viewport[0]*_ww ) &&
           ogl_y  >= int( itr->viewport[1]*_wh ) &&
           x <= int( (itr->viewport[0]+itr->viewport[2])*_ww ) &&
           ogl_y  <= int( (itr->viewport[1]+itr->viewport[3])*_wh ) )
        return index;
    }
    return -1;
}

// GWM July 2001 - moved all draw stats to  Statistics structure, and related RenderBin
// GWM Sept 2001 - all draw stats now calculated by calls to <Drawable>->getStats(Statistic..)

void Viewer::showStats(const unsigned int /*viewport*/)
{ // collect stats for viewport
    static int maxbins=1; // count number of bins
    static GLfloat tmax=100;
    glViewport(0,0,_ww,_wh);
    float vh = _wh;

    glDisable( GL_DEPTH_TEST ); // to see the stats always
    glDisable( GL_ALPHA_TEST );
    glDisable( GL_LIGHTING );
    glDisable( GL_TEXTURE_2D );
    
    glMatrixMode( GL_PROJECTION );
    glPushMatrix();
    glLoadIdentity();
    // set tmax using hysteresis to prevent flip-flopping between two values of tmax
    // tmax is the scale for the stage timing graph
    if (times[2].timeFrame>360.0f && tmax<1600) tmax=1600;
    else if (times[2].timeFrame<300.0f && tmax>800) tmax=800;
    else if (times[2].timeFrame>180.0f && tmax<800) tmax=800;
    else if (times[2].timeFrame<150.0f && tmax>400) tmax=400;
    else if (times[2].timeFrame>90.0f && tmax<400) tmax=400;
    else if (times[2].timeFrame<75.0f && tmax>200) tmax=200;
    else if (times[2].timeFrame>45.0f && tmax<200) tmax=200;
    else if (times[2].timeFrame<36.0f && tmax>100) tmax=100;
    glOrtho(-0.1f*tmax, tmax*1.1f,0,vh,0,500);
    glDepthRange(0,1);
    
    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    glLoadIdentity();
    glDisable(GL_STENCIL_TEST); // dont want to count pixels set by performance graphs
    
    if (_printStats>0) { // output the text frame rate
        char clin[72]; // buffer to print
        glColor3f(1.0f,1.0f,0.0f);
        if (frRate>10.0f)
        {
            float smoothRatio = 0.4; // should be >0 and <= 1.0, 
                                     // lower the value greater smoothing.
            frRate=(1.0f-smoothRatio)*frRate+smoothRatio*frameRate(); // smooth out variations in frame rate
        }
        else
        {
            frRate=frameRate(); // frame rate so slow no need to smooth in frame rate
        }
        sprintf(clin,"%.1f Hz.", frRate);
        displaytext(0,(int)(0.98f*vh),clin);
    }
    
    if (_printStats>=Statistics::STAT_GRAPHS  && _printStats!=Statistics::STAT_PRIMSPERVIEW  && _printStats!=Statistics::STAT_PRIMSPERBIN) { // more stats - graphs this time
    
        int sampleIndex = 2;
        float timeUpdate=times[sampleIndex].timeUpdate;
        float timeCull=times[sampleIndex].timeCull;
        float timeDraw=times[sampleIndex].timeDraw;
        float timeFrame=times[sampleIndex].timeFrame;

        osg::Vec4 app_color(0.0f,1.0f,0.0f,1.0f);
        osg::Vec4 cull_color(1.0f,0.0f,1.0f,1.0f);
        osg::Vec4 draw_color(0.0f,1.0f,1.0f,1.0f);
        osg::Vec4 swap_color(1.0f,0.5f,0.5f,1.0f);
        osg::Vec4 frame_color(1.0f,1.0f,0.0f,1.0f);

        char clin[72]; // buffer to print
        glColor4fv((GLfloat * )&app_color);
        sprintf(clin,"App %.2f ms.", timeUpdate);
        displaytext((int)(.15f*tmax),(int)(0.98f*vh),clin);
 
        glColor4fv((GLfloat * )&cull_color);
        sprintf(clin,"Cull %.2f ms.", timeCull);
        displaytext((int)(.35*tmax),(int)(0.98f*vh),clin);

        glColor4fv((GLfloat * )&draw_color);
        sprintf(clin,"Draw %.2f ms.", timeDraw);
        displaytext((int)(.55*tmax),(int)(0.98f*vh),clin);

        glColor4fv((GLfloat * )&frame_color);
        sprintf(clin,"Frame %.2f ms.", timeFrame);
        displaytext((int)(.75*tmax),(int)(0.98f*vh),clin);

 /*       osg::notify(osg::NOTICE) << "Time of App  "<<timeUpdate<<"ms   "<< std::endl;
        osg::notify(osg::NOTICE) << "Time of Cull "<<timeCull<<"ms   "<< std::endl;
        osg::notify(osg::NOTICE) << "Time of Draw "<<timeDraw<<"ms   "<< std::endl;
        osg::notify(osg::NOTICE) << "Frame time   "<<frameTime<< std::endl;
        osg::notify(osg::NOTICE) << "frameRate() = "<<frameRate()<< std::endl;*/

        glLineWidth(2.0f);
        glBegin(GL_LINE_LOOP );
        // yellow frame/grid in front of stats, units of .01 sec(?)
        glColor3f(.6f,.6f,0.0f);
        glVertex2f(0.0,0.97f*vh);
        glVertex2f(tmax,0.97f*vh);
        glVertex2f(tmax,0.88f*vh);
        glVertex2f(0.0,0.88f*vh);
        glEnd();
        glBegin(GL_LINES);
        // time marks
        int i;
        for (i=10; i<tmax; i+=10) {
            if ((i%100) == 0) glColor3f(.6f,.0f,0.0f); // red mark every 0.1 sec
            else glColor3f(.6f,.6f,0.0f); // yellow every 0.01 sec
            glVertex2f(i,0.97f*vh);
            glVertex2f(i,0.88f*vh);
        }
        // plot time for app, cull, draw, frame...
        float tstart=0;
        for (i=0; i<3; i++) {
            glColor4fv((GLfloat * )&app_color);
            glVertex2f(tstart,0.95f*vh);
            glVertex2f(tstart+times[i].timeUpdate,0.95f*vh);
            glColor4fv((GLfloat * )&cull_color);
            glVertex2f(tstart+times[i].timeUpdate,0.93f*vh);
            glVertex2f(tstart+times[i].timeUpdate+times[i].timeCull, 0.93f*vh);
            glColor4fv((GLfloat * )&draw_color);
            glVertex2f(tstart+times[i].timeUpdate+times[i].timeCull, 0.91f*vh);
            glVertex2f(tstart+times[i].timeUpdate+times[i].timeCull+times[i].timeDraw, 0.91f*vh);
            glColor4fv((GLfloat * )&swap_color);
            glVertex2f(tstart+times[i].timeUpdate+times[i].timeCull+times[i].timeDraw, 0.90f*vh);
            glVertex2f(tstart+times[i].timeFrame, 0.90f*vh);
            tstart+=times[i].timeFrame;
        }
        glEnd();
        glLineWidth(1.0f);
    }
    if (_printStats==osg::Statistics::STAT_PRIMS) { // yet more stats - add triangles, number of strips...
    /* 
       * Use the new renderStage.  Required mods to RenderBin.cpp, and RenderStage.cpp (add getPrims)
       * also needed to define a new class called Statistic (see osgUtil/Statistic).
       * RO, July 2001.
        */
        
        ViewportList::iterator itr;
        Statistics primStats;
        for(itr=_viewportList.begin();
            itr!=_viewportList.end();
            ++itr)
        {
            osgUtil::RenderStage *stage = itr->sceneView->getRenderStage();
            stage->getPrims(&primStats);
        }
        primStats.setType(Statistics::STAT_PRIMS); // full print out required
        writePrims((int)(0.86f*vh),primStats);
        maxbins=(primStats.getBins()>maxbins)?primStats.getBins():maxbins;

    }
    if (_printStats==Statistics::STAT_PRIMSPERBIN) { // more stats - add triangles, number of strips... as seen per bin
    /* 
       * Use the new renderStage.  Required mods to RenderBin.cpp, and RenderStage.cpp (add getPrims)
       * also needed to define a new class called Statistic (see osgUtil/Statistic).
       * RO, July 2001.
        */
        
        std::vector<Statistics> primStats(maxbins); // array of bin stats
        ViewportList::iterator itr;
        for(itr=_viewportList.begin();
            itr!=_viewportList.end();
            ++itr)
        {
            osgUtil::RenderStage *stage = itr->sceneView->getRenderStage();
            stage->getPrims(&primStats.front(), maxbins);
        }
        int nbinsUsed=(primStats[0].getBins()<maxbins)?primStats[0].getBins():maxbins;
        int ntop=0; // offset
        for (int i=0; i<nbinsUsed; i++) {
            primStats[i].setType(Statistics::STAT_PRIMSPERBIN); // cuts out vertices & triangles to save space on screen
            ntop+=writePrims((int)(0.96f*vh-ntop),primStats[i]);
            //osg::notify(osg::INFO) << "ntop "<< ntop<< std::endl;
        }
        maxbins=(primStats[0].getBins()>maxbins)?primStats[0].getBins():maxbins;
    }
    if (_printStats==Statistics::STAT_DC) { // yet more stats - read the depth complexity
        int wid=_ww, ht=_wh; // temporary local screen size - must change during this section
        if (wid>0 && ht>0) {
            const int blsize=16;
            char *clin=new char[wid/blsize+2]; // buffer to print dc
            char ctext[128]; // buffer to print details
            float mdc=0;
            GLubyte *buffer=new GLubyte[wid*ht];
            if (buffer) {
                glPixelStorei(GL_PACK_ALIGNMENT, 1); // no extra bytes at ends of rows- easier to analyse
                glColor3f(.9f,.9f,0.0f);
                glReadPixels(0,0,wid,ht, GL_STENCIL_INDEX ,GL_UNSIGNED_BYTE, buffer);
                for (int j=0; j<ht; j+=blsize) { // break up screen into blsize*blsize pixel blocks
                    char *clpt=clin; // moves across the clin to display lines of text
                    for (int i=0; i<wid; i+=blsize) { // horizontal pixel blocks
                        int dc=0;
                        int nav=0; // number of pixels averaged for DC calc
                        for (int jj=j; jj<j+blsize; jj++) {
                            for (int ii=i; ii<i+blsize; ii++) {
                                if (jj<ht && ii<wid && jj>=0 && ii>=0) {
                                    dc+=buffer[ii+ (ht-jj-1)*wid];
                                    nav++;
                                }
                            }
                        }
                        mdc+=dc;
                        if (dc<nav) *clpt= ' '+(10*dc)/nav; // fine detail in dc=[0,1]; 0.1 increment in display, space for empty areas
                        else if (dc<80*nav) *clpt= '0'+dc/nav; // show 1-9 for DC=1-9; then ascii to 127
                        else *clpt= '+'; // too large a DC - use + to show over limit
                        clpt++;
                    }
                    *clpt='\0';
                    displaytext(0,(int)(0.84f*vh-(j*12)/blsize),clin); // display average DC over the blsize box
                }
                sprintf(ctext, "Pixels hit %.1f Mean DC %.2f: %4d by %4d pixels.", mdc, mdc/(wid*ht), wid, ht);
                displaytext(0,(int)(0.86f*vh),ctext);
                
                glEnable(GL_STENCIL_TEST); // re-enable stencil buffer counting
                delete [] buffer;
            }
            delete [] clin;
        }
    }

    glMatrixMode( GL_MODELVIEW );
    glPopMatrix();

    glMatrixMode( GL_PROJECTION );
    glPopMatrix();
}


void Viewer::display()
{

    _frameStamp->setFrameNumber(_frameStamp->getFrameNumber()+1);
    _frameStamp->setReferenceTime(clockSeconds());

    // application traverasal.
    times[2].timeUpdate=0.0f;

    // cull traverasal.
    times[2].timeCull=0.0f;

    // draw traverasal.
    times[2].timeDraw=0.0f;
    
    for(unsigned int i = 0; i < getNumViewports(); i++ )
    {
        // update traverasal.
        times[2].timeUpdate+=update(i);


        // cull traverasal.
        times[2].timeCull+=cull(i);

        if (_printStats>=1) glFinish();

        // draw traverasal.
        times[2].timeDraw+=draw(i);
        if (_printStats==Statistics::STAT_PRIMSPERVIEW)
        { // gwm - get and show stats in each window
            showStats(i);
        }

    }


    if (_printStats>1) glFinish();

    times[2].frameend=updateFrameTick(); // absolute time

    times[2].timeFrame=frameSeconds()*1000;

    if (_printStats) {
        showStats(0); // output selected stats at this point - timing graph.
        times[0]=times[1]; // move the times buffers down
        times[1]=times[2];
    }

    glutSwapBuffers(); // moved after draw of stats & glFinish() to get accurate timing (excluding stat draw!)
    //    cout << "Time elapsed "<<_timer.delta_s(_initialTick,_timer.tick())<< std::endl;

    if (_printStats>1) glFinish();

    #ifdef SGV_USE_RTFS
    fs->frame();
    #endif
}


void Viewer::reshape(GLint w, GLint h)
{
    Window::reshape(w,h);

    // Propagate new window size to viewports
    for(ViewportList::iterator itr=_viewportList.begin();
        itr!=_viewportList.end();
        ++itr)
    {
        osgUtil::SceneView* sceneView = itr->sceneView.get();
        int view[4] = { int(itr->viewport[0]*_ww), int(itr->viewport[1]*_wh),
                        int(itr->viewport[2]*_ww), int(itr->viewport[3]*_wh) };

        sceneView->setViewport(view[0], view[1], view[2], view[3]);

        osg::ref_ptr<GLUTEventAdapter> ea = new GLUTEventAdapter;
        ea->adaptResize(clockSeconds(), 
                        view[0], view[1], 
                        view[0]+view[2], view[1]+view[3]);

        if (itr->_cameraManipulator->handle(*ea,*this))
        {
            //        osg::notify(osg::INFO) << "Handled reshape "<< std::endl;
        }
        for ( EventHandlerList::iterator eh = itr->_eventHandlerList.begin();
                eh != itr->_eventHandlerList.end(); eh++ ) {
            if ( eh->valid() ) {
                if ( (*eh)->handle(*ea,*this) ) {
                    // event handler handle this call.
                }
            }
        }
    
    }
}


void Viewer::mouseMotion(int x, int y)
{
    osg::ref_ptr<GLUTEventAdapter> ea = new GLUTEventAdapter;
    ea->adaptMouseMotion(clockSeconds(),x,y);

    bool handled = false;
    for ( EventHandlerList::iterator eh =
            _viewportList[_focusedViewport]._eventHandlerList.begin();
            eh != _viewportList[_focusedViewport]._eventHandlerList.end();
            eh++ ) {
        if ( eh->valid() ) {
            if ( (*eh)->handle(*ea,*this) ) {
                handled = true;
                break;
            }
        }
    }
    if ( !handled ) {
        if ( _viewportList[_focusedViewport]._cameraManipulator->handle(
                    *ea,*this) ) {
            //        osg::notify(osg::INFO) << "Handled mouseMotion "<<ea->_buttonMask<<" x="<<ea->_mx<<" y="<<ea->_my<< std::endl;
        }
    }

    _mx = x;
    _my = y;

}


void Viewer::mousePassiveMotion(int x, int y)
{
    osg::ref_ptr<GLUTEventAdapter> ea = new GLUTEventAdapter;
    ea->adaptMousePassiveMotion(clockSeconds(),x,y);

    // Switch viewport focus if no buttons are pressed
    if (ea->getButtonMask() == 0)
    {
        int focus = mapWindowXYToViewport(x,y);
        if (focus >= 0 && focus != int(_focusedViewport))
          setFocusedViewport(focus);
    }

    bool handled = false;
    for ( EventHandlerList::iterator eh =
            _viewportList[_focusedViewport]._eventHandlerList.begin();
            eh != _viewportList[_focusedViewport]._eventHandlerList.end();
            eh++ ) {
        if ( eh->valid() ) {
            if ( (*eh)->handle(*ea,*this) ) {
                handled = true;
                break;
            }
        }
    }
    if ( !handled ) {
        if ( _viewportList[_focusedViewport]._cameraManipulator->handle(
                    *ea,*this) ) {
            //        osg::notify(osg::INFO) << "Handled mousePassiveMotion "<<ea->_buttonMask<<" x="<<ea->_mx<<" y="<<ea->_my<< std::endl;
        }
    }
}


void Viewer::mouse(int button, int state, int x, int y)
{
    osg::ref_ptr<GLUTEventAdapter> ea = new GLUTEventAdapter;
    ea->adaptMouse(clockSeconds(),button,state,x,y);

    // Switch viewport focus if button is pressed, and it is the only one
    unsigned mask = ea->getButtonMask();
    if (state == GLUT_DOWN && 
        (mask == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON   || 
         mask == osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON || 
         mask == osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON))
    {
        int focus = mapWindowXYToViewport(x,y);
        if (focus >= 0
                && _viewportList[(unsigned int)focus]._focusable
                && focus != int(_focusedViewport))
          setFocusedViewport(focus);
    }

    bool handled = false;
    for ( EventHandlerList::iterator eh =
            _viewportList[_focusedViewport]._eventHandlerList.begin();
            eh != _viewportList[_focusedViewport]._eventHandlerList.end();
            eh++ ) {
        if ( eh->valid() ) {
            if ( (*eh)->handle(*ea,*this) ) {
                handled = true;
                break;
            }
        }
    }
    if ( !handled ) {
        if ( _viewportList[_focusedViewport]._cameraManipulator->handle(
                    *ea,*this) ) {
        //        osg::notify(osg::INFO) << "Handled mouse "<<ea->_buttonMask<<" x="<<ea->_mx<<" y="<<ea->_my<< std::endl;
        }
    }

}



void Viewer::keyboard(int key, int x, int y, bool keydown )
{
    osg::ref_ptr<GLUTEventAdapter> ea = new GLUTEventAdapter;
    ea->adaptKeyboard(clockSeconds(),key,x,y,keydown);

    for ( EventHandlerList::iterator eh =
            _viewportList[_focusedViewport]._eventHandlerList.begin();
            eh != _viewportList[_focusedViewport]._eventHandlerList.end();
            eh++ ) {
        if ( eh->valid() ) {
            if ( (*eh)->handle(*ea,*this) ) {
                return;
            }
        }
    }
    if ( _viewportList[_focusedViewport]._cameraManipulator->handle(
                *ea,*this) ) {
        return;
    }

    // only keydown handled below
    if ( !keydown ) {
        return;
    }

    if (key>='1' && key<='4')
    {
        int pos = key-'1';
        selectCameraManipulator(pos,_focusedViewport);
    }

    osgUtil::SceneView* sceneView = getViewportSceneView(_focusedViewport);
    osg::StateSet* globalStateSet = sceneView->getGlobalStateSet();
    if (!globalStateSet)
    {
        globalStateSet = new osg::StateSet;
        sceneView->setGlobalStateSet(globalStateSet);
    }


    switch( key )
    {

        case '>' :
            {
            osg::DisplaySettings* ds = const_cast<osg::DisplaySettings*>(sceneView->getDisplaySettings());
            if (ds) ds->setEyeSeparation(ds->getEyeSeparation()*1.5f);
            }
            break;

        case '<' :
            {
            osg::DisplaySettings* ds = const_cast<osg::DisplaySettings*>(sceneView->getDisplaySettings());
            if (ds) ds->setEyeSeparation(ds->getEyeSeparation()/1.5f);
            }
            break;


        case '7' :
            sceneView->setBackgroundColor(osg::Vec4(0.0f,0.0f,0.0f,1.0f));
            break;

        case '8' :
            sceneView->setBackgroundColor(osg::Vec4(0.2f, 0.2f, 0.4f, 1.0f));
            break;

        case '9' :
            sceneView->setBackgroundColor(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
            break;

        case '/' :
            if (sceneView->getLODScale()>0.5) sceneView->setLODScale(sceneView->getLODScale()/1.5f);
            break;

        case '*' :
            if (sceneView->getLODScale()<30) sceneView->setLODScale(sceneView->getLODScale()*1.5f);
            break;

        case '+' :
        #ifdef SGV_USE_RTFS
            frame_rate <<= 1;
            fs->stop();
            fs->setUpdateRate( frame_rate );
            fs->start();
        #endif
            break;

        case '-' :
        #ifdef SGV_USE_RTFS
            frame_rate >>= 1;
            if( frame_rate < 1 ) frame_rate = 1;
            fs->stop();
            fs->setUpdateRate( frame_rate );
            fs->start();
        #endif
            break;

        case 'd' :
            _useDisplayLists = !_useDisplayLists;
            if (_useDisplayLists)
            {
                // traverse the scene graph setting up all osg::Drawable's so they will use
                // OpenGL display lists.
                osgUtil::DisplayListVisitor dlv(osgUtil::DisplayListVisitor::SWITCH_ON_DISPLAY_LISTS);
                sceneView->getSceneData()->accept(dlv);
                osg::notify(osg::NOTICE) << "Switched on use of OpenGL Display Lists."<< std::endl;
            }
            else
            {
                // traverse the scene graph setting up all osg::Drawable's so they will use
                // OpenGL display lists.
                osgUtil::DisplayListVisitor dlv(osgUtil::DisplayListVisitor::SWITCH_OFF_DISPLAY_LISTS);
                sceneView->getSceneData()->accept(dlv);
                osg::notify(osg::NOTICE) << "Switched off use of OpenGL Display Lists."<< std::endl;
            }
            break;

        case 'p' :
            if (_printStats==Statistics::STAT_DC) glDisable(GL_STENCIL_TEST); // switch off stencil counting
            _printStats++; //gwm jul 2001 range of possible outputs, 0-4 = !_printStats;
            if (_printStats>=Statistics::STAT_RESTART) _printStats=0;
            if (getNumViewports()<=1 && _printStats==Statistics::STAT_PRIMSPERVIEW) _printStats++; // no need for these stats as only one view
            if (_printStats==Statistics::STAT_DC)
            {
            
                 // count depth complexity by incrementing the stencil buffer every 
                // time a pixel is hit
                GLint nsten=0; // Number of stencil planes available
                glGetIntegerv(GL_STENCIL_BITS , &nsten);
                if (_displayMode & GLUT_STENCIL && nsten>0) {
                    glEnable(GL_STENCIL_TEST);
                    glStencilOp(GL_INCR ,GL_INCR ,GL_INCR);
                } else {// skip this option
                    _printStats++;
                }
            }
            break;

        case 's' :
        {
            flat_shade = 1  - flat_shade ;
            osg::StateSet* stateset = globalStateSet;
            osg::ShadeModel* shademodel = dynamic_cast<osg::ShadeModel*>(stateset->getAttribute(osg::StateAttribute::SHADEMODEL));
            if (!shademodel)
            {
                shademodel = new osg::ShadeModel;
                stateset->setAttribute(shademodel,osg::StateAttribute::OVERRIDE);
            }

            if( flat_shade )
                shademodel->setMode( osg::ShadeModel::FLAT );
            else
                shademodel->setMode( osg::ShadeModel::SMOOTH );
            
            break;
        }
        case 'S' :
        {
            osg::notify(osg::NOTICE) << "Smoothing scene..."<< std::endl;
            osgUtil::SmoothingVisitor sv;
            sceneView->getSceneData()->accept(sv);
            osg::notify(osg::NOTICE) << "Smoothed scene."<< std::endl;
        }

        break;

        case 'R' :
        {
            osg::notify(osg::NOTICE) << "Tri Striping scene..."<< std::endl;
            osgUtil::TriStripVisitor tsv;
            sceneView->getSceneData()->accept(tsv);
            osg::notify(osg::NOTICE) << "Tri Striping scene scene."<< std::endl;
        }

        break;

        case 'b' :

            backface = 1 - backface;
            if( backface )
                globalStateSet->setMode(GL_CULL_FACE,osg::StateAttribute::ON);
            else
                globalStateSet->setMode(GL_CULL_FACE,osg::StateAttribute::OVERRIDE|osg::StateAttribute::OFF);

            break;

        case 'l' :
            lighting = 1  - lighting ;
            if( lighting )
                globalStateSet->setMode(GL_LIGHTING,osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON);
            else
                globalStateSet->setMode(GL_LIGHTING,osg::StateAttribute::OVERRIDE|osg::StateAttribute::OFF);
            break;

        case 'L' :
        {
            osgUtil::SceneView::LightingMode lm= sceneView->getLightingMode();
            switch(lm)
            {
                case(osgUtil::SceneView::HEADLIGHT) : lm = osgUtil::SceneView::SKY_LIGHT; break;
                case(osgUtil::SceneView::SKY_LIGHT) : lm = osgUtil::SceneView::NO_SCENEVIEW_LIGHT; break;
                case(osgUtil::SceneView::NO_SCENEVIEW_LIGHT) : lm = osgUtil::SceneView::HEADLIGHT; break;
            }
            sceneView->setLightingMode(lm);
            break;
        }
        case 't' :
            texture = 1 - texture;
            if (texture)
            {
                globalStateSet->setTextureModeToInherit(0,GL_TEXTURE_2D);
//                globalStateSet->setAttributeToInherit(osg::StateAttribute::TEXTURE);
            }
            else
            {
                // use blank texture to override all local texture in scene graph.
                // thus causing them to all use the same texture attribute, hence
                // preventing a state attribute change due to unused textures.
                globalStateSet->setTextureMode(0,GL_TEXTURE_2D,osg::StateAttribute::OVERRIDE|osg::StateAttribute::OFF);
//                static osg::ref_ptr<osg::Texture> blank_texture = new osg::Texture2D;
//                globalStateSet->setTextureAttribute(0,blank_texture.get());
            }
            break;

        case 'T' :
            {
                osg::LightModel* lightmodel = dynamic_cast<LightModel*>(globalStateSet->getAttribute(osg::StateAttribute::LIGHTMODEL));
                if (lightmodel)
                {
                    lightmodel = new osg::LightModel;
                    globalStateSet->setAttribute(lightmodel);
                }
                lightmodel->setTwoSided(!lightmodel->getTwoSided());
            }
            break;

        case 'w' :
            {
                polymode = (polymode+1)%3;
                osg::PolygonMode* polyModeObj = new osg::PolygonMode;
                polyModeObj->setMode(osg::PolygonMode::FRONT_AND_BACK,polymodes[polymode]);
                globalStateSet->setAttribute(polyModeObj);
            }
            break;

        case 'f' :
            toggleFullScreen();
            break;

        case 'o' :
            if (sceneView->getSceneData() && osgDB::writeNodeFile(*sceneView->getSceneData(), _saveFileName))
            {
                osg::notify(osg::NOTICE) << "Saved scene to '"<<_saveFileName<<"'"<< std::endl;
            }
            break;

        case 'c' :
            _smallFeatureCullingActive = !_smallFeatureCullingActive;
            sceneView->setCullingMode((osgUtil::CullVisitor::CullingMode)
                ((_smallFeatureCullingActive ? osgUtil::CullVisitor::SMALL_FEATURE_CULLING : osgUtil::CullVisitor::NO_CULLING) |
                (_viewFrustumCullingActive ? osgUtil::CullVisitor::VIEW_FRUSTUM_CULLING : osgUtil::CullVisitor::NO_CULLING)));
            if (_smallFeatureCullingActive)
            {
                osg::notify(osg::NOTICE) << "Small feature culling switched on   "<< std::endl;
            }
            else
            {
                osg::notify(osg::NOTICE) << "Small feature culling switched off  "<< std::endl;
            }
            break;

        case 'C' :
            _viewFrustumCullingActive = !_viewFrustumCullingActive;
            if (_viewFrustumCullingActive)
            {
                osg::notify(osg::NOTICE) << "View frustum culling switched on   "<< std::endl;
            }
            else
            {
                osg::notify(osg::NOTICE) << "View frustum culling switched off  "<< std::endl;
            }
            sceneView->setCullingMode((osgUtil::CullVisitor::CullingMode)
                ((_smallFeatureCullingActive ? osgUtil::CullVisitor::SMALL_FEATURE_CULLING : osgUtil::CullVisitor::NO_CULLING) |
                (_viewFrustumCullingActive ? osgUtil::CullVisitor::VIEW_FRUSTUM_CULLING : osgUtil::CullVisitor::NO_CULLING)));
            break;

        case 'P' :
            sceneView->setPrioritizeTextures(!sceneView->getPrioritizeTextures());
            if (sceneView->getPrioritizeTextures())
            {
                osg::notify(osg::NOTICE) << "Prioritize textures switched on   "<< std::endl;
            }
            else
            {
                osg::notify(osg::NOTICE) << "Prioritize textures switched off  "<< std::endl;
            }
            break;

        case '?' :
        case 'h' :
            help(osg::notify(osg::NOTICE));
            break;


        case 'O' :
            {
                osg::ref_ptr<osg::Viewport> viewport = new osg::Viewport;
                viewport->setViewport(_wx,_wy,_ww,_wh);
                std::string filename("screenshot.rgb");

                glReadBuffer(GL_FRONT);
                osg::ref_ptr<Image> image = new osg::Image;
                image->readPixels(viewport->x(),viewport->y(),viewport->width(),viewport->height(),
                                  GL_RGB,GL_UNSIGNED_BYTE);

                osgDB::writeImageFile(*image,filename);

                osg::notify(osg::NOTICE) << "Saved screen image to `"<<filename<<"`"<< std::endl;
            }
            break;
        case 'v' :
            {
                osg::notify(osg::NOTICE) << "Creating occluders"<< std::endl;

                osg::CollectOccludersVisitor cov;
                cov.setCreateDrawablesOnOccludeNodes(true);

                cov.pushViewport(sceneView->getViewport());
                
                if (sceneView->getProjectionMatrix()) cov.pushProjectionMatrix(sceneView->getProjectionMatrix());
                else if (sceneView->getCamera()) cov.pushProjectionMatrix(new RefMatrix(sceneView->getCamera()->getProjectionMatrix()));
                else cov.pushProjectionMatrix(new RefMatrix());
                
                if (sceneView->getModelViewMatrix()) cov.pushModelViewMatrix(sceneView->getModelViewMatrix());
                else if (sceneView->getCamera()) cov.pushModelViewMatrix(new RefMatrix(sceneView->getCamera()->getModelViewMatrix()));
                else cov.pushModelViewMatrix(new RefMatrix());
                
                sceneView->getSceneData()->accept(cov);

                cov.popModelViewMatrix();
                cov.popProjectionMatrix();
                cov.popViewport();


            }
            break;

        case 'i' :
        case 'r' :
            {
                osg::notify(osg::NOTICE) << "***** Intersecting **************"<< std::endl;

                osg::Vec3 near_point,far_point;
                if (!sceneView->projectWindowXYIntoObject(x,_wh-y,near_point,far_point))
                {
                    osg::notify(osg::NOTICE) << "Failed to calculate intersection ray."<< std::endl;
                    return;
                }

                osg::ref_ptr<osg::LineSegment> lineSegment = new osg::LineSegment;
                lineSegment->set(near_point,far_point);
                osg::notify(osg::NOTICE) << "start("<<lineSegment->start()<<")  end("<<lineSegment->end()<<")"<< std::endl;

                osgUtil::IntersectVisitor iv;
                iv.addLineSegment(lineSegment.get());

                float startTime = clockSeconds();

                sceneView->getSceneData()->accept(iv);

                float endTime = clockSeconds();

                osg::notify(osg::NOTICE) << "Time for interesection = "<<(endTime-startTime)*1000<<"ms"<< std::endl;

                if (iv.hits())
                {
                    osgUtil::IntersectVisitor::HitList& hitList = iv.getHitList(lineSegment.get());
                    for(osgUtil::IntersectVisitor::HitList::iterator hitr=hitList.begin();
                        hitr!=hitList.end();
                        ++hitr)
                    {
                        osg::Vec3 ip = hitr->getLocalIntersectPoint();
                        osg::Vec3 in = hitr->getLocalIntersectNormal();
                        osg::Geode* geode = hitr->_geode.get();
                        osg::notify(osg::NOTICE) << "  Itersection Point ("<<ip<<") Normal ("<<in<<")"<< std::endl;
                        if (hitr->_matrix.valid())
                        {
                            osg::Vec3 ipEye = hitr->getWorldIntersectPoint();
                            osg::Vec3 inEye = hitr->getWorldIntersectNormal();
                            inEye.normalize();
                            if (geode) osg::notify(osg::NOTICE) << "Geode '"<<geode->getName()<< std::endl;
                            osg::notify(osg::NOTICE) << "  Eye Itersection Point ("<<ipEye<<") Normal ("<<inEye<<")"<< std::endl;

                        }
                        if (key=='r' && geode)
                        {
                            // remove geoset..
                            osg::Drawable* drawable = hitr->_drawable.get();
                            osg::notify(osg::NOTICE) << "  drawable ("<<drawable<<") "<<geode->removeDrawable(drawable)<<")"<< std::endl;
                        }

                    }

                }

                osg::notify(osg::NOTICE) << std::endl << std::endl;
            }
            break;


        case 'y':
        case 'Y':
            {
                osg::Fog* globalFog = dynamic_cast<osg::Fog*>(globalStateSet->getAttribute(osg::StateAttribute::FOG));

                if(!globalFog)
                {
                    globalFog = new osg::Fog;
                    globalFog->setColor(osg::Vec4(0.9,0.9,0.9,1.0));
                    globalFog->setDensity(0.01f);
                    
                    globalStateSet->setAttributeAndModes(globalFog,osg::StateAttribute::ON);
                }

                //increase/reduce fog:
                if(globalFog)
                {
                    float density = globalFog->getDensity();

                    if (key=='y') density -= 0.001;
                    else density += 0.001;

                    if (density<0.0f) density=0.0f;

                    std::cout<<"setting fog density "<<density<<std::endl;                    
                    
                    globalFog->setDensity(density);
                }
            }
            break;

        case 'Z' :
        case 'z' :
        
            if (getRecordingAnimationPath())
            {
                // have already been recording so switch of recording.
                setRecordingAnimationPath(false);
                
                osg::notify(osg::NOTICE) << "Finished recording camera animation, press 'Z' to reply."<< std::endl;

                if (getAnimationPath())
                {
                    std::ofstream fout("saved_animation.path");
                    const AnimationPath::TimeControlPointMap& tcpm = getAnimationPath()->getTimeControlPointMap();
                    for(AnimationPath::TimeControlPointMap::const_iterator tcpmitr=tcpm.begin();
                        tcpmitr!=tcpm.end();
                        ++tcpmitr)
                    {
                        const AnimationPath::ControlPoint& cp = tcpmitr->second;
                        fout<<tcpmitr->first<<" "<<cp._position<<" "<<cp._rotation<<std::endl;
                    }
                    fout.close();
                    
                    osg::notify(osg::NOTICE) << "Saved camera animation to 'saved_animation.path'"<< std::endl;

                }
                
            }
            else if (key=='z') 
            {
                setRecordingAnimationPath(true);
                setAnimationPath(new osg::AnimationPath());
                getAnimationPath()->setLoopMode(osg::AnimationPath::LOOP);
                osg::notify(osg::NOTICE) << "Recording camera animation, press 'z' to finish recording."<< std::endl;
            }

            if (key=='Z')
            {
                osgGA::AnimationPathManipulator* apm = 0;
                int apmNo = 0;
                CameraManipList& cmlist = _viewportList[_focusedViewport]._cameraManipList;
                for(CameraManipList::iterator cmlitr=cmlist.begin();
                    cmlitr!=cmlist.end() && apm==0;
                    ++cmlitr,++apmNo)
                {
                    apm = dynamic_cast<osgGA::AnimationPathManipulator*>(cmlitr->get());
                }

                if (!apm)
                {
                    apm = new osgGA::AnimationPathManipulator();
                    apmNo = registerCameraManipulator(apm,_focusedViewport);
                }

                apm->setAnimationPath(getAnimationPath());
                apm->home(*ea,*this);

                selectCameraManipulator(apmNo,_focusedViewport);
            }

            break;

        case 27 :
            
            _exit = true;
            
// 
//             // Escape
//             #ifdef __MWERKS__
//             std::exit(0); // avoid collision of std::exit(..) / exit(..) compile errors.
//             #else
//             exit(0);
//             #endif
//             break;

    }
}


void Viewer::help(std::ostream& fout)
{

    fout << std::endl
        <<"Scene Graph Viewer (sgv) keyboard bindings:"<< std::endl
        << std::endl
        <<"1     Select the trackball camera manipulator."<< std::endl
        <<"      Left mouse button - rotate,"<< std::endl
        <<"      Middle (or Left & Right) mouse button - pan,"<< std::endl
        <<"      Right mouse button - zoom."<< std::endl
        <<"2     Select the flight camera manipulator."<< std::endl
        <<"      Left mouse button - speed up,"<< std::endl
        <<"      Middle (or Left & Right) mouse button - stop,"<< std::endl
        <<"      Right mouse button - slow down, reverse."<< std::endl
        <<"      Move mouse left to roll left, right to roll right."<< std::endl
        <<"      Move mouse back (down) to pitch nose up, forward to pitch down."<< std::endl
        <<"      In mode Q, the default, selected by pressing 'q'"<< std::endl
        <<"        The flight path is yawed automatically into the turn to"<< std::endl
        <<"        produce a similar effect as flying an aircaft."<< std::endl
        <<"      In mode A, selected by pressing 'a'"<< std::endl
        <<"        The flight path is not yawed automatically into the turn,"<< std::endl
        <<"        producing a similar effect as space/marine flight."<< std::endl
        <<"3     Select the drive camera manipulator."<< std::endl
        <<"      In mode Q, the default, selected by pressing 'q'"<< std::endl
        <<"        Move mouse left to turn left, right to turn right."<< std::endl
        <<"        Move mouse back (down) to reverse, forward to drive forward."<< std::endl
        <<"      In mode A, selected by pressing 'a'"<< std::endl
        <<"        Move mouse left to turn left, right to turn right."<< std::endl
        <<"        Left mouse button - speed up,"<< std::endl
        <<"        Middle (or Left & Right) mouse button - stop,"<< std::endl
        <<"        Right mouse button - slow down, reverse."<< std::endl
        << std::endl
        <<"+     Half the frame delay which speeds up the frame rate on Linux and Windows."<< std::endl
        <<"-     Double the frame delay and therefore reduce the frame rate on Linux"<< std::endl
        <<"      and Windows."<< std::endl
        << std::endl
        <<"/     Divide the Level-Of-Detail (LOD) bias by 1.5, to encourage the"<< std::endl
        <<"      selection of more complex LOD children."<< std::endl
        <<"*     Multiple the Level-of-Detail (LOD) bias by 1.5, to encourage the"<< std::endl
        <<"      selection of less complex LOD children."<< std::endl
        << std::endl
        <<"c     Toggle Small Feature Culling on or off."<< std::endl
        <<"C     Toggle View Frustum Culling on or off."<< std::endl
        <<"d     Toggle use of OpenGL's display lists."<< std::endl
        <<"b     Toggle OpenGL's backface culling."<< std::endl
        <<"t     Toggle OpenGL texturing on or off."<< std::endl
        <<"T     Toggle OpenGL two-sided lighting on or off."<< std::endl
        <<"l     Toggle OpenGL lighting on or off."<< std::endl
        <<"L     Toggle SceneView lighting mode between HEADLIGHT, SKY_LIGHT"<< std::endl
        <<"      and NO_SCENEVIEW_LIGHT."<< std::endl
        <<"s     Toggle OpenGL shade model between flat and smooth shading."<< std::endl
        <<"S     Apply osgUtil::SmoothingVisitor to the scene."<< std::endl
        <<"w     Toggle OpenGL polygon mode between solid, wireframe and points modes."<< std::endl
        << std::endl
        <<"i     Calculate and report the intersections with the scene under the"<< std::endl
        <<"      current mouse x and mouse y position."<< std::endl
        << std::endl
        <<"r     Calculate and report the intersections with the scene under the"<< std::endl
        <<"      current mouse x and mouse y position and delete the nearest"<< std::endl
        <<"      interesected geoset."<< std::endl
        << std::endl
        <<"7     Set the background color to black."<< std::endl
        <<"8     Set the background color to blue."<< std::endl
        <<"9     Set the background color to white."<< std::endl
        << std::endl
        <<"p     Print frame rate statistics on each frame."<< std::endl
        <<"o     Output the loaded scene to 'saved_model.osg'."<< std::endl
        <<"?/h   Print out sgv's keyboard bindings."<< std::endl
        <<"f     Toggle between fullscreen and the previous window size. Note, GLUT"<< std::endl
        <<"      fullscreen works properly on Windows and Irix, but on Linux"<< std::endl
        <<"      it just maximizes the window and leaves the window's borders."<< std::endl
        <<"      If started in full screen mode on Linux, window will be borderless"<< std::endl
        <<"      and will resize down, but will remain borderless."<< std::endl
        << std::endl
        <<"y     Reduce fog density."<< std::endl
        <<"Y     Increase fog density."<< std::endl
        << std::endl
        <<"z     Toggle recording of camera path."<< std::endl
        <<"Z     Reply recorded camera path."<< std::endl
        <<"Space Reset scene to the default view."<< std::endl
        <<"Esc   Exit sgv."<< std::endl;
}


osg::Timer_t Viewer::clockTick()
{
    return _timer.tick();
}


osg::Timer_t Viewer::frameTick()
{
    return _frameTick;
}


// update time from the current frame update and the previous one.
osg::Timer_t Viewer::updateFrameTick()
{
    _lastFrameTick = _frameTick;
    _frameTick = _timer.tick();
    return _frameTick;
}


bool Viewer::run()
{

    {
        // Reset the views of all of SceneViews
        osg::ref_ptr<GLUTEventAdapter> ea = new GLUTEventAdapter;

        for(ViewportList::iterator itr=_viewportList.begin();
            itr!=_viewportList.end();
            ++itr)
        {
            itr->_cameraManipulator->home(*ea,*this);
        }
    }
    
    updateFrameTick();
    return Window::run();
}


unsigned int Viewer::addViewport(osgUtil::SceneView* sv,
                         float x, float y, float width, float height) 
{ 
    ViewportDef def;
    def.sceneView   = sv;
    def.viewport[0] = x;
    def.viewport[1] = y;
    def.viewport[2] = width;
    def.viewport[3] = height;
    
    if (!_viewportList.empty())
    {
        // force all the viewports to share the same state object as
        // they all share the same graphics context.
        osg::State* state = _viewportList.front().sceneView->getState();
        def.sceneView->setState(state);
    }

    unsigned int pos = _viewportList.size();
    _viewportList.push_back(def);
    
    
    return pos;
}


/**
  * Adds a default SceneView referring to the passed scene graph root.
  */
unsigned int Viewer::addViewport(osg::Node* rootnode,
                         float x, float y, float width, float height) 
{
    osgUtil::SceneView *sceneView = new osgUtil::SceneView(_displaySettings.get());
    sceneView->setDefaults();
    sceneView->setSceneData(rootnode);

    return addViewport( sceneView, x, y, width, height );
}

void Viewer::init(osg::Node* rootnode)
{
    osg::notify(osg::WARN)<<"Warning - call to Viewer::init(osg::Node*) which is a deprecated method."<< std::endl;
    osg::notify(osg::WARN)<<"          This should be replaced with Viewer::addViewport(osg::Node*)."<< std::endl;
    osg::notify(osg::WARN)<<"          Automatically mapping init to addViewport."<< std::endl;
    addViewport(rootnode);
}



//////////////////////////////////////////////////////////////////////

GLuint makeRasterFont(void)
{ // GWM creates a set of display lists which may be used to render a character string on the screen
        // data from GWM's reading of the Windows ASCII_FIXED_FONT.
    GLubyte rasters[][12] = { // ascii symbols 32-127, small font
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        {0x00, 0x00, 0x08, 0x00, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x00},
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0x14, 0x14, 0x00},
        {0x00, 0x00, 0x28, 0x28, 0x7e, 0x14, 0x14, 0x14, 0x3f, 0x0a, 0x0a, 0x00},
        {0x00, 0x00, 0x08, 0x1c, 0x22, 0x02, 0x1c, 0x20, 0x22, 0x1c, 0x08, 0x00},
        {0x00, 0x00, 0x02, 0x45, 0x22, 0x10, 0x08, 0x04, 0x22, 0x51, 0x20, 0x00},
        {0x00, 0x00, 0x3b, 0x44, 0x4a, 0x49, 0x30, 0x10, 0x20, 0x20, 0x18, 0x00},
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x08, 0x08, 0x00},
        {0x04, 0x08, 0x08, 0x10, 0x10, 0x10, 0x10, 0x10, 0x08, 0x08, 0x04, 0x00},
        {0x10, 0x08, 0x08, 0x04, 0x04, 0x04, 0x04, 0x04, 0x08, 0x08, 0x10, 0x00},
        {0x00, 0x00, 0x00, 0x00, 0x36, 0x1c, 0x7f, 0x1c, 0x36, 0x00, 0x00, 0x00},
        {0x00, 0x00, 0x08, 0x08, 0x08, 0x7f, 0x08, 0x08, 0x08, 0x00, 0x00, 0x00},
        {0x00, 0x10, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        {0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        {0x00, 0x00, 0x00, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01, 0x00, 0x00},
        {0x00, 0x00, 0x1c, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x1c, 0x00},
        {0x00, 0x00, 0x3e, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x38, 0x08, 0x00},
        {0x00, 0x00, 0x3e, 0x20, 0x10, 0x08, 0x04, 0x02, 0x02, 0x22, 0x1c, 0x00},
        {0x00, 0x00, 0x1c, 0x22, 0x02, 0x02, 0x0c, 0x02, 0x02, 0x22, 0x1c, 0x00},
        {0x00, 0x00, 0x0e, 0x04, 0x3e, 0x24, 0x14, 0x14, 0x0c, 0x0c, 0x04, 0x00},
        {0x00, 0x00, 0x1c, 0x22, 0x02, 0x02, 0x3c, 0x20, 0x20, 0x20, 0x3e, 0x00},
        {0x00, 0x00, 0x1c, 0x22, 0x22, 0x22, 0x3c, 0x20, 0x20, 0x10, 0x0c, 0x00},
        {0x00, 0x00, 0x10, 0x10, 0x08, 0x08, 0x04, 0x04, 0x02, 0x22, 0x3e, 0x00},
        {0x00, 0x00, 0x1c, 0x22, 0x22, 0x22, 0x1c, 0x22, 0x22, 0x22, 0x1c, 0x00},
        {0x00, 0x00, 0x18, 0x04, 0x02, 0x02, 0x1e, 0x22, 0x22, 0x22, 0x1c, 0x00},
        {0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00},
        {0x00, 0x10, 0x08, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00},
        {0x00, 0x00, 0x04, 0x08, 0x10, 0x20, 0x10, 0x08, 0x04, 0x00, 0x00, 0x00},
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x3e, 0x00, 0x3e, 0x00, 0x00, 0x00, 0x00},
        {0x00, 0x00, 0x10, 0x08, 0x04, 0x02, 0x04, 0x08, 0x10, 0x00, 0x00, 0x00},
        {0x00, 0x00, 0x08, 0x00, 0x08, 0x08, 0x04, 0x02, 0x02, 0x22, 0x1c, 0x00},
        {0x00, 0x00, 0x1c, 0x20, 0x4e, 0x55, 0x55, 0x55, 0x4d, 0x21, 0x1e, 0x00},
        {0x00, 0x00, 0x77, 0x22, 0x3e, 0x22, 0x14, 0x14, 0x08, 0x08, 0x18, 0x00},
        {0x00, 0x00, 0x7e, 0x21, 0x21, 0x21, 0x3e, 0x21, 0x21, 0x21, 0x7e, 0x00},
        {0x00, 0x00, 0x1e, 0x21, 0x40, 0x40, 0x40, 0x40, 0x40, 0x21, 0x1e, 0x00},
        {0x00, 0x00, 0x7c, 0x22, 0x21, 0x21, 0x21, 0x21, 0x21, 0x22, 0x7c, 0x00},
        {0x00, 0x00, 0x7f, 0x21, 0x20, 0x24, 0x3c, 0x24, 0x20, 0x21, 0x7f, 0x00},
        {0x00, 0x00, 0x78, 0x20, 0x20, 0x24, 0x3c, 0x24, 0x20, 0x21, 0x7f, 0x00},
        {0x00, 0x00, 0x1e, 0x21, 0x41, 0x47, 0x40, 0x40, 0x40, 0x21, 0x1e, 0x00},
        {0x00, 0x00, 0x77, 0x22, 0x22, 0x22, 0x3e, 0x22, 0x22, 0x22, 0x77, 0x00},
        {0x00, 0x00, 0x3e, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x3e, 0x00},
        {0x00, 0x00, 0x38, 0x44, 0x44, 0x04, 0x04, 0x04, 0x04, 0x04, 0x1e, 0x00},
        {0x00, 0x00, 0x73, 0x22, 0x24, 0x38, 0x28, 0x24, 0x24, 0x22, 0x73, 0x00},
        {0x00, 0x00, 0x7f, 0x11, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x7c, 0x00},
        {0x00, 0x00, 0x77, 0x22, 0x22, 0x2a, 0x2a, 0x36, 0x36, 0x22, 0x63, 0x00},
        {0x00, 0x00, 0x72, 0x22, 0x26, 0x26, 0x2a, 0x32, 0x32, 0x22, 0x67, 0x00},
        {0x00, 0x00, 0x1c, 0x22, 0x41, 0x41, 0x41, 0x41, 0x41, 0x22, 0x1c, 0x00},
        {0x00, 0x00, 0x78, 0x20, 0x20, 0x20, 0x3e, 0x21, 0x21, 0x21, 0x7e, 0x00},
        {0x00, 0x1b, 0x1c, 0x22, 0x41, 0x41, 0x41, 0x41, 0x41, 0x22, 0x1c, 0x00},
        {0x00, 0x00, 0x73, 0x22, 0x24, 0x24, 0x3e, 0x21, 0x21, 0x21, 0x7e, 0x00},
        {0x00, 0x00, 0x3e, 0x41, 0x01, 0x01, 0x3e, 0x40, 0x40, 0x41, 0x3e, 0x00},
        {0x00, 0x00, 0x1c, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x49, 0x7f, 0x00},
        {0x00, 0x00, 0x1c, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x77, 0x00},
        {0x00, 0x00, 0x08, 0x08, 0x14, 0x14, 0x14, 0x22, 0x22, 0x22, 0x77, 0x00},
        {0x00, 0x00, 0x14, 0x14, 0x2a, 0x2a, 0x2a, 0x22, 0x22, 0x22, 0x77, 0x00},
        {0x00, 0x00, 0x77, 0x22, 0x14, 0x14, 0x08, 0x14, 0x14, 0x22, 0x77, 0x00},
        {0x00, 0x00, 0x1c, 0x08, 0x08, 0x08, 0x14, 0x14, 0x22, 0x22, 0x77, 0x00},
        {0x00, 0x00, 0x7f, 0x21, 0x10, 0x10, 0x08, 0x04, 0x04, 0x42, 0x7f, 0x00},
        {0x1c, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1c, 0x00},
        {0x00, 0x00, 0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x00, 0x00},
        {0x1c, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x1c, 0x00},
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x22, 0x14, 0x08},
        {0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x10, 0x00},
        {0x00, 0x00, 0x3d, 0x42, 0x42, 0x3e, 0x02, 0x3c, 0x00, 0x00, 0x00, 0x00},
        {0x00, 0x00, 0x7e, 0x21, 0x21, 0x21, 0x21, 0x3e, 0x20, 0x20, 0x60, 0x00},
        {0x00, 0x00, 0x3e, 0x41, 0x40, 0x40, 0x41, 0x3e, 0x00, 0x00, 0x00, 0x00},
        {0x00, 0x00, 0x3f, 0x42, 0x42, 0x42, 0x42, 0x3e, 0x02, 0x02, 0x06, 0x00},
        {0x00, 0x00, 0x3e, 0x41, 0x40, 0x7f, 0x41, 0x3e, 0x00, 0x00, 0x00, 0x00},
        {0x00, 0x00, 0x3c, 0x10, 0x10, 0x10, 0x10, 0x3c, 0x10, 0x10, 0x0c, 0x00},
        {0x3c, 0x02, 0x02, 0x3e, 0x42, 0x42, 0x42, 0x3f, 0x00, 0x00, 0x00, 0x00},
        {0x00, 0x00, 0x77, 0x22, 0x22, 0x22, 0x32, 0x2c, 0x20, 0x20, 0x60, 0x00},
        {0x00, 0x00, 0x3e, 0x08, 0x08, 0x08, 0x08, 0x38, 0x00, 0x00, 0x08, 0x00},
        {0x38, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x3c, 0x00, 0x00, 0x04, 0x00},
        {0x00, 0x00, 0x63, 0x24, 0x38, 0x28, 0x24, 0x26, 0x20, 0x20, 0x60, 0x00},
        {0x00, 0x00, 0x3e, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x18, 0x00},
        {0x00, 0x00, 0x6b, 0x2a, 0x2a, 0x2a, 0x2a, 0x74, 0x00, 0x00, 0x00, 0x00},
        {0x00, 0x00, 0x77, 0x22, 0x22, 0x22, 0x32, 0x6c, 0x00, 0x00, 0x00, 0x00},
        {0x00, 0x00, 0x3e, 0x41, 0x41, 0x41, 0x41, 0x3e, 0x00, 0x00, 0x00, 0x00},
        {0x70, 0x20, 0x3e, 0x21, 0x21, 0x21, 0x21, 0x7e, 0x00, 0x00, 0x00, 0x00},
        {0x07, 0x02, 0x3e, 0x42, 0x42, 0x42, 0x42, 0x3f, 0x00, 0x00, 0x00, 0x00},
        {0x00, 0x00, 0x7c, 0x10, 0x10, 0x10, 0x19, 0x76, 0x00, 0x00, 0x00, 0x00},
        {0x00, 0x00, 0x3e, 0x41, 0x06, 0x38, 0x41, 0x3e, 0x00, 0x00, 0x00, 0x00},
        {0x00, 0x00, 0x0c, 0x12, 0x10, 0x10, 0x10, 0x3c, 0x10, 0x10, 0x00, 0x00},
        {0x00, 0x00, 0x1b, 0x26, 0x22, 0x22, 0x22, 0x66, 0x00, 0x00, 0x00, 0x00},
        {0x00, 0x00, 0x08, 0x14, 0x14, 0x22, 0x22, 0x77, 0x00, 0x00, 0x00, 0x00},
        {0x00, 0x00, 0x14, 0x14, 0x2a, 0x2a, 0x22, 0x77, 0x00, 0x00, 0x00, 0x00},
        {0x00, 0x00, 0x77, 0x22, 0x1c, 0x1c, 0x22, 0x77, 0x00, 0x00, 0x00, 0x00},
        {0x30, 0x08, 0x08, 0x14, 0x14, 0x22, 0x22, 0x77, 0x00, 0x00, 0x00, 0x00},
        {0x00, 0x00, 0x7e, 0x22, 0x10, 0x08, 0x44, 0x7e, 0x00, 0x00, 0x00, 0x00},
        {0x06, 0x08, 0x08, 0x08, 0x08, 0x30, 0x08, 0x08, 0x08, 0x08, 0x06, 0x00},
        {0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08},
        {0x30, 0x08, 0x08, 0x08, 0x08, 0x06, 0x08, 0x08, 0x08, 0x08, 0x30, 0x00},
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46, 0x49, 0x31, 0x00, 0x00},
        {0x00, 0x1c, 0x1c, 0x1c, 0x1c, 0x1c, 0x1c, 0x1c, 0x1c, 0x1c, 0x00, 0x00}
    };
    // the remaining lines of this routine are similar to code developed and published in the
    // OPENGL big red book.  However I have modified the code slightly, and
    // SGI are not responsible for any errors, omissions etc.
    static GLuint fontOffset=0; // first display list
    if (!fontOffset) { // then make the raster fonts
        GLuint i;
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        fontOffset = glGenLists (128);
        for (i = 32; i < 127; i++) {
            glNewList(i+fontOffset, GL_COMPILE);
            glBitmap(8, 12, 0.0, 2.0, 10.0, 0.0, rasters[i-32]);
            glEndList();
        }
    }
    return fontOffset;
}

void displaytext(int x, int y, char *s)
{ // GWM July 2001 statistics text display at xy text S
    GLuint fontOffset=makeRasterFont(); // first display list
    glRasterPos2i(x,y);
    glListBase(fontOffset);
    glCallLists(strlen(s), GL_UNSIGNED_BYTE, (GLubyte *) s);
    glListBase(0);
    //    glPopAttrib ();
}


int writePrims( const int ypos, osg::Statistics& stats)
{
    char clin[128]; // buffer to print
    char ctmp[32];
    char intro[32]; // start of first line
    int npix=0; // offset from ypos
    static char *prtypes[]=
    {
        "  Point", // GL_POINTS               0x0000
        "  Lines", // GL_LINES                0x0001
        " LnLoop", // GL_LINE_LOOP            0x0002
        "  LnStr", // GL_LINE_SRIP            0x0002
        "   Tris", // GL_TRIANGLES            0x0004 
        " TriStr", // GL_TRIANGLE_STRIP       0x0005
        " TriFan", // GL_TRIANGLE_FAN         0x0006
        "  Quads", // GL_QUADS                0x0007
        " QudStr", // GL_QUAD_STRIP           0x0008
        "   Poly"  // GL_POLYGON              0x0009
    };         
            
    glColor3f(.9f,.9f,0.0f);

    if (stats.depth==1) sprintf(intro,"==> Bin %2d", stats._binNo);
    else                sprintf(intro,"General Stats: ");
    sprintf(clin,"%s  %d Drawables   %d Lights   %d Bins   %d Impostors", 
        intro ,stats.numDrawables, stats.nlights, stats.nbins, stats.nimpostor);
    displaytext(0,ypos-npix,clin);
    npix+=24;
    
    strcpy(clin,"              Total");
    
    unsigned int totalNumPrimives = 0;
    unsigned int totalNumIndices = 0;
    osg::Statistics::PrimtiveValueMap::iterator pItr;
    for(pItr=stats._primitiveCount.begin();
        pItr!=stats._primitiveCount.end();
        ++pItr)
    {
        totalNumPrimives += (pItr->second.first);
        totalNumIndices += (pItr->second.second);
        strcat(clin, prtypes[pItr->first]);
    }
    displaytext(0,ypos-npix,clin);
    npix+=12;
    
    strcpy(clin,"Primitives: ");
    sprintf(ctmp,"%7d", totalNumPrimives);
    strcat(clin, ctmp);
    for(pItr=stats._primitiveCount.begin();
        pItr!=stats._primitiveCount.end();
        ++pItr)
    {
        sprintf(ctmp,"%7d", pItr->second.first);
        strcat(clin, ctmp);
    }
    displaytext(0,ypos-npix,clin);
    npix+=12;

    strcpy(clin,"Indices:    ");
    sprintf(ctmp,"%7d", totalNumIndices);
    strcat(clin, ctmp);
    for(pItr=stats._primitiveCount.begin();
        pItr!=stats._primitiveCount.end();
        ++pItr)
    {
        sprintf(ctmp,"%7d", pItr->second.second);
        strcat(clin, ctmp);
    }
    displaytext(0,ypos-npix,clin);
    npix+=12;

    strcpy(clin,"Vertices:   ");
    sprintf(ctmp,"%7d", stats._vertexCount);
    strcat(clin, ctmp);
    displaytext(0,ypos-npix,clin);
    npix+=12;

    return npix;
}
