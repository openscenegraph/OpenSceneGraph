/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2018 Robert Osfield
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

#include <osgPresentation/Cursor>

#include <osgUtil/CullVisitor>
#include <osgGA/EventVisitor>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osgViewer/View>
#include <osg/Texture2D>
#include <osg/io_utils>

using namespace osgPresentation;

Cursor::Cursor():
    _size(0.05f),
    _cursorDirty(true)
{
    setDataVariance(osg::Object::DYNAMIC);
    setCullingActive(false);
    setNumChildrenRequiringEventTraversal(1);
    setNumChildrenRequiringUpdateTraversal(1);
}

Cursor::Cursor(const std::string& filename, float size):
    _cursorDirty(true)
{
    setDataVariance(osg::Object::DYNAMIC);
    setCullingActive(false);
    setNumChildrenRequiringEventTraversal(1);
    setNumChildrenRequiringUpdateTraversal(1);

    setFilename(filename);
    setSize(size);
}

Cursor::Cursor(const Cursor& rhs,const osg::CopyOp& copyop):
    osg::Group(rhs, copyop),
    _filename(rhs._filename),
    _size(rhs._size),
    _cursorDirty(true)
{
    setDataVariance(osg::Object::DYNAMIC);
    setCullingActive(false);
}

Cursor::~Cursor()
{
}

void Cursor::initializeCursor()
{
    if (!_cursorDirty) return;
    if (_filename.empty()) return;

    removeChildren(0, getNumChildren()-1);

    OSG_INFO<<"Curosr::initializeCursor()"<<std::endl;
    _cursorDirty = false;

    _transform = new osg::AutoTransform;

    _transform->setAutoRotateMode(osg::AutoTransform::ROTATE_TO_CAMERA);
    _transform->setAutoScaleToScreen(true);

    osg::ref_ptr<osg::Geode> geode = new osg::Geode;


    osg::ref_ptr<osg::Image> image = osgDB::readRefImageFile(osgDB::findDataFile(_filename));
    osg::ref_ptr<osg::Texture2D> texture = (image.valid()) ? new osg::Texture2D(image.get()) : 0;

    // full cursor
    {
        osg::ref_ptr<osg::Geometry> geom = osg::createTexturedQuadGeometry(osg::Vec3(-_size*0.5f,-_size*0.5f,0.0f),osg::Vec3(_size,0.0f,0.0f),osg::Vec3(0.0f,_size,0.0f));
        geode->addDrawable(geom.get());

        osg::StateSet* stateset = geom->getOrCreateStateSet();
        stateset->setMode(GL_BLEND,osg::StateAttribute::ON|osg::StateAttribute::PROTECTED);
        stateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
        stateset->setRenderBinDetails(1001, "DepthSortedBin");

        if (texture.valid()) stateset->setTextureAttributeAndModes(0, texture.get(), osg::StateAttribute::ON|osg::StateAttribute::PROTECTED);
    }

    {
        osg::ref_ptr<osg::Geometry> geom = osg::createTexturedQuadGeometry(osg::Vec3(-_size*0.5f,-_size*0.5f,0.0f),osg::Vec3(_size,0.0f,0.0f),osg::Vec3(0.0f,_size,0.0f));
        geode->addDrawable(geom.get());

        osg::Vec4Array* colors = new osg::Vec4Array;
        colors->push_back(osg::Vec4(1.0f,1.0f,1.0f,0.25f));
        geom->setColorArray(colors, osg::Array::BIND_OVERALL);

        osg::StateSet* stateset = geom->getOrCreateStateSet();
        stateset->setMode(GL_BLEND,osg::StateAttribute::ON|osg::StateAttribute::PROTECTED);
        stateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
        stateset->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);
        stateset->setRenderBinDetails(1000, "DepthSortedBin");

        if (texture.valid()) stateset->setTextureAttributeAndModes(0, texture.get(), osg::StateAttribute::ON|osg::StateAttribute::PROTECTED);
    }



    _transform->addChild(geode.get());

    addChild(_transform.get());
}

