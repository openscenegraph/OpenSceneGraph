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

#include <osgTXP/TrPageArchive.h>
#include <osgTXP/TrPageViewer.h>

/*
#if defined(WIN32) && !defined(__CYGWIN__)
#include <sys/timeb.h>
#else
#endif
*/

using namespace osg;
using namespace osgUtil;
using namespace osgGLUT;
using namespace osgGA;

using namespace std;
using namespace txp;

PagingViewer::PagingViewer() : Viewer()
{
	pageManage = NULL;
}

bool PagingViewer::Init(OSGPageManager *in_manage,OSGPageManager::ThreadMode threadMode)
{
	pageManage = in_manage;

	// Start up the thread if needed
	if (threadMode != OSGPageManager::ThreadNone) {
		ThreadID newThread;
		pageManage->StartThread(threadMode,newThread);
	}
	
	return true;
}

/* App call
	This replaces the app() call for the base viewer.
	In this one, we ask the page manager to load stuff in.
 */
float PagingViewer::app(unsigned int viewport)
{
    osg::Timer_t beforeApp = _timer.tick();

    // update the camera manipulator.
    osg::ref_ptr<GLUTEventAdapter> ea = osgNew GLUTEventAdapter;
    ea->adaptFrame(_frameStamp->getReferenceTime());

    bool handled = false;
    for (EventHandlerList::iterator eh = _viewportList[viewport]._eventHandlerList.begin();         eh != _viewportList[viewport]._eventHandlerList.end();
         eh++ )
    {
        if ( eh->valid() )
        {
            if ( (*eh)->handle(*ea,*this) )
            {
                handled = true;
                break;
            }
        }
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

	// Update the paging
	if (pageManage) {
		int numTile = 1;
		osgUtil::SceneView *sceneView = getViewportSceneView(viewport);
		osg::Camera *camera = sceneView->getCamera();
		const Vec3 &eyePt = camera->getEyePoint();
		double eyeX = eyePt[0];
		double eyeY = eyePt[1];

		/* If we're in ThreadFree mode, merge in whatever may be ready.
		   If we're in non-thread mode, load in the given number of tiles (maximum).
		 */
		if (pageManage->GetThreadMode() == OSGPageManager::ThreadFree) {
			pageManage->MergeUpdateThread((osg::Group *)sceneView->getSceneData());
			pageManage->UpdatePositionThread(eyeX,eyeY);
		} else {
			pageManage->UpdateNoThread((osg::Group *)sceneView->getSceneData(),eyeX,eyeY,numTile);	
		}
	}

    // do app traversal.
    
    getViewportSceneView(viewport)->setFrameStamp(_frameStamp.get());
    getViewportSceneView(viewport)->app();

    osg::Timer_t beforeCull = _timer.tick();

    return  _timer.delta_m(beforeApp,beforeCull);
}

bool PagingViewer::run()
{    
    updateFrameTick();
    return Window::run();
}
