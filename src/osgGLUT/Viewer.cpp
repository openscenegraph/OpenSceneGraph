#include <stdlib.h>
#ifndef WIN32
#include <unistd.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <math.h>

#include "osgGLUT/Viewer"
#include "osgGLUT/GLUTEventAdapter"

#include <osg/OSG>
#include <osg/GeoState>
#include <osg/Scene>
#include <osg/Switch>
#include <osg/Billboard>
#include <osg/LOD>
#include <osg/DCS>
#include <osg/Light>
#include <osg/LightSource>
#include <osg/Geode>
#include <osg/Group>
#include <osg/Output>
#include <osg/Input>
#include <osg/Registry>
#include <osg/NodeVisitor>
#include <osg/Seg>
#include <osg/Notify>

#include <osgUtil/RenderVisitor>
#include <osgUtil/IntersectVisitor>
#include <osgUtil/DisplayListVisitor>

#include <osgUtil/TrackballManipulator>
#include <osgUtil/FlightManipulator>
#include <osgUtil/DriveManipulator>

#include <osg/Version>
#include <osgUtil/Version>


#ifdef WIN32
#define USE_FLTK
#define USE_GLUT
#endif

#include <GL/glut.h>

#ifdef WIN32
#include <sys/timeb.h>
#else
#endif

#include <osg/Timer>
osg::Timer g_timer;
osg::Timer_t g_initTime;

static GLenum polymodes [] = { GL_FILL, GL_LINE, GL_POINT };

using namespace osgGLUT;

Viewer* Viewer::s_theViewer = 0;

Viewer::Viewer()
{
    s_theViewer = this;


    fullscreen = false;
    _saved_ww = ww = 1024,
    _saved_wh = wh = 768;

    mx = ww/2,
    my = wh/2;
    mbutton = 0;


    polymode = 0;
    texture = 1;
    backface = 1;
    lighting = 1;
    flat_shade = 0;

    _printStats = false;

    #ifdef SGV_USE_RTFS
    fs = new RTfs( RTFS_MODE_RTC_SPIN );
    frame_rate = 16;
    fs->setUpdateRate( frame_rate );
    #endif

    _viewFrustumCullingActive = true;
    _smallFeatureCullingActive = true;

    _two_sided_lighting=0;

    _useDisplayLists = true;

    _saveFileName = "saved_model.osg";


    _sceneView = new osgUtil::SceneView;
    _sceneView->setDefaults();

    registerCameraManipulator(new osgUtil::TrackballManipulator);
    registerCameraManipulator(new osgUtil::FlightManipulator);
    registerCameraManipulator(new osgUtil::DriveManipulator);
    
    osg::notify(osg::INFO)<<"Scene Graph Viewer (sgv)"<<endl;
    osg::notify(osg::INFO)<<"   '"<<osgGetLibraryName()<<"' Version "<<osgGetVersion()<<endl;
    osg::notify(osg::INFO)<<"   '"<<osgUtilGetLibraryName()<<"' Version "<<osgUtilGetVersion()<<endl;

    _initialTick = _timer.tick();
    _frameTick = _initialTick;

}


Viewer::~Viewer()
{
}





bool Viewer::init(osg::Node* rootnode)
{

    bool saveModel = false;

    GLUTEventAdapter::setWindowSize( 0,0,ww, wh );
    GLUTEventAdapter::setButtonMask(0);

    _sceneView->setViewport(0,0,ww,wh);

    glutInitWindowSize( ww, wh );
    //glutInit( &argc, argv );    // I moved this into main to avoid passing
    				  // argc and argv to the Viewer
    glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_MULTISAMPLE);
    glutCreateWindow( "OSG Viewer" );

    glutReshapeFunc(    reshapeCB );
    glutVisibilityFunc( visibilityCB );
    glutDisplayFunc(    displayCB );
    glutKeyboardFunc(   keyboardCB );

    glutMouseFunc( mouseCB );
    glutMotionFunc( mouseMotionCB );
    glutPassiveMotionFunc( mousePassiveMotionCB );

    #ifdef USE_FLTK
    // required to specify the idle function when using FLTK.
    // GLUT and FLTK GLUT differ in that GLUT implicitly calls visibilityFunc
    // on window creation, while FLTK calls it when the windows is iconised
    // or deiconsied but not on window creation.
    visibilityCB(GLUT_VISIBLE);
    #endif    

    if (_useDisplayLists)
    {
        // traverse the scene graph setting up all osg::GeoSet's so they will use
        // OpenGL display lists.
        osgUtil::DisplayListVisitor dlv(osgUtil::DisplayListVisitor::SWITCH_ON_DISPLAY_LISTS);
        rootnode->accept(dlv);
    }
    
    _sceneView->setSceneData(rootnode);

    if (saveModel) 
    {
        osg::saveNodeFile(*rootnode, _saveFileName.c_str());
    }
 
    selectCameraManipulator(0);

    osg::ref_ptr<GLUTEventAdapter> ea = new GLUTEventAdapter;
    _cameraManipulator->home(*ea,*this);


     // std::string name = Registry::instance()->createLibraryNameForExt("osg");
     // Registry::instance()->loadLibrary(name);
     // Registry::instance()->closeLibrary(name);

    return true;

}

