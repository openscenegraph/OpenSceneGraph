/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield 
 *
 * This library is open source and may be redistributed and/or modified under  
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or 
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRA;NTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * OpenSceneGraph Public License for more details.
*/
#include <osg/View>
#include <osg/Notify>
#include <osg/TexEnv>
#include <osg/DeleteHandler>

using namespace osg;

View::View()
{
    // osg::notify(osg::NOTICE)<<"Constructing osg::View"<<std::endl;

    setLightingMode(HEADLIGHT);

    setCamera(new osg::Camera);

#if 1    
    double height = osg::DisplaySettings::instance()->getScreenHeight();
    double width = osg::DisplaySettings::instance()->getScreenWidth();
    double distance = osg::DisplaySettings::instance()->getScreenWidth();
    
    double vfov = osg::RadiansToDegrees(atan2(height/2.0f,distance)*2.0);

    _camera->setProjectionMatrixAsPerspective( vfov, width/height, 1.0f,10000.0f);
#else
    _camera->setProjectionMatrixAsFrustum(-0.325, 0.325, -0.26, 0.26, 1.0f,10000.0f);
#endif

    _camera->setClearColor(osg::Vec4f(0.2f, 0.2f, 0.4f, 1.0f));
    
    osg::StateSet* stateset = _camera->getOrCreateStateSet();
    stateset->setGlobalDefaults();
}

View::View(const osg::View& view, const osg::CopyOp& copyop):
    Object(view,copyop),
    _lightingMode(view._lightingMode),
    _light(view._light),
    _camera(view._camera),
    _slaves(view._slaves)
{
}


View::~View()
{
    osg::notify(osg::INFO)<<"Destructing osg::View"<<std::endl;

    if (_camera.valid())
    {
        _camera->setView(0);
        _camera->setCullCallback(0);
    }
    
    // detatch the cameras from this View to prevent dangling pointers
    for(Slaves::iterator itr = _slaves.begin();
        itr != _slaves.end();
        ++itr)
    {
        Slave& cd = *itr;
        cd._camera->setView(0);
        cd._camera->setCullCallback(0);
    }
    
    _camera = 0;
    _slaves.clear();
    _light = 0;    

    if (osg::Referenced::getDeleteHandler())
    {
        // osg::Referenced::getDeleteHandler()->setNumFramesToRetainObjects(0);
        osg::Referenced::getDeleteHandler()->flushAll();
    }

    
    osg::notify(osg::INFO)<<"Done destructing osg::View"<<std::endl;
}

void View::setLightingMode(LightingMode lightingMode)
{
    _lightingMode = lightingMode;
    if (_lightingMode != NO_LIGHT && !_light) 
    {
        _light = new osg::Light;
        _light->setLightNum(0);
        _light->setAmbient(Vec4(0.00f,0.0f,0.00f,1.0f));
        _light->setDiffuse(Vec4(0.8f,0.8f,0.8f,1.0f));
        _light->setSpecular(Vec4(1.0f,1.0f,1.0f,1.0f));
    }
}


void View::setCamera(osg::Camera* camera)
{
    if (_camera.valid()) _camera->setView(0);
    
    _camera = camera;

    if (_camera.valid()) _camera->setView(this);
}

void View::updateSlaves()
{
    for(unsigned int i=0; i<_slaves.size(); ++i)
    {
        updateSlave(i);
    }
}

void View::updateSlave(unsigned int i)
{
    if (i >= _slaves.size() || !_camera) return;

    Slave& slave = _slaves[i];

    if (slave._camera->getReferenceFrame()==osg::Transform::RELATIVE_RF)
    {
        slave._camera->setProjectionMatrix(_camera->getProjectionMatrix() * slave._projectionOffset);
        slave._camera->setViewMatrix(_camera->getViewMatrix() * slave._viewOffset);
    }

    slave._camera->inheritCullSettings(*_camera);
    if (slave._camera->getInheritanceMask() & osg::CullSettings::CLEAR_COLOR) slave._camera->setClearColor(_camera->getClearColor());
}

bool View::addSlave(osg::Camera* camera, const osg::Matrix& projectionOffset, const osg::Matrix& viewOffset, bool useMastersSceneData)
{
    if (!camera) return false;

    camera->setView(this);

    unsigned int i = _slaves.size();

    _slaves.push_back(Slave(camera, projectionOffset, viewOffset, useMastersSceneData));

    updateSlave(i);

    return true;
}

bool View::removeSlave(unsigned int pos)
{
    if (pos >= _slaves.size()) return false;

    _slaves[pos]._camera->setView(0);
    _slaves[pos]._camera->setCullCallback(0);
    
    _slaves.erase(_slaves.begin()+pos);

    return true;
}

View::Slave* View::findSlaveForCamera(osg::Camera* camera)
{
    if (_camera == camera) return 0;

    for(unsigned int i=0; i<_slaves.size(); ++i)
    {
        if (_slaves[i]._camera == camera) return &(_slaves[i]);
    }
    
    return 0;
}

