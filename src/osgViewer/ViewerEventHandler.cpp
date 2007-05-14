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

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandler>

namespace osgViewer
{

/*
** ScreenHandler
*/

ScreenHandler::ScreenHandler() :
_keyEventToggleFullscreen('f'),
_toggleFullscreen(true),
_keyEventWindowedResolutionUp('>'),
_keyEventWindowedResolutionDown('<'),
_changeWindowedResolution(true),
_currentResolutionIndex(-1)
{
    _resolutionList.push_back(osg::Vec2(640, 480));
    _resolutionList.push_back(osg::Vec2(800, 600));
    _resolutionList.push_back(osg::Vec2(1024, 768));
    _resolutionList.push_back(osg::Vec2(1152, 864));
    _resolutionList.push_back(osg::Vec2(1280, 720));
    _resolutionList.push_back(osg::Vec2(1280, 768));
    _resolutionList.push_back(osg::Vec2(1280, 1024));
    _resolutionList.push_back(osg::Vec2(1440, 900));
    _resolutionList.push_back(osg::Vec2(1400, 1050));
    _resolutionList.push_back(osg::Vec2(1600, 900));
    _resolutionList.push_back(osg::Vec2(1600, 1024));
    _resolutionList.push_back(osg::Vec2(1600, 1200));
    _resolutionList.push_back(osg::Vec2(1680, 1050));
    _resolutionList.push_back(osg::Vec2(1920, 1080));
    _resolutionList.push_back(osg::Vec2(1920, 1200));
    _resolutionList.push_back(osg::Vec2(2048, 1536));
    _resolutionList.push_back(osg::Vec2(2560, 2048));
    _resolutionList.push_back(osg::Vec2(3200, 2400));
    _resolutionList.push_back(osg::Vec2(3840, 2400));
}

void ScreenHandler::getUsage(osg::ApplicationUsage &usage) const
{
    usage.addKeyboardMouseBinding("f", "Toggle full screen.");
    usage.addKeyboardMouseBinding(">", "Increase the screen resolution (in windowed mode).");
    usage.addKeyboardMouseBinding("<", "Decrease the screen resolution (in windowed mode).");
}

bool ScreenHandler::handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
{
    osgViewer::Viewer    *viewer = dynamic_cast<osgViewer::Viewer *>(&aa);

    if (viewer == NULL)
    {
        return (false);
    }

    switch(ea.getEventType())
    {
    case(osgGA::GUIEventAdapter::KEYUP):
        {
            if (_toggleFullscreen == true && ea.getKey() == _keyEventToggleFullscreen)
            {
                osgViewer::Viewer::Windows    windows;

                viewer->getWindows(windows);
                for(osgViewer::Viewer::Windows::iterator itr = windows.begin();
                    itr != windows.end();
                    ++itr)
                {
                    this->toggleFullscreen(*itr);
                }
                return (true);
            }
            else if (_changeWindowedResolution == true && ea.getKey() == _keyEventWindowedResolutionUp)
            {
                // Increase resolution
                osgViewer::Viewer::Windows    windows;

                viewer->getWindows(windows);
                for(osgViewer::Viewer::Windows::iterator itr = windows.begin();
                    itr != windows.end();
                    ++itr)
                {
                    this->changeWindowedResolution(*itr, true);
                }
                return (true);
            }
            else if (_changeWindowedResolution == true && ea.getKey() == _keyEventWindowedResolutionDown)
            {
                // Decrease resolution
                osgViewer::Viewer::Windows    windows;

                viewer->getWindows(windows);
                for(osgViewer::Viewer::Windows::iterator itr = windows.begin();
                    itr != windows.end();
                    ++itr)
                {
                    this->changeWindowedResolution(*itr, false);
                }
                return (true);
            }
            break;
        }
    default:
        break;
    }
    return (false);
}

void ScreenHandler::toggleFullscreen(osgViewer::GraphicsWindow *window)
{
    osg::GraphicsContext::WindowingSystemInterface    *wsi = osg::GraphicsContext::getWindowingSystemInterface();

    if (wsi == NULL) 
    {
        osg::notify(osg::NOTICE) << "Error, no WindowSystemInterface available, cannot toggle window fullscreen." << std::endl;
        return;
    }

    unsigned int    screenWidth;
    unsigned int    screenHeight;

    wsi->getScreenResolution(*(window->getTraits()), screenWidth, screenHeight);

    int x;
    int y;
    int width;
    int    height;

    window->getWindowRectangle(x, y, width, height);

    bool    isFullScreen = x == 0 && y == 0 && width == screenWidth && height == screenHeight;

    if (isFullScreen)
    {
        osg::Vec2    resolution;

        if (_currentResolutionIndex == -1)
        {
            _currentResolutionIndex = this->getNearestResolution(screenWidth, screenHeight, screenWidth / 2, screenHeight / 2);
        }
        resolution = _resolutionList[_currentResolutionIndex];
        window->setWindowDecoration(true);
        window->setWindowRectangle((screenWidth - (int)resolution.x()) / 2, (screenHeight - (int)resolution.y()) / 2, (int)resolution.x(), (int)resolution.y());
        osg::notify(osg::INFO) << "Screen resolution = " << (int)resolution.x() << "x" << (int)resolution.y() << std::endl;  
    }
    else
    {
        window->setWindowDecoration(false);
        window->setWindowRectangle(0, 0, screenWidth, screenHeight);
    }

    window->grabFocusIfPointerInWindow();
}

void ScreenHandler::changeWindowedResolution(osgViewer::GraphicsWindow *window, bool increase)
{
    osg::GraphicsContext::WindowingSystemInterface    *wsi = osg::GraphicsContext::getWindowingSystemInterface();

    if (wsi == NULL) 
    {
        osg::notify(osg::NOTICE) << "Error, no WindowSystemInterface available, cannot toggle window fullscreen." << std::endl;
        return;
    }

    unsigned int    screenWidth;
    unsigned int    screenHeight;

    wsi->getScreenResolution(*(window->getTraits()), screenWidth, screenHeight);

    int x;
    int y;
    int width;
    int    height;

    window->getWindowRectangle(x, y, width, height);

    bool    isFullScreen = x == 0 && y == 0 && width == screenWidth && height == screenHeight;

    if (window->getWindowDecoration() == true || isFullScreen == false)
    {
        osg::Vec2    resolution;

        if (_currentResolutionIndex == -1)
        {
            _currentResolutionIndex = this->getNearestResolution(screenWidth, screenHeight, width, height);
        }

        if (increase == true)
        {
            // Find the next resolution
            for (int i = _currentResolutionIndex + 1; i < (int)_resolutionList.size(); ++i)
            {
                if ((unsigned int)_resolutionList[i].x() <= screenWidth && (unsigned int)_resolutionList[i].y() <= screenHeight)
                {
                    _currentResolutionIndex = i;
                    break;
                }
            }
        }
        else
        {
            // Find the previous resolution
            for (int i = _currentResolutionIndex - 1; i >= 0; --i)
            {
                if ((unsigned int)_resolutionList[i].x() <= screenWidth && (unsigned int)_resolutionList[i].y() <= screenHeight)
                {
                    _currentResolutionIndex = i;
                    break;
                }
            }
        }

        resolution = _resolutionList[_currentResolutionIndex];
        window->setWindowDecoration(true);
        window->setWindowRectangle((screenWidth - (int)resolution.x()) / 2, (screenHeight - (int)resolution.y()) / 2, (int)resolution.x(), (int)resolution.y());
        osg::notify(osg::INFO) << "Screen resolution = " << (int)resolution.x() << "x" << (int)resolution.y() << std::endl;  

        window->grabFocusIfPointerInWindow();
    }
}

unsigned int ScreenHandler::getNearestResolution(int screenWidth, int screenHeight, int width, int height) const
{
    unsigned int    position = 0;
    unsigned int    result = 0;
    int                delta = INT_MAX;

    for (std::vector<osg::Vec2>::const_iterator it = _resolutionList.begin();
        it != _resolutionList.end();
        ++it, ++position)
    {
        if ((int)it->x() <= screenWidth && (int)it->y() <= screenHeight)
        {
            int tmp = static_cast<int>(osg::absolute((width * height) - (it->x() * it->y())));

            if (tmp < delta)
            {
                delta = tmp;
                result = position;
            }
        }
    }
    return (result);
}

/*
** ThreadingHandler
*/

ThreadingHandler::ThreadingHandler() :
_keyEventChangeThreadingModel('m'),
_changeThreadingModel(true),
_keyEventChangeEndBarrierPosition('e'),
_changeEndBarrierPosition(true)
{
    _tickOrLastKeyPress = osg::Timer::instance()->tick();
}

void ThreadingHandler::getUsage(osg::ApplicationUsage &usage) const
{
    usage.addKeyboardMouseBinding("m", "Toggle threading model.");
    usage.addKeyboardMouseBinding("e", "Toggle the placement of the end of frame barrier.");
}

bool ThreadingHandler::handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
{
    osgViewer::Viewer    *viewer = dynamic_cast<osgViewer::Viewer*>(&aa);

    if (viewer == NULL)
    {
        return (false);
    }

    switch(ea.getEventType())
    {
    case(osgGA::GUIEventAdapter::KEYUP):
        {
            double    delta = osg::Timer::instance()->delta_s(_tickOrLastKeyPress, osg::Timer::instance()->tick());

            if (_changeThreadingModel == true && ea.getKey() == _keyEventChangeThreadingModel && delta > 1.0)
            {
                _tickOrLastKeyPress = osg::Timer::instance()->tick();

                switch(viewer->getThreadingModel())
                {
                case(osgViewer::Viewer::SingleThreaded):
                    viewer->setThreadingModel(osgViewer::Viewer::CullDrawThreadPerContext);
                    osg::notify(osg::NOTICE)<<"Threading model 'CullDrawThreadPerContext' selected."<<std::endl;
                    break;
                case(osgViewer::Viewer::CullDrawThreadPerContext):
                    viewer->setThreadingModel(osgViewer::Viewer::DrawThreadPerContext);
                    osg::notify(osg::NOTICE)<<"Threading model 'DrawThreadPerContext' selected."<<std::endl;
                    break;
                case(osgViewer::Viewer::DrawThreadPerContext):
                    viewer->setThreadingModel(osgViewer::Viewer::CullThreadPerCameraDrawThreadPerContext);
                    osg::notify(osg::NOTICE)<<"Threading model 'CullThreadPerCameraDrawThreadPerContext' selected."<<std::endl;
                    break;
                case(osgViewer::Viewer::CullThreadPerCameraDrawThreadPerContext):
                    viewer->setThreadingModel(osgViewer::Viewer::SingleThreaded);
                    osg::notify(osg::NOTICE)<<"Threading model 'SingleThreaded' selected."<<std::endl;
                    break;
                case(osgViewer::Viewer::AutomaticSelection):
                    viewer->setThreadingModel(viewer->suggestBestThreadingModel());
                    osg::notify(osg::NOTICE)<<"Threading model 'AutomaticSelection' selected."<<std::endl;
                    break;
                }
                return (true);
            }
            if (_changeEndBarrierPosition == true && ea.getKey() == _keyEventChangeEndBarrierPosition)
            {
                switch(viewer->getEndBarrierPosition())
                {
                case(osgViewer::Viewer::BeforeSwapBuffers):
                    viewer->setEndBarrierPosition(osgViewer::Viewer::AfterSwapBuffers);
                    osg::notify(osg::NOTICE)<<"Threading model 'AfterSwapBuffers' selected."<<std::endl;
                    break;
                case(osgViewer::Viewer::AfterSwapBuffers):
                    viewer->setEndBarrierPosition(osgViewer::Viewer::BeforeSwapBuffers);
                    osg::notify(osg::NOTICE)<<"Threading model 'BeforeSwapBuffers' selected."<<std::endl;
                    break;
                }
                return (true);
            }
            break;
        }
    default:
        break;
    }
    return (false);
}

}