void Viewer::registerCameraManipulator(osgUtil::CameraManipulator* cm)
{
    _cameraManipList.push_back(cm);
}

void Viewer::selectCameraManipulator(unsigned int pos)
{
    if (pos>=_cameraManipList.size()) return;

    _cameraManipulator = _cameraManipList[pos];
    _cameraManipulator->setCamera(_sceneView->getCamera());
    _cameraManipulator->setNode(_sceneView->getSceneData());


    osg::ref_ptr<GLUTEventAdapter> ea = new GLUTEventAdapter;
    _cameraManipulator->init(*ea,*this);
}

void Viewer::needWarpPointer(int x,int y)
{
    glutWarpPointer(x,y);
}

bool Viewer::update()
{
    osg::ref_ptr<GLUTEventAdapter> ea = new GLUTEventAdapter;
    ea->adaptFrame(clockSeconds());

    if (_cameraManipulator->update(*ea,*this)) 
    {
//        osg::notify(osg::INFO) << "Handled update frame"<<endl;
    }

    updateFrameTick();

    if (_printStats) osg::notify(osg::NOTICE) << "\033[0;0H";
    if (_printStats) osg::notify(osg::NOTICE) << "frameRate() = "<<frameRate()<<endl;

    return true;
}


bool Viewer::traverse()
{
    return true;
}


bool Viewer::draw()
{
    //    osg::notify(osg::INFO) << "Viewer::draw("<<mx<<","<<my<<") scale("<<bsphere._radius/(float)ww<<")"<<endl;

    osg::Timer_t beforeTraversal = _timer.tick();

    _sceneView->cull();

    float timeTraversal = _timer.delta_m(beforeTraversal,_timer.tick());
    if (_printStats) osg::notify(osg::NOTICE) << "Time of Cull Traversal "<<timeTraversal<<"ms   "<<endl;


    osg::Timer_t beforeDraw = _timer.tick();
    
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,_two_sided_lighting);

    _sceneView->draw();
    
     float timeDraw = _timer.delta_m(beforeDraw,_timer.tick());
     if (_printStats) osg::notify(osg::NOTICE) << "Time of Draw "<<timeDraw<<"ms   "<<endl;

    return true;
}


void Viewer::displayCB()
{
    s_theViewer->display();
}


void Viewer::reshapeCB(GLint w, GLint h)
{
    s_theViewer->reshape(w, h);
}


void Viewer::visibilityCB( int state )
{
    s_theViewer->visibility(state);
}


void Viewer::mouseCB(int button, int state, int x, int y)
{
    s_theViewer->mouse(button, state, x, y);
}


void Viewer::mouseMotionCB(int x, int y)
{
    s_theViewer->mouseMotion(x,y);
}


void Viewer::mousePassiveMotionCB(int x, int y)
{
    s_theViewer->mousePassiveMotion(x,y);
}


void Viewer::keyboardCB(unsigned char key, int x, int y)
{
    s_theViewer->keyboard(key,x,y);
}


void Viewer::display()
{
    // application traverasal.
    update();

    draw();

    glutSwapBuffers();

//    cout << "Time elapsed "<<_timer.delta_s(_initialTick,_timer.tick())<<endl;

    #ifdef SGV_USE_RTFS
    fs->frame();
    #endif
}


void Viewer::reshape(GLint w, GLint h)
{
    ww = w;
    wh = h;
    
    _sceneView->setViewport(0,0,ww,wh);

    osg::ref_ptr<GLUTEventAdapter> ea = new GLUTEventAdapter;
    ea->adaptResize(clockSeconds(),0,0,ww,wh);
 
    if (_cameraManipulator->update(*ea,*this)) 
    {
//        osg::notify(osg::INFO) << "Handled reshape "<<endl;
    }
}


void Viewer::visibility(int state)
{
    if (state == GLUT_VISIBLE)
        glutIdleFunc( displayCB );
    else
        glutIdleFunc(0L);
}


