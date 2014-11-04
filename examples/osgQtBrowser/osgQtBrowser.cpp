/* OpenSceneGraph example, osgcompositeviewer.
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

#include <iostream>

#include <osg/Notify>
#include <osg/io_utils>

#include <osg/ArgumentParser>
#include <osgDB/WriteFile>
#include <osgGA/TrackballManipulator>
#include <osgGA/StateSetManipulator>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgWidget/Browser>

#include <QtGlobal>
#if QT_VERSION >= 0x050000
# include <QtWebKitWidgets>
#else
# include <QtWebKit>
#endif

#include <QWebSettings>

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QApplication>
#include <QPainter>
#include <QtEvents>

#include <osgQt/QGraphicsViewAdapter>
#include <osgQt/QWebViewImage>


// Thread that runs the viewer's frame loop as we can't run Qt in the background...
class ViewerFrameThread : public OpenThreads::Thread
{
    public:

        ViewerFrameThread(osgViewer::ViewerBase* viewerBase, bool doQApplicationExit):
            _viewerBase(viewerBase),
            _doQApplicationExit(doQApplicationExit) {}

        ~ViewerFrameThread()
        {
            if (isRunning())
            {
                cancel();
                join();
            }
        }

        int cancel()
        {
            _viewerBase->setDone(true);
            return 0;
        }

        void run()
        {
            int result = _viewerBase->run();

            if (_doQApplicationExit) QApplication::exit(result);
        }

        osg::ref_ptr<osgViewer::ViewerBase> _viewerBase;
        bool _doQApplicationExit;
};


int main(int argc, char **argv)
{
    // Qt requires that we construct the global QApplication before creating any widgets.
    QApplication app(argc, argv);

    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    bool useFrameLoopThread = false;
    if (arguments.read("--no-frame-thread")) useFrameLoopThread = false;
    if (arguments.read("--frame-thread")) useFrameLoopThread = true;

    osg::ref_ptr<osgQt::QWebViewImage> image = new osgQt::QWebViewImage;

    if (arguments.argc()>1) image->navigateTo((arguments[1]));
    else image->navigateTo("http://www.youtube.com/");

    osgWidget::GeometryHints hints(osg::Vec3(0.0f,0.0f,0.0f),
                                   osg::Vec3(1.0f,0.0f,0.0f),
                                   osg::Vec3(0.0f,0.0f,1.0f),
                                   osg::Vec4(1.0f,1.0f,1.0f,1.0f),
                                   osgWidget::GeometryHints::RESIZE_HEIGHT_TO_MAINTAINCE_ASPECT_RATIO);


    osg::ref_ptr<osgWidget::Browser> browser = new osgWidget::Browser;
    browser->assign(image.get(), hints);

    // image->focusBrowser(true);

    osg::ref_ptr<osgViewer::Viewer> viewer = new osgViewer::Viewer(arguments);
    viewer->setSceneData(browser.get());
    viewer->setCameraManipulator(new osgGA::TrackballManipulator());
    viewer->addEventHandler(new osgViewer::StatsHandler);
    viewer->addEventHandler(new osgViewer::WindowSizeHandler);

    if (useFrameLoopThread)
    {
        // create a thread to run the viewer's frame loop
        ViewerFrameThread viewerThread(viewer.get(), true);
        viewerThread.startThread();

        // now start the standard Qt event loop, then exists when the viewerThead sends the QApplication::exit() signal.
        return QApplication::exec();

    }
    else
    {
        // run the frame loop, interleaving Qt and the main OSG frame loop
        while(!viewer->done())
        {
            // process Qt events - this handles both events and paints the browser image
            QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

            viewer->frame();
        }

        return 0;
    }
}
