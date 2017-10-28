/* OpenSceneGraph example, osgemscripten.
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

#ifndef OSGEMSCRIPTEN_OSGEMSCRIPTEN_H
#define OSGEMSCRIPTEN_OSGEMSCRIPTEN_H

#include "functions.h"

#include <osgDB/ReadFile>
#include <osgViewer/Viewer>

#include <osgGA/TrackballManipulator>

// VBO setup visitor.
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/NodeVisitor>

// Initialize OSG plugins statically.
USE_OSGPLUGIN(osg2)
USE_SERIALIZER_WRAPPER_LIBRARY(osg)

// This class prints OpenSceneGraph notifications to console.
class Logger : public osg::NotifyHandler
{
    public:
        Logger() { }
        virtual ~Logger() { }

        // Override NotifyHandler::notify() to receive OpenSceneGraph notifications.
        void notify(osg::NotifySeverity severity, const char *message)
        {
            printf("OSG/%s %s", logLevelToString(severity).c_str(), message);
        }
};

// This class forces the use of VBO.
class VBOSetupVisitor : public osg::NodeVisitor
{
    public:
        VBOSetupVisitor() :
            osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) { }
        virtual void apply(osg::Geode &geode)
        {
            for (unsigned int i = 0; i < geode.getNumDrawables(); ++i)
            {
                osg::Geometry *geom =
                    dynamic_cast<osg::Geometry*>(geode.getDrawable(i));
                if (geom)
                {
                    geom->setUseVertexBufferObjects(true);
                }
            }
            NodeVisitor::apply(geode);
        }
};

class Application
{
    public:
        Application()
        {
            setupLogging();
            setupRendering();
        }
        ~Application()
        {
            tearDownLogging();
            tearDownRendering();
        }

        bool handleEvent(SDL_Event &e)
        {
            osgViewer::GraphicsWindow *gw =
                dynamic_cast<osgViewer::GraphicsWindow *>(
                    mViewer->getCamera()->getGraphicsContext());
            if (!gw)
            {
                return false;
            }
            osgGA::EventQueue &queue = *(gw->getEventQueue());
            switch (e.type)
            {
                case SDL_MOUSEMOTION:
                    queue.mouseMotion(e.motion.x, e.motion.y);
                    return true;
                case SDL_MOUSEBUTTONDOWN:
                    queue.mouseButtonPress(e.button.x, e.button.y, e.button.button);
                    printf("OSGWeb. Application. Mouse button down\n");
                    return true;
                case SDL_MOUSEBUTTONUP:
                    queue.mouseButtonRelease(e.button.x, e.button.y, e.button.button);
                    printf("OSGWeb. Application. Mouse button up\n");
                    return true;
                default:
                    break;
            }
            return false;
        }
        void loadScene(const std::string &fileName)
        {
            // Load scene.
            osg::Node *scene = osgDB::readNodeFile(fileName);
            if (!scene)
            {
                printf("Could not load scene\n");
                return;
            }
            // Use VBO and EBO instead of display lists. CRITICAL for Emscripten
            // to skip FULL_ES2 emulation flag.
            VBOSetupVisitor vbo;
            scene->accept(vbo);
            // Load shaders.
            osg::Program *prog = createShaderProgram(shaderVertex, shaderFragment);
            // Apply shaders.
            scene->getOrCreateStateSet()->setAttribute(prog);
            // Set scene.
            mViewer->setSceneData(scene);
        }
        void setupWindow(int width, int height)
        {
            mViewer->setUpViewerAsEmbeddedInWindow(0, 0, width, height);
        }
        void frame()
        {
            mViewer->frame();
        }

    private:
        void setupLogging()
        {
            // Create custom logger.
            mLogger = new Logger;
            // Provide the logger to OpenSceneGraph.
            osg::setNotifyHandler(mLogger);
            // Only accept notifications of Info level or higher
            // like warnings and errors.
            osg::setNotifyLevel(osg::INFO);
        }
        void setupRendering()
        {
            // Create OpenSceneGraph viewer.
            mViewer = new osgViewer::Viewer;
            // Use single thread: CRITICAL for Emscripten.
            mViewer->setThreadingModel(osgViewer::ViewerBase::SingleThreaded);
            // Create manipulator: CRITICAL for Emscripten.
            mViewer->setCameraManipulator(new osgGA::TrackballManipulator);
        }
        void tearDownLogging()
        {
            // Remove the logger from OpenSceneGraph.
            // This also destroys the logger: no need to deallocate it manually.
            osg::setNotifyHandler(0);
        }
        void tearDownRendering()
        {
            delete mViewer;
        }

    private:
        Logger *mLogger;
        osgViewer::Viewer *mViewer;
};

#endif // OSGEMSCRIPTEN_OSGEMSCRIPTEN_H