void Viewer::mouseMotion(int x, int y)
{
    osg::ref_ptr<GLUTEventAdapter> ea = new GLUTEventAdapter;
    ea->adaptMouseMotion(clockSeconds(),x,y);
 
    if (_cameraManipulator->update(*ea,*this)) 
    {
//        osg::notify(osg::INFO) << "Handled mouseMotion "<<ea->_buttonMask<<" x="<<ea->_mx<<" y="<<ea->_my<<endl;
    }


    mx = x;
    my = y;



}


void Viewer::mousePassiveMotion(int x, int y)
{
    osg::ref_ptr<GLUTEventAdapter> ea = new GLUTEventAdapter;
    ea->adaptMousePassiveMotion(clockSeconds(),x,y);

    if (_cameraManipulator->update(*ea,*this)) 
    {
//        osg::notify(osg::INFO) << "Handled mousePassiveMotion "<<ea->_buttonMask<<" x="<<ea->_mx<<" y="<<ea->_my<<endl;
    }


}


void Viewer::mouse(int button, int state, int x, int y)
{
    osg::ref_ptr<GLUTEventAdapter> ea = new GLUTEventAdapter;
    ea->adaptMouse(clockSeconds(),button,state,x,y);

    if (_cameraManipulator->update(*ea,*this)) 
    {
//        osg::notify(osg::INFO) << "Handled mouse "<<ea->_buttonMask<<" x="<<ea->_mx<<" y="<<ea->_my<<endl;
    }



}