void Cursor::updatePosition()
{
    if (!_camera)
    {
        OSG_INFO<<"Cursor::updatePosition() : Update position failed, no camera assigned"<<std::endl;
        return;
    }

    double distance = 1.0f;

    osgViewer::View* view = dynamic_cast<osgViewer::View*>(_camera->getView());
    if (view)
    {
        osg::DisplaySettings* ds = (view->getDisplaySettings()!=0) ? view->getDisplaySettings() : osg::DisplaySettings::instance().get();

        double sd = ds->getScreenDistance();
        double fusionDistance = sd;
        switch(view->getFusionDistanceMode())
        {
            case(osgUtil::SceneView::USE_FUSION_DISTANCE_VALUE):
                fusionDistance = view->getFusionDistanceValue();
                break;
            case(osgUtil::SceneView::PROPORTIONAL_TO_SCREEN_DISTANCE):
                fusionDistance *= view->getFusionDistanceValue();
                break;
        }

        distance = fusionDistance;
    }

    osg::Matrix VP =  _camera->getViewMatrix() * _camera->getProjectionMatrix();

    osg::Matrix inverse_VP;
    inverse_VP.invert(VP);

    osg::Vec3d eye(0.0,0.0,0.0);
    osg::Vec3d farpoint(_cursorXY.x(), _cursorXY.y(), 1.0);

    osg::Vec3d eye_world = eye * osg::Matrix::inverse(_camera->getViewMatrix());
    osg::Vec3d farpoint_world = farpoint * inverse_VP;

    osg::Vec3d normal = farpoint_world-eye_world;
    normal.normalize();

    osg::Vec3d cursorPosition = eye_world + normal * distance;
    _transform->setPosition(cursorPosition);
}


void Cursor::traverse(osg::NodeVisitor& nv)
{
    if (nv.getVisitorType()==osg::NodeVisitor::UPDATE_VISITOR)
    {
        if (_cursorDirty) initializeCursor();

        // updatePosition();

        // traverse the subgraph
        Group::traverse(nv);
    }
    else if (nv.getVisitorType()==osg::NodeVisitor::EVENT_VISITOR)
    {
        osgGA::EventVisitor* ev = nv.asEventVisitor();
        if (!ev) return;

        osgGA::EventQueue::Events& events = ev->getEvents();
        for(osgGA::EventQueue::Events::iterator itr = events.begin();
            itr != events.end();
            ++itr)
        {
            osgGA::GUIEventAdapter* event = (*itr)->asGUIEventAdapter();
            if (!event) continue;

            switch(event->getEventType())
            {
                case(osgGA::GUIEventAdapter::PUSH):
                case(osgGA::GUIEventAdapter::RELEASE):
                case(osgGA::GUIEventAdapter::MOVE):
                case(osgGA::GUIEventAdapter::DRAG):
                {
                    if (event->getNumPointerData()>=1)
                    {
                        const osgGA::PointerData* pd = event->getPointerData(event->getNumPointerData()-1);
                        osg::Camera* camera = pd->object.valid() ? pd->object->asCamera() : 0;

                        _cursorXY.set(pd->getXnormalized(), pd->getYnormalized());
                        _camera = camera;
                    }
                    else
                    {
                        osgViewer::View* view = dynamic_cast<osgViewer::View*>(ev->getActionAdapter());
                        osg::Camera* camera = (view!=0) ? view->getCamera() : 0;

                        _cursorXY.set(event->getXnormalized(), event->getYnormalized());
                        _camera = camera;
                    }
                    break;
                }
                case(osgGA::GUIEventAdapter::KEYDOWN):
                {
                    if (event->getKey()=='c')
                    {
                        for(unsigned int i=0; i< getNumChildren(); ++i)
                        {
                            osg::Node* node = getChild(i);
                            node->setNodeMask(node->getNodeMask()!=0 ? 0 : 0xffffff);
                        }
                    }
                    break;
                }
                default:
                    break;

            }
        }
        Group::traverse(nv);
    }
    else  if (nv.getVisitorType()==osg::NodeVisitor::CULL_VISITOR)
    {
#if 0
        if (!_camera)
        {
            osgUtil::CullVisitor* cv = nv.asCullVisitor();
            if (cv)
            {
                _camera = cv->getCurrentCamera();
            }
        }
#endif
        if (_camera.valid())
        {
            updatePosition();
            Group::traverse(nv);
        }
    }
}
