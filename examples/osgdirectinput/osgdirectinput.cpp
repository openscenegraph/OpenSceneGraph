/* OpenSceneGraph example, osgdirectinput.
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
*  THE SOFTWARE.
*/

#include <osgDB/ReadFile>
#include <osgGA/StateSetManipulator>
#include <osgViewer/api/Win32/GraphicsWindowWin32>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <iostream>
#include "DirectInputRegistry"

class CustomViewer : public osgViewer::Viewer
{
public:
    CustomViewer() : osgViewer::Viewer() {}
    virtual ~CustomViewer() {}
    
    virtual void eventTraversal()
    {
        DirectInputRegistry::instance()->updateState( _eventQueue.get() );
        osgViewer::Viewer::eventTraversal();
    }
    
protected:
    virtual void viewerInit()
    {
        osgViewer::GraphicsWindowWin32* windowWin32 =
            dynamic_cast<osgViewer::GraphicsWindowWin32*>( _camera->getGraphicsContext() );
        if ( windowWin32 )
        {
            HWND hwnd = windowWin32->getHWND();
            DirectInputRegistry::instance()->initKeyboard( hwnd );
            //DirectInputRegistry::instance()->initMouse( hwnd );
            DirectInputRegistry::instance()->initJoystick( hwnd );
        }
        osgViewer::Viewer::viewerInit();
    }
};

class JoystickHandler : public osgGA::GUIEventHandler
{
public:
    JoystickHandler() {}
    
    bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
    {
        switch ( ea.getEventType() )
        {
        case osgGA::GUIEventAdapter::KEYDOWN:
            std::cout << "*** Key 0x" << std::hex << ea.getKey() << std::dec << " down ***" << std::endl;
            break;
        case osgGA::GUIEventAdapter::KEYUP:
            std::cout << "*** Key 0x" << std::hex << ea.getKey() << std::dec << " up ***" << std::endl;
            break;
        case osgGA::GUIEventAdapter::USER:
            {
                const JoystickEvent* event = dynamic_cast<const JoystickEvent*>( ea.getUserData() );
                if ( !event ) break;
                
                const DIJOYSTATE2& js = event->_js;
                for ( unsigned int i=0; i<128; ++i )
                {
                    if ( js.rgbButtons[i] )
                        std::cout << "*** Joystick Btn" << i << " = " << (int)js.rgbButtons[i] << std::endl;
                }
                
                if ( js.lX==0x0000 ) std::cout << "*** Joystick X-" << std::endl;
                else if ( js.lX==0xffff ) std::cout << "*** Joystick X+" << std::endl;
                
                if ( js.lY==0 ) std::cout << "*** Joystick Y-" << std::endl;
                else if ( js.lY==0xffff ) std::cout << "*** Joystick Y+" << std::endl;
            }
            return true;
        default:
            break;
        }
        return false;
    }
};

int main( int argc, char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );
    osg::Node* model = osgDB::readNodeFiles( arguments );
    if ( !model ) model = osgDB::readNodeFile( "cow.osgt" );
    if ( !model ) 
    {
        std::cout << arguments.getApplicationName() <<": No data loaded" << std::endl;
        return 1;
    }
    
    CustomViewer viewer;
    viewer.addEventHandler( new JoystickHandler );
    viewer.addEventHandler( new osgViewer::StatsHandler );
    viewer.addEventHandler( new osgViewer::WindowSizeHandler );
    viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );
    viewer.setSceneData( model );
    viewer.setUpViewInWindow( 250, 50, 800, 600 );
    return viewer.run();
}