void Viewer::keyboard(unsigned char key, int x, int y)
{
    osg::ref_ptr<GLUTEventAdapter> ea = new GLUTEventAdapter;
    ea->adaptKeyboard(clockSeconds(),key,x,y);

    if (_cameraManipulator->update(*ea,*this)) return;

    if (key>='1' && key<='3')
    {
        int pos = key-'1';
        selectCameraManipulator(pos);
    }

    switch( key )
    {
            
        case '/' :
            if (_sceneView->getLODBias()>0.5) _sceneView->setLODBias(_sceneView->getLODBias()/1.5f);
            break;

        case '*' :
            if (_sceneView->getLODBias()<30) _sceneView->setLODBias(_sceneView->getLODBias()*1.5f);
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
                // traverse the scene graph setting up all osg::GeoSet's so they will use
                // OpenGL display lists.
                osgUtil::DisplayListVisitor dlv(osgUtil::DisplayListVisitor::SWITCH_ON_DISPLAY_LISTS);
                _sceneView->getSceneData()->accept(dlv);
                osg::notify(osg::NOTICE) << "Switched on use of OpenGL Display Lists."<<endl;
            }
            else
            {
                // traverse the scene graph setting up all osg::GeoSet's so they will use
                // OpenGL display lists.
                osgUtil::DisplayListVisitor dlv(osgUtil::DisplayListVisitor::SWITCH_OFF_DISPLAY_LISTS);
                _sceneView->getSceneData()->accept(dlv);
                osg::notify(osg::NOTICE) << "Switched off use of OpenGL Display Lists."<<endl;
            }
            break;

        case 'p' :
            _printStats = !_printStats;
            break;

        case 's' :
            flat_shade = 1  - flat_shade ;
            if( flat_shade )
                glShadeModel( GL_FLAT );
            else
                glShadeModel( GL_SMOOTH );

            break;

        case 'b' :

            backface = 1 - backface;
            if( backface )
                _sceneView->getGlobalState()->setMode(osg::GeoState::FACE_CULL,osg::GeoState::ON);
            else
                _sceneView->getGlobalState()->setMode(osg::GeoState::FACE_CULL,osg::GeoState::OVERRIDE_OFF);

            break;


        case 'l' :
            lighting = 1  - lighting ;
            if( lighting )
                _sceneView->getGlobalState()->setMode(osg::GeoState::LIGHTING,osg::GeoState::ON);
            else
                _sceneView->getGlobalState()->setMode(osg::GeoState::LIGHTING,osg::GeoState::OVERRIDE_OFF);
            break;

        case 'L' :
        {
            osgUtil::SceneView::LightingMode lm= _sceneView->getLightingMode();
            switch(lm)
            {
            case(osgUtil::SceneView::HEADLIGHT) : lm = osgUtil::SceneView::SKY_LIGHT; break;
            case(osgUtil::SceneView::SKY_LIGHT) : lm = osgUtil::SceneView::NO_SCENEVIEW_LIGHT; break;
            case(osgUtil::SceneView::NO_SCENEVIEW_LIGHT) : lm = osgUtil::SceneView::HEADLIGHT; break;
            }
            _sceneView->setLightingMode(lm);
            break;
        }
        case 't' :
            texture = 1 - texture;
            if (texture)
                _sceneView->getGlobalState()->setMode(osg::GeoState::TEXTURE,osg::GeoState::INHERIT);
            else
                _sceneView->getGlobalState()->setMode(osg::GeoState::TEXTURE,osg::GeoState::OVERRIDE_OFF);
            break;

        case 'T' :
            _two_sided_lighting = 1 - _two_sided_lighting;
            break;

        case 'w' :
            polymode = (polymode+1)%3;
            glPolygonMode( GL_FRONT_AND_BACK, polymodes[polymode] );
            break;

        case 'f' :
            fullscreen = !fullscreen;
            if (fullscreen)
            {
                _saved_ww = ww;
                _saved_wh = wh;
                glutFullScreen();
            } else
            {
                //glutPositionWindow(wx,wy);
                glutReshapeWindow(_saved_ww,_saved_wh);
            }
            break;

        case 'o' :
            if (_sceneView->getSceneData() && osg::Registry::instance()->writeNode(*_sceneView->getSceneData(),_saveFileName))
            {
                osg::notify(osg::NOTICE) << "Saved scene to '"<<_saveFileName<<"'"<<endl;
            }
            break;

        case 'c' :
            _smallFeatureCullingActive = !_smallFeatureCullingActive;
            if (_smallFeatureCullingActive)
            {
                osg::notify(osg::NOTICE) << "Small feature culling switched on   "<<endl;
            }
            else
            {
                osg::notify(osg::NOTICE) << "Small feature culling switched off  "<<endl;
            }
            _sceneView->getRenderVisitor()->setCullingActive(osgUtil::RenderVisitor::SMALL_FEATURE_CULLING,_smallFeatureCullingActive);
            break;

        case 'C' :
            _viewFrustumCullingActive = !_viewFrustumCullingActive;
            _sceneView->getRenderVisitor()->setCullingActive(osgUtil::RenderVisitor::VIEW_FRUSTUM_CULLING,_viewFrustumCullingActive);
            if (_viewFrustumCullingActive)
            {
                osg::notify(osg::NOTICE) << "View frustum culling switched on   "<<endl;
            }
            else
            {
                osg::notify(osg::NOTICE) << "View frustum culling switched off  "<<endl;
            }
            break;

        case '?' :
        case 'h' :
            help(osg::notify(osg::NOTICE));
            break;

        case 'i' :
        case 'r' :
            {
            osg::notify(osg::NOTICE) << "***** Intersecting **************"<< endl;


            osg::Vec3 near_point,far_point;
            if (!_sceneView->projectWindowXYIntoObject(x,wh-y,near_point,far_point)) 
            {
                osg::notify(osg::NOTICE) << "Failed to calculate intersection ray."<<endl;
                return;
            }

            osg::ref_ptr<osg::Seg> seg = new osg::Seg;
            seg->set(near_point,far_point);
            osg::notify(osg::NOTICE) << "start("<<seg->start()<<")  end("<<seg->end()<<")"<<endl;

            osgUtil::IntersectVisitor iv;
            iv.addSeg(seg.get());

            float startTime = clockSeconds();

            _sceneView->getSceneData()->accept(iv);
            
            float endTime = clockSeconds();

            osg::notify(osg::NOTICE) << "Time for interesection = "<<(endTime-startTime)*1000<<"ms"<<endl;
    
            if (iv.hits())
            {
                osgUtil::IntersectVisitor::HitList& hitList = iv.getHitList(seg.get());
                for(osgUtil::IntersectVisitor::HitList::iterator hitr=hitList.begin();
                                                             hitr!=hitList.end();
                                                             ++hitr)
                {
                    osg::Vec3 ip = hitr->_intersectPoint;
                    osg::Vec3 in = hitr->_intersectNormal;
                    osg::Geode* geode = hitr->_geode;
                    osg::notify(osg::NOTICE) << "  Itersection Point ("<<ip<<") Normal ("<<in<<")"<<endl;
                    if (hitr->_matrix)
                    {
                        osg::Vec3 ipEye = ip*(*(hitr->_matrix));
                        osg::Vec3 inEye = (in+ip)*(*(hitr->_matrix))-ipEye;
                        inEye.normalize();
                        if (geode) osg::notify(osg::NOTICE) << "Geode '"<<geode->getName()<<endl;
                        osg::notify(osg::NOTICE) << "  Eye Itersection Point ("<<ipEye<<") Normal ("<<inEye<<")"<<endl;


                    }
                    if (key=='r' && geode)
                    {
                        // remove geoset..
                        osg::GeoSet* gset = hitr->_geoset;
                        osg::notify(osg::NOTICE) << "  geoset ("<<gset<<") "<<geode->removeGeoSet(gset)<<")"<<endl;
                    }

                }
                
            }

            osg::notify(osg::NOTICE) << endl << endl;
            }
            break;

        case 27 :                // Escape
            exit(0);
            break;
    }
}

