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

#include <osgViewer/ViewerEventHandlers>
#include <osgViewer/Renderer>

#include <osg/PolygonMode>

#include <osgText/Text>

using namespace osgViewer;

HelpHandler::HelpHandler(osg::ApplicationUsage* au):
    _applicationUsage(au),
    _keyEventTogglesOnScreenHelp('h'),
    _helpEnabled(false),
    _initialized(false)
{
    _camera = new osg::Camera;
    _camera->setRenderer(new Renderer(_camera.get()));
    _camera->setRenderOrder(osg::Camera::POST_RENDER, 11);
}


void HelpHandler::reset()
{
    _initialized = false;
    _camera->setGraphicsContext(0);
}

bool HelpHandler::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
    osgViewer::View* view = dynamic_cast<osgViewer::View*>(&aa);
    if (!view) return false;
    
    osgViewer::ViewerBase* viewer = view->getViewerBase();
    if (!viewer) return false;
    
    if (ea.getHandled()) return false;

    switch(ea.getEventType())
    {
        case(osgGA::GUIEventAdapter::KEYDOWN):
        {
            if (ea.getKey()==_keyEventTogglesOnScreenHelp)
            {
                if (!_initialized)
                {
                    setUpHUDCamera(viewer);
                    setUpScene(viewer);
                }

                _helpEnabled = !_helpEnabled;

                if (_helpEnabled)
                {
                    _camera->setNodeMask(0xffffffff);
                }
                else
                {
                    _camera->setNodeMask(0);
                }
                return true;
            }
        }
        default: break;
    }

    return false;

}

void HelpHandler::setUpHUDCamera(osgViewer::ViewerBase* viewer)
{
    osgViewer::GraphicsWindow* window = dynamic_cast<osgViewer::GraphicsWindow*>(_camera->getGraphicsContext());

    if (!window)
    {    
        osgViewer::Viewer::Windows windows;
        viewer->getWindows(windows);

        if (windows.empty()) return;

        window = windows.front();

        _camera->setGraphicsContext(window);
    }

    _camera->setGraphicsContext(window);
    _camera->setViewport(0, 0, window->getTraits()->width, window->getTraits()->height);

    _camera->setProjectionMatrix(osg::Matrix::ortho2D(0,1280,0,1024));
    _camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    _camera->setViewMatrix(osg::Matrix::identity());

    // only clear the depth buffer
    _camera->setClearMask(0);

    _initialized = true;
}

void HelpHandler::setUpScene(osgViewer::ViewerBase* viewer)
{
    _switch = new osg::Switch;

    _camera->addChild(_switch.get());

    osg::StateSet* stateset = _switch->getOrCreateStateSet();
    stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
    stateset->setMode(GL_BLEND,osg::StateAttribute::ON);
    stateset->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);
    stateset->setAttribute(new osg::PolygonMode(), osg::StateAttribute::PROTECTED);

    std::string font("fonts/arial.ttf");

    if (!_applicationUsage) setApplicationUsage(new osg::ApplicationUsage());

    viewer->getUsage(*_applicationUsage);

    float leftPos = 10.0f;
    float startDescription = 200.0f;
    float characterSize = 20.0f;

    osg::Vec3 pos(leftPos,1000.0f,0.0f);
    osg::Vec4 color(1.0f,1.0f,1.0f,1.0f);

    osg::Geode* geode = new osg::Geode();
    _switch->addChild(geode, true);

    // application description
    if (!_applicationUsage->getDescription().empty())
    {

        osg::ref_ptr<osgText::Text> label = new osgText::Text;
        geode->addDrawable( label.get() );

        label->setColor(color);
        label->setBackdropType(osgText::Text::OUTLINE);
        label->setFont(font);
        label->setCharacterSize(characterSize);
        label->setPosition(pos);
        label->setText(_applicationUsage->getDescription());

        pos.x() = label->getBound().xMax();
        pos.y() -= characterSize*2.5f;
    }

    const osg::ApplicationUsage::UsageMap& keyboardBinding = _applicationUsage->getKeyboardMouseBindings();
    
    for(osg::ApplicationUsage::UsageMap::const_iterator itr = keyboardBinding.begin();
        itr != keyboardBinding.end();
        ++itr)
    {
        pos.x() = leftPos;

        osg::ref_ptr<osgText::Text> key = new osgText::Text;
        geode->addDrawable( key.get() );
        key->setColor(color);
        key->setBackdropType(osgText::Text::OUTLINE);
        key->setFont(font);
        key->setCharacterSize(characterSize);
        key->setPosition(pos);
        key->setText(itr->first);

        pos.x() = startDescription;

        osg::ref_ptr<osgText::Text> description = new osgText::Text;
        geode->addDrawable( description.get() );
        description->setColor(color);
        description->setBackdropType(osgText::Text::OUTLINE);
        description->setFont(font);
        description->setCharacterSize(characterSize);
        description->setPosition(pos);

        description->setText(itr->second);

        pos.y() -= characterSize*1.5f;

    }

    osg::BoundingBox bb = geode->getBoundingBox();
    if (bb.valid())
    {
        float width = bb.xMax() - bb.xMin();
        float height = bb.yMax() - bb.yMin();
        float ratio = 1.0;
        if (width > 1024.0f) ratio = 1024.0f/width;
        if (height*ratio > 800.0f) ratio = 800.0f/height;
        
        _camera->setViewMatrix(osg::Matrix::translate(-bb.center()) * 
                               osg::Matrix::scale(ratio,ratio,ratio) * 
                               osg::Matrix::translate(osg::Vec3(640.0f, 520.0f, 0.0f)));
    }
}


void HelpHandler::getUsage(osg::ApplicationUsage& usage) const
{
    usage.addKeyboardMouseBinding("h","Onscreen help.");
}
