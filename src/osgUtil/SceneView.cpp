#include "osgUtil/SceneView"
#include "osg/Notify"

using namespace osg;
using namespace osgUtil;

SceneView::SceneView()
{
    _calc_nearfar = true;

    _backgroundColor.set(0.2f, 0.2f, 0.4f, 1.0f);

    _near_plane = 1.0f;
    _far_plane = 1.0f;
    
    _lodBias = 1.0f;

    _lightingMode=HEADLIGHT;
}

SceneView::~SceneView()
{
}

void SceneView::setDefaults()
{
    _globalState = new osg::GeoState;

    _lightingMode=HEADLIGHT;
    _light = new osg::Light;

    _camera = new osg::Camera;

    _renderVisitor = new osgUtil::RenderVisitor;
    
    _globalState->setGlobalDefaults();
    _globalState->setMode(osg::GeoState::LIGHTING, osg::GeoState::ON);

    _backgroundColor.set(0.2f, 0.2f, 0.4f, 1.0f);

}


void SceneView::cull()
{
    if (!_renderVisitor) return;
    if (!_sceneData) return;
    
    _camera->setAspectRatio((GLfloat)_view[2]/(GLfloat) _view[3]);

    _renderVisitor->reset();
    
    _renderVisitor->setGlobalState(_globalState.get());
    _renderVisitor->setLODBias(_lodBias);
//    _renderVisitor->setPerspective(60.0f, (GLfloat)_view[2]/(GLfloat) _view[3], _near_plane, _far_plane );
    _renderVisitor->setPerspective(*_camera);
    _renderVisitor->setLookAt(*_camera);

    _sceneData->accept(*_renderVisitor);

    if (_calc_nearfar)
    {
	if (_renderVisitor->calcNearFar(_near_plane,_far_plane))
	{
	    // shift the far plane slight further away from the eye point.
	    // and shift the near plane slightly near the eye point, this
	    // will give a little space betwenn the near and far planes
	    // and the model, crucial when the naer and far planes are
	    // coincedent.
	    _far_plane  *= 1.05;
	    _near_plane *= 0.95;

	    // if required clamp the near plane to prevent negative or near zero
	    // near planes.
	    float min_near_plane = _far_plane*0.0005f;
	    if (_near_plane<min_near_plane) _near_plane=min_near_plane;
	}
	else
	{
	    _near_plane = 1.0f;
	    _far_plane = 1000.0f;
	}

	_camera->setNearPlane(_near_plane);
	_camera->setFarPlane(_far_plane);
    }
}
    
void SceneView::draw()
{
    if (!_renderVisitor) return;
    if (!_sceneData) return;

    glViewport( _view[0], _view[1], _view[2], _view[3] );

    glEnable( GL_DEPTH_TEST );

    glClearColor( _backgroundColor[0], _backgroundColor[1], _backgroundColor[2], _backgroundColor[3]);

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

//     glMatrixMode( GL_PROJECTION );
//     glLoadIdentity();
//     gluPerspective( 60.0f, (GLfloat)_view[2]/(GLfloat) _view[3], _near_plane, _far_plane );

    _camera->draw_PROJECTION();

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    if (_lightingMode==HEADLIGHT)
    {
        _light->apply();
    }

    _camera->draw_MODELVIEW();
    
//     gluLookAt( _camera->getEyePoint().x(),  _camera->getEyePoint().y(),  _camera->getEyePoint().z(),
// 	       _camera->getLookPoint().x(), _camera->getLookPoint().y(), _camera->getLookPoint().z(), 
// 	       _camera->getUpVector().x(),  _camera->getUpVector().y(),  _camera->getUpVector().z());

    if (_lightingMode==SKY_LIGHT)
    {
        _light->apply();
    }


//    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,_two_sided_lighting);


    glGetDoublev(GL_MODELVIEW_MATRIX,_model);
    glGetDoublev(GL_PROJECTION_MATRIX,_proj);


    _renderVisitor->render();


}

/** Calculate, via glUnProject, the object coordinates of a window point.
    Note, current implementation requires that SceneView::draw() has been previously called
    for projectWindowIntoObject to produce valid values.  As per OpenGL
    windows coordinates are calculated relative to the bottom left of the window.*/
bool SceneView::projectWindowIntoObject(const osg::Vec3& window,osg::Vec3& object) const
{
    GLdouble sX,sY,sZ;
    GLint result_start = gluUnProject((GLdouble)window[0],(GLdouble)window[1],(GLdouble)window[2],
                                     _model,_proj,_view,
                                     &sX,&sY,&sZ);
    if (result_start)
    {
        object.set(sX,sY,sZ);
        return true;
    }
    else
    {
        return false;
    }
}

/** Calculate, via glUnProject, the object coordinates of a window x,y
    when projected onto the near and far planes.
    Note, current implementation requires that SceneView::draw() has been previously called
    for projectWindowIntoObject to produce valid values.  As per OpenGL
    windows coordinates are calculated relative to the bottom left of the window.*/
bool SceneView::projectWindowXYIntoObject(int x,int y,osg::Vec3& near_point,osg::Vec3& far_point) const
{
    GLdouble nX,nY,nZ;
    GLint result_near = gluUnProject((GLdouble)x,(GLdouble)y,(GLdouble)0.0,
                                     _model,_proj,_view,
                                     &nX,&nY,&nZ);


    if (result_near==0) return false;

    GLdouble fX,fY,fZ;
    GLint result_far = gluUnProject((GLdouble)x,(GLdouble)y,(GLdouble)1.0,
                                     _model,_proj,_view,
                                     &fX,&fY,&fZ);

    if (result_far==0) return false;

    near_point.set(nX,nY,nZ);    
    far_point.set(fX,fY,fZ);

    return true;
}

/** Calculate, via glProject, the object coordinates of a window.
    Note, current implementation requires that SceneView::draw() has been previously called
    for projectWindowIntoObject to produce valid values.  As per OpenGL
    windows coordinates are calculated relative to the bottom left of the window.*/
bool SceneView::projectObjectIntoWindow(const osg::Vec3& object,osg::Vec3& window) const
{
    GLdouble sX,sY,sZ;
    GLint result_start = gluProject((GLdouble)object[0],(GLdouble)object[1],(GLdouble)object[2],
                                     _model,_proj,_view,
                                     &sX,&sY,&sZ);
    if (result_start)
    {
        window.set(sX,sY,sZ);
        return true;
    }
    else
    {
        return false;
    }
}