void Viewer::help(ostream& fout)
{

fout <<endl
     <<"Scene Graph Viewer (sgv) keyboard bindings:"<<endl
     <<endl
     <<"1     Select the trackball camera manipulator."<<endl
     <<"      Left mouse button - rotate,"<<endl
     <<"      Middle (or Left & Right) mouse button - pan,"<<endl
     <<"      Right mouse button - zoom."<<endl
     <<"2     Select the flight camera manipulator."<<endl
     <<"      Left mouse button - speed up,"<<endl
     <<"      Middle (or Left & Right) mouse button - stop,"<<endl
     <<"      Right mouse button - slow down, reverse."<<endl
     <<"      Move mouse left to roll left, right to roll right."<<endl
     <<"      Move mouse back (down) to pitch nose up, forward to pitch down."<<endl
     <<"      In mode Q, the default, selected by pressing 'q'"<<endl
     <<"        The flight path is yawed automatically into the turn to"<<endl
     <<"        produce a similar effect as flying an aircaft."<<endl
     <<"      In mode A, selected by pressing 'a'"<<endl
     <<"        The flight path is not yawed automatically into the turn,"<<endl
     <<"        producing a similar effect as space/marine flight."<<endl
     <<"3     Select the drive camera manipulator."<<endl
     <<"      In mode Q, the default, selected by pressing 'q'"<<endl
     <<"        Move mouse left to turn left, right to turn right."<<endl
     <<"        Move mouse back (down) to reverse, forward to drive forward."<<endl
     <<"      In mode A, selected by pressing 'a'"<<endl
     <<"        Move mouse left to turn left, right to turn right."<<endl
     <<"        Left mouse button - speed up,"<<endl
     <<"        Middle (or Left & Right) mouse button - stop,"<<endl
     <<"        Right mouse button - slow down, reverse."<<endl
     <<endl
     <<"+     Half the frame delay which speeds up the frame rate on Linux and Windows."<<endl
     <<"-     Double the frame delay and therefore reduce the frame rate on Linux"<<endl
     <<"      and Windows."<<endl
     <<endl
     <<"/     Divide the Level-Of-Detail (LOD) bias by 1.5, to encourage the"<<endl
     <<"      selection of more complex LOD children."<<endl
     <<"*     Multiple the Level-of-Detail (LOD) bias by 1.5, to encourage the"<<endl
     <<"      selection of less complex LOD children."<<endl
     <<endl
     <<"c     Toggle Small Feature Culling on or off."<<endl
     <<"C     Toggle View Frustum Culling on or off."<<endl
     <<"d     Toggle use of OpenGL's display lists."<<endl
     <<"b     Toggle OpenGL's backface culling."<<endl
     <<"t     Toggle OpenGL texturing on or off."<<endl
     <<"T     Toggle OpenGL two-sided lighting on or off."<<endl
     <<"l     Toggle OpenGL lighting on or off."<<endl
     <<"L     Toggle SceneView lighting mode between HEADLIGHT, SKY_LIGHT"<<endl
     <<"      and NO_SCENEVIEW_LIGHT."<<endl
     <<"s     Toggle OpenGL shade model between flat and smooth shading."<<endl
     <<"w     Toggle OpenGL polygon mode between solid, wireframe and points modes."<<endl
     <<endl
     <<"i     Calculate and report the intersections with the scene under the"<<endl
     <<"      current mouse x and mouse y position."<<endl
     <<endl
     <<"r     Calculate and report the intersections with the scene under the"<<endl
     <<"      current mouse x and mouse y position and delete the nearest"<<endl
     <<"      interesected geoset."<<endl
     <<endl
     <<"p     Print frame rate statistics on each frame."<<endl
     <<"o     Output the loaded scene to 'saved_model.osg'."<<endl
     <<"?/h   Print out sgv's keyboard bindings."<<endl
     <<"f     Toggle between fullscreen and the previous window size. Note, GLUT"<<endl
     <<"      fullscreen works properly on Windows and Irix, but on Linux"<<endl
     <<"      it just maximizes the window and leaves the window's borders."<<endl
     <<"Space Reset scene to the default view."<<endl
     <<"Esc   Exit sgv."<<endl;
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
    updateFrameTick();
    #ifdef SGV_USE_RTFS
    fs->start();
    #endif
    glutMainLoop();
    return true;
}
