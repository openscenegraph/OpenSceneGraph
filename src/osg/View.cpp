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

View::View():
    Object(true)
{
    // OSG_NOTICE<<"Constructing osg::View"<<std::endl;

    setLightingMode(HEADLIGHT);

    _camera = new osg::Camera;
    _camera->setView(this);

    double height = osg::DisplaySettings::instance()->getScreenHeight();
    double width = osg::DisplaySettings::instance()->getScreenWidth();
    double distance = osg::DisplaySettings::instance()->getScreenDistance();
    double vfov = osg::RadiansToDegrees(atan2(height/2.0f,distance)*2.0);

    _camera->setProjectionMatrixAsPerspective( vfov, width/height, 1.0f,10000.0f);

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
    OSG_INFO<<"Destructing osg::View"<<std::endl;

    if (_camera.valid())
    {
        _camera->setView(0);
        _camera->setCullCallback(0);
    }

    // detach the cameras from this View to prevent dangling pointers
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

#if 0
    if (osg::Referenced::getDeleteHandler())
    {
        // osg::Referenced::getDeleteHandler()->setNumFramesToRetainObjects(0);
        osg::Referenced::getDeleteHandler()->flushAll();
    }
#endif

    OSG_INFO<<"Done destructing osg::View"<<std::endl;
}

void View::take(osg::View& rhs)
{
    // copy across the contents first
    _lightingMode = rhs._lightingMode;
    _light = rhs._light;
    _camera = rhs._camera;
    _slaves = rhs._slaves;

    // update the cameras so they all now see this View as their parent View
    if (_camera.valid()) _camera->setView(this);

    for(unsigned int i=0; i<_slaves.size(); ++i)
    {
        if (_slaves[i]._camera.valid()) _slaves[i]._camera->setView(this);
    }

    // then clear the passing in view.
    rhs._light = 0;
    rhs._camera = 0;
    rhs._slaves.clear();
}


void View::setLightingMode(LightingMode lightingMode)
{
    _lightingMode = lightingMode;
    if (_lightingMode != NO_LIGHT && !_light)
    {
        _light = new osg::Light;
        _light->setThreadSafeRefUnref(true);
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

    if (_camera.valid())
    {
        _camera->setView(this);
        _camera->setRenderer(createRenderer(camera));
    }
}

void View::updateSlaves()
{
    for(unsigned int i=0; i<_slaves.size(); ++i)
    {
        Slave& slave = _slaves[i];
        slave.updateSlave(*this);
    }
}

void View::Slave::updateSlaveImplementation(View& view)
{
    if (!view.getCamera()) return;

    if (_camera->getReferenceFrame()==osg::Transform::RELATIVE_RF)
    {
        _camera->setProjectionMatrix(view.getCamera()->getProjectionMatrix() * _projectionOffset);
        _camera->setViewMatrix(view.getCamera()->getViewMatrix() * _viewOffset);
    }

    _camera->inheritCullSettings(*(view.getCamera()), _camera->getInheritanceMask());
}

bool View::addSlave(osg::Camera* camera, const osg::Matrix& projectionOffset, const osg::Matrix& viewOffset, bool useMastersSceneData)
{
    if (!camera) return false;

    camera->setView(this);


    if (useMastersSceneData)
    {
        camera->removeChildren(0,camera->getNumChildren());

        if (_camera.valid())
        {
            for(unsigned int i=0; i<_camera->getNumChildren(); ++i)
            {
                camera->addChild(_camera->getChild(i));
            }
        }
    }

    unsigned int i = _slaves.size();

    _slaves.push_back(Slave(camera, projectionOffset, viewOffset, useMastersSceneData));
    _slaves[i].updateSlave(*this);

    camera->setRenderer(createRenderer(camera));

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


View::Slave * View::findSlaveForCamera(osg::Camera* camera)
{
    unsigned int i = findSlaveIndexForCamera(camera);

    if (i >= getNumSlaves()) return (NULL);

    return &(_slaves[i]);
}

unsigned int View::findSlaveIndexForCamera(osg::Camera* camera) const
{
    if (_camera == camera) return _slaves.size();

    for(unsigned int i=0; i<_slaves.size(); ++i)
    {
        if (_slaves[i]._camera == camera) return (i);
    }

    return _slaves.size();
}

void View::resizeGLObjectBuffers(unsigned int maxSize)
{
    if (_camera) _camera->resizeGLObjectBuffers(maxSize);

    for(Slaves::iterator itr = _slaves.begin();
        itr != _slaves.end();
        ++itr)
    {
        if (itr->_camera.valid()) itr->_camera->resizeGLObjectBuffers(maxSize);
    }
}

void View::releaseGLObjects(osg::State* state) const
{
    if (_camera) _camera->releaseGLObjects(state);

    for(Slaves::const_iterator itr = _slaves.begin();
        itr != _slaves.end();
        ++itr)
    {
        if (itr->_camera.valid()) itr->_camera->releaseGLObjects(state);
    }
}
