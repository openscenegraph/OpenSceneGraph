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
#include <osg/MatrixTransform>
#include <osg/AutoTransform>
#include <osg/Camera>
#include <osg/TexMat>
#include <osg/TextureRectangle>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgGA/TrackballManipulator>
#include <osgGA/StateSetManipulator>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgWidget/Browser>


//#include <QWebSettings>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QApplication>
#include <QPainter>
#include <QtEvents>
#include <QDialog>
#include <QVBoxLayout>
#include <QMainWindow>
#include <QtOpenGL>

#include <osgQt/QGraphicsViewAdapter>
#include <osgQt/QWebViewImage>
#include <osgQt/QWidgetImage>


// Thread that runs the viewer's frame loop as we can't run Qt in the background...
class ViewerFrameThread : public OpenThreads::Thread
{
    public:

        ViewerFrameThread(osgViewer::ViewerBase* viewerBase, bool doQApplicationExit):
            _viewerBase(viewerBase),
            _doQApplicationExit(doQApplicationExit) {}

        ~ViewerFrameThread()
        {
            cancel();
            while(isRunning())
            {
                OpenThreads::Thread::YieldCurrentThread();
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


class MyPushButton : public QPushButton
{
public:
    MyPushButton(const QString& text) : QPushButton(text) {}

protected:
    virtual void mousePressEvent(QMouseEvent* e)
    {
        bool ok = false;
#if QT_VERSION >= 0x040500
        int val = QInputDialog::getInt(this, "Get integer", "Please enter an integer between 0 and pi", 0, 0, 3, 1, &ok);
#else
        int val = QInputDialog::getInteger(this, "Get integer", "Please enter an integer between 0 and pi", 0, 0, 3, 1, &ok);
#endif
        std::cout << "Ok was " << (ok ? "" : "not") << " pressed, val is " << val << std::endl;
    }
};


//We would need to document the following somewhere in order to guide people on 
//what they need to use...
//
//----------------------------------------------
//There are two angles to consider.
//
//1. If someone wants a widget in their Qt app to be an OSG-rendered scene, they 
//need GraphicsWindowQt (in the osgViewerQtContext example) or QOSGWidget (in the 
//osgViewerQt example). These two allow both OSG and Qt to manage their threads 
//in a way which is optimal to them. We've used QOSGWidget in the past and had 
//trouble when Qt tried to overlay other widgets over the QOSGWidget (since OSG 
//did its rendering independently of Qt, it would overwrite what Qt had drawn). I 
//haven't tried GraphicsWindowQt yet, but I expect since it uses QGLWidget, it 
//will result in Qt knowing when OSG has drawn and be able to do overlays at the 
//right time. Eventually GraphicsWindowQt can be brought into osgViewer I imagine...
//
//2. If someone wants to bring Qt widgets inside their OSG scene (to do HUDs or 
//an interface on a computer screen which is inside the 3D scene, or even 
//floating Qt widgets, for example). That's where QGraphicsViewAdapter + 
//QWidgetImage will be useful.
//----------------------------------------------


int main(int argc, char **argv)
{
    // Qt requires that we construct the global QApplication before creating any widgets.
    QApplication app(argc, argv);

    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    // true = run osgViewer in a separate thread than Qt
    // false = interleave osgViewer and Qt in the main thread
    bool useFrameLoopThread = false;
    if (arguments.read("--no-frame-thread")) useFrameLoopThread = false;
    if (arguments.read("--frame-thread")) useFrameLoopThread = true;

    // true = use QWidgetImage
    // false = use QWebViewImage
    bool useWidgetImage = false;
    if (arguments.read("--useWidgetImage")) useWidgetImage = true;

    // true = use QWebView in a QWidgetImage to compare to QWebViewImage
    // false = make an interesting widget
    bool useBrowser = false;
    if (arguments.read("--useBrowser")) useBrowser = true;

    // true = use a QLabel for text
    // false = use a QTextEdit for text
    // (only applies if useWidgetImage == true and useBrowser == false)
    bool useLabel = false;
    if (arguments.read("--useLabel")) useLabel = true;

    // true = make a Qt window with the same content to compare to 
    // QWebViewImage/QWidgetImage
    // false = use QWebViewImage/QWidgetImage (depending on useWidgetImage)
    bool sanityCheck = false;
    if (arguments.read("--sanityCheck")) sanityCheck = true;

    // Add n floating windows inside the QGraphicsScene.
    int numFloatingWindows = 0;
    while (arguments.read("--numFloatingWindows", numFloatingWindows));

    // true = Qt widgets will be displayed on a quad inside the 3D scene
    // false = Qt widgets will be an overlay over the scene (like a HUD)
    bool inScene = true;
    if (arguments.read("--fullscreen")) { inScene = false; }


    osg::ref_ptr<osg::Group> root = new osg::Group;

    if (!useWidgetImage)
    {
        //-------------------------------------------------------------------
        // QWebViewImage test
        //-------------------------------------------------------------------
        // Note: When the last few issues with QWidgetImage are fixed, 
        // QWebViewImage and this if() {} section can be removed since 
        // QWidgetImage can display a QWebView just like QWebViewImage. Use 
        // --useWidgetImage --useBrowser to see that in action.

        if (!sanityCheck)
        {
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

            root->addChild(browser.get());
        }
        else
        {
            // Sanity check, do the same thing as QGraphicsViewAdapter but in 
            // a separate Qt window.
            QWebPage* webPage = new QWebPage;
            webPage->settings()->setAttribute(QWebSettings::JavascriptEnabled, true);
            webPage->settings()->setAttribute(QWebSettings::PluginsEnabled, true);

            QWebView* webView = new QWebView;
            webView->setPage(webPage);

            if (arguments.argc()>1) webView->load(QUrl(arguments[1]));
            else webView->load(QUrl("http://www.youtube.com/"));

            QGraphicsScene* graphicsScene = new QGraphicsScene;
            graphicsScene->addWidget(webView);

            QGraphicsView* graphicsView = new QGraphicsView;
            graphicsView->setScene(graphicsScene);

            QMainWindow* mainWindow = new QMainWindow;
            //mainWindow->setLayout(new QVBoxLayout);
            mainWindow->setCentralWidget(graphicsView);
            mainWindow->setGeometry(50, 50, 1024, 768);
            mainWindow->show();
            mainWindow->raise();
        }
    }
    else
    {
        //-------------------------------------------------------------------
        // QWidgetImage test
        //-------------------------------------------------------------------
        // QWidgetImage still has some issues, some examples are:
        // 
        // 1. Editing in the QTextEdit doesn't work. Also when started with 
        //    --useBrowser, editing in the search field on YouTube doesn't 
        //    work. But that same search field when using QWebViewImage 
        //    works... And editing in the text field in the pop-up getInteger 
        //    dialog works too. All these cases use QGraphicsViewAdapter 
        //    under the hood, so why do some work and others don't?
        //
        //    a) osgQtBrowser --useWidgetImage [--fullscreen] (optional)
        //    b) Try to click in the QTextEdit and type, or to select text
        //       and drag-and-drop it somewhere else in the QTextEdit. These
        //       don't work.
        //    c) osgQtBrowser --useWidgetImage --sanityCheck
        //    d) Try the operations in b), they all work.
        //    e) osgQtBrowser --useWidgetImage --useBrowser [--fullscreen]
        //    f) Try to click in the search field and type, it doesn't work.
        //    g) osgQtBrowser
        //    h) Try the operation in f), it works.
        //
        // 2. Operations on floating windows (--numFloatingWindows 1 or more). 
        //    Moving by dragging the titlebar, clicking the close button, 
        //    resizing them, none of these work. I wonder if it's because the 
        //    OS manages those functions (they're functions of the window 
        //    decorations) so we need to do something special for that? But 
        //    in --sanityCheck mode they work.
        //
        //    a) osgQtBrowser --useWidgetImage --numFloatingWindows 1 [--fullscreen]
        //    b) Try to drag the floating window, click the close button, or
        //       drag its sides to resize it. None of these work.
        //    c) osgQtBrowser --useWidgetImage --numFloatingWindows 1 --sanityCheck
        //    d) Try the operations in b), all they work.
        //    e) osgQtBrowser --useWidgetImage [--fullscreen]
        //    f) Click the button so that the getInteger() dialog is 
        //       displayed, then try to move that dialog or close it with the 
        //       close button, these don't work.
        //    g) osgQtBrowser --useWidgetImage --sanityCheck
        //    h) Try the operation in f), it works.
        //
        // 3. (Minor) The QGraphicsView's scrollbars don't appear when 
        //    using QWidgetImage or QWebViewImage. QGraphicsView is a 
        //    QAbstractScrollArea and it should display scrollbars as soon as
        //    the scene is too large to fit the view.
        //
        //    a) osgQtBrowser --useWidgetImage --fullscreen
        //    b) Resize the OSG window so it's smaller than the QTextEdit.
        //       Scrollbars should appear but don't.
        //    c) osgQtBrowser --useWidgetImage --sanityCheck
        //    d) Try the operation in b), scrollbars appear. Even if you have 
        //       floating windows (by clicking the button or by adding 
        //       --numFloatingWindows 1) and move them outside the view, 
        //       scrollbars appear too. You can't test that case in OSG for 
        //       now because of problem 2 above, but that's pretty cool.
        //
        // 4. (Minor) In sanity check mode, the widget added to the 
        //    QGraphicsView is centered. With QGraphicsViewAdapter, it is not.
        //
        //    a) osgQtBrowser --useWidgetImage [--fullscreen]
        //    b) The QTextEdit and button are not in the center of the image
        //       generated by the QGraphicsViewAdapter.
        //    c) osgQtBrowser --useWidgetImage --sanityCheck
        //    d) The QTextEdit and button are in the center of the 
        //       QGraphicsView.


        QWidget* widget = 0;
        if (useBrowser)
        {
            QWebPage* webPage = new QWebPage;
            webPage->settings()->setAttribute(QWebSettings::JavascriptEnabled, true);
            webPage->settings()->setAttribute(QWebSettings::PluginsEnabled, true);

            QWebView* webView = new QWebView;
            webView->setPage(webPage);

            if (arguments.argc()>1) webView->load(QUrl(arguments[1]));
            else webView->load(QUrl("http://www.youtube.com/"));

            widget = webView;
        }
        else
        {
            widget = new QWidget;
            widget->setLayout(new QVBoxLayout);

            QString text("Lorem ipsum dolor sit amet, consectetur adipiscing elit. Quisque velit turpis, euismod ac ultrices et, molestie non nisi. Nullam egestas dignissim enim, quis placerat nulla suscipit sed. Donec molestie elementum risus sit amet sodales. Nunc consectetur congue neque, at viverra massa pharetra fringilla. Integer vitae mi sem. Donec dapibus semper elit nec sollicitudin. Vivamus egestas ultricies felis, in mollis mi facilisis quis. Nam suscipit bibendum eros sed cursus. Suspendisse mollis suscipit hendrerit. Etiam magna eros, convallis non congue vel, faucibus ac augue. Integer ante ante, porta in ornare ullamcorper, congue nec nibh. Etiam congue enim vitae enim sollicitudin fringilla. Mauris mattis, urna in fringilla dapibus, ipsum sem feugiat purus, ac hendrerit felis arcu sed sapien. Integer id velit quam, sit amet dignissim tortor. Sed mi tortor, placerat ac luctus id, tincidunt et urna. Nulla sed nunc ante.Sed ut sodales enim. Ut sollicitudin ultricies magna, vel ultricies ante venenatis id. Cras luctus mi in lectus rhoncus malesuada. Sed ac sollicitudin nisi. Nunc venenatis congue quam, et suscipit diam consectetur id. Donec vel enim ac enim elementum bibendum ut quis augue. Nulla posuere suscipit dolor, id convallis tortor congue eu. Vivamus sagittis consectetur dictum. Duis a ante quis dui varius fermentum. In hac habitasse platea dictumst. Nam dapibus dolor eu felis eleifend in scelerisque dolor ultrices. Donec arcu lectus, fringilla ut interdum non, tristique id dolor. Morbi sagittis sagittis volutpat. Pellentesque habitant morbi tristique senectus et netus et malesuada fames ac turpis egestas. Duis venenatis ultrices euismod.Nam sit amet convallis libero. Integer lectus urna, eleifend et sollicitudin non, porttitor vel erat. Vestibulum pulvinar egestas leo, a porttitor turpis ullamcorper et. Vestibulum in ornare turpis. Ut nec libero a sem mattis iaculis quis id purus. Praesent ante neque, dictum vitae pretium vel, iaculis luctus dui. Etiam luctus tellus vel nunc suscipit a ullamcorper nisl semper. Nunc dapibus, eros in sodales dignissim, orci lectus egestas felis, sit amet vehicula tortor dolor eu quam. Vivamus pellentesque convallis quam aliquet pellentesque. Phasellus facilisis arcu ac orci fringilla aliquet. Donec sed euismod augue. Duis eget orci sit amet neque tempor fringilla. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia Curae; In hac habitasse platea dictumst. Duis sollicitudin, lacus ac pellentesque lacinia, lacus magna pulvinar purus, pulvinar porttitor est nibh quis augue.Duis eleifend, massa sit amet mattis fringilla, elit turpis venenatis libero, sed convallis turpis diam sit amet ligula. Morbi non dictum turpis. Integer porttitor condimentum elit, sit amet sagittis nibh ultrices sit amet. Mauris ac arcu augue, id aliquet mauris. Donec ultricies urna id enim accumsan at pharetra dui adipiscing. Nunc luctus rutrum molestie. Curabitur libero ipsum, viverra at pulvinar ut, porttitor et neque. Aliquam sit amet dolor et purus sagittis adipiscing. Nam sit amet hendrerit sem. Etiam varius, ligula non ultricies dignissim, sapien dui commodo urna, eu vehicula enim nunc molestie augue. Fusce euismod, erat vitae pharetra tempor, quam eros tincidunt lorem, ut iaculis ligula erat vitae nibh. Aenean eu ultricies dolor. Curabitur suscipit viverra bibendum.Sed egestas adipiscing mi in egestas. Proin in neque in nibh blandit consequat nec quis tortor. Vestibulum sed interdum justo. Sed volutpat velit vitae elit pulvinar aliquam egestas elit rutrum. Proin lorem nibh, bibendum vitae sollicitudin condimentum, pulvinar ut turpis. Maecenas iaculis, mauris in consequat ultrices, ante erat blandit mi, vel fermentum lorem turpis eget sem. Integer ultrices tristique erat sit amet volutpat. In sit amet diam et nunc congue pellentesque at in dolor. Mauris eget orci orci. Integer posuere augue ornare tortor tempus elementum. Quisque iaculis, nunc ac cursus fringilla, magna elit cursus eros, id feugiat diam eros et tellus. Etiam consectetur ultrices erat quis rhoncus. Mauris eu lacinia neque. Curabitur suscipit feugiat tellus in dictum. Class aptent taciti sociosqu ad litora torquent per conubia nostra, per inceptos himenaeos. Sed aliquam tempus ante a tempor. Praesent viverra erat quis sapien pretium rutrum. Praesent dictum scelerisque venenatis.Proin bibendum lectus eget nisl lacinia porta. Morbi eu erat in sapien malesuada vulputate. Cras non elit quam. Ut dictum urna quis nisl feugiat ac sollicitudin libero luctus. Donec leo mauris, varius at luctus eget, placerat quis arcu. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia Curae; Etiam tristique, mauris ut lacinia elementum, mauris erat consequat massa, ac gravida nisi tellus vitae purus. Curabitur consectetur ultricies commodo. Cras pulvinar orci nec enim adipiscing tristique. Ut ornare orci id est fringilla sit amet blandit libero pellentesque. Vestibulum tincidunt sapien ut enim venenatis vestibulum ultricies ipsum tristique. Mauris tempus eleifend varius. Lorem ipsum dolor sit amet, consectetur adipiscing elit. Suspendisse vitae dui ac quam gravida semper. In ac enim ac ligula rutrum porttitor.Integer dictum sagittis leo, at convallis sapien facilisis eget. Etiam cursus bibendum tortor, faucibus aliquam lectus ullamcorper sed. Nulla pulvinar posuere quam, ut sagittis ligula tincidunt ut. Nulla convallis velit ut enim condimentum pulvinar. Quisque gravida accumsan scelerisque. Proin pellentesque nisi cursus tortor aliquet dapibus. Duis vel eros orci. Sed eget purus ligula. Lorem ipsum dolor sit amet, consectetur adipiscing elit. Donec ullamcorper porta congue. Nunc id velit ut neque malesuada consequat in eu nisi. Nulla facilisi. Quisque pellentesque magna vitae nisl euismod ac accumsan tellus feugiat.Nulla facilisi. Integer quis orci lectus, non aliquam nisi. Vivamus varius porta est, ac porttitor orci blandit mattis. Sed dapibus facilisis dapibus. Duis tincidunt leo ac tortor faucibus hendrerit. Morbi sit amet sapien risus, vel luctus enim. Aliquam sagittis nunc id purus aliquam lobortis. Duis posuere viverra dui, sit amet convallis sem vulputate at. Lorem ipsum dolor sit amet, consectetur adipiscing elit. Quisque pellentesque, lectus id imperdiet commodo, diam diam faucibus lectus, sit amet vestibulum tortor lacus viverra eros.Maecenas nec augue lectus. Duis nec arcu eget lorem tempus sollicitudin suscipit vitae arcu. Nullam vitae mauris lectus. Vivamus id risus neque, dignissim vehicula diam. Cras rhoncus velit sed velit iaculis ac dignissim turpis luctus. Suspendisse potenti. Sed vitae ligula a ligula ornare rutrum sit amet ut quam. Duis tincidunt, nibh vitae iaculis adipiscing, dolor orci cursus arcu, vel congue tortor quam eget arcu. Suspendisse tellus felis, blandit ac accumsan vitae, fringilla id lorem. Duis tempor lorem mollis est congue ut imperdiet velit laoreet. Nullam interdum cursus mollis. Pellentesque non mauris accumsan elit laoreet viverra ut at risus. Proin rutrum sollicitudin sem, vitae ultricies augue sagittis vel. Cras quis vehicula neque. Aliquam erat volutpat. Aliquam erat volutpat. Praesent non est erat, accumsan rutrum lacus. Pellentesque tristique molestie aliquet. Cras ullamcorper facilisis faucibus. In non lorem quis velit lobortis pulvinar.Phasellus non sem ipsum. Praesent ut libero quis turpis viverra semper. Class aptent taciti sociosqu ad litora torquent per conubia nostra, per inceptos himenaeos. In hac habitasse platea dictumst. Donec at velit tellus. Fusce commodo pharetra tincidunt. Proin lacus enim, fringilla a fermentum ut, vestibulum ut nibh. Duis commodo dolor vel felis vehicula at egestas neque bibendum. Phasellus malesuada dictum ante in aliquam. Curabitur interdum semper urna, nec placerat justo gravida in. Praesent quis mauris massa. Pellentesque porttitor lacinia tincidunt. Phasellus egestas viverra elit vel blandit. Sed dapibus nisi et lectus pharetra dignissim. Mauris hendrerit lectus nec purus dapibus condimentum. Sed ac eros nulla. Aenean semper sapien a nibh aliquam lobortis. Aliquam elementum euismod sapien, in dapibus leo dictum et. Pellentesque augue neque, ultricies non viverra eu, tincidunt ac arcu. Morbi ut porttitor lectus.");

            if (useLabel)
            {
                QLabel* label = new QLabel(text);
                label->setWordWrap(true);
                label->setTextInteractionFlags(Qt::TextEditorInteraction);

                QPalette palette = label->palette();
                palette.setColor(QPalette::Highlight, Qt::darkBlue);
                palette.setColor(QPalette::HighlightedText, Qt::white);
                label->setPalette(palette);

                QScrollArea* scrollArea = new QScrollArea;
                scrollArea->setWidget(label);

                widget->layout()->addWidget(scrollArea);
            }
            else
            {
                QTextEdit* textEdit = new QTextEdit(text);
                textEdit->setReadOnly(false);
                textEdit->setTextInteractionFlags(Qt::TextEditable);

                QPalette palette = textEdit->palette();
                palette.setColor(QPalette::Highlight, Qt::darkBlue);
                palette.setColor(QPalette::HighlightedText, Qt::white);
                textEdit->setPalette(palette);

                widget->layout()->addWidget(textEdit);
            }

            QPushButton* button = new MyPushButton("Button");
            widget->layout()->addWidget(button);

            widget->setGeometry(0, 0, 800, 600);
        }

        QGraphicsScene* graphicsScene = 0;

        if (!sanityCheck)
        {
            osg::ref_ptr<osgQt::QWidgetImage> widgetImage = new osgQt::QWidgetImage(widget);
#if (QT_VERSION >= QT_VERSION_CHECK(4, 5, 0))
            widgetImage->getQWidget()->setAttribute(Qt::WA_TranslucentBackground);
#endif
            widgetImage->getQGraphicsViewAdapter()->setBackgroundColor(QColor(0, 0, 0, 0));
            //widgetImage->getQGraphicsViewAdapter()->resize(800, 600);
            graphicsScene = widgetImage->getQGraphicsViewAdapter()->getQGraphicsScene();

            osg::Camera* camera = 0;        // Will stay NULL in the inScene case.
            osg::Geometry* quad = osg::createTexturedQuadGeometry(osg::Vec3(0,0,0), osg::Vec3(1,0,0), osg::Vec3(0,1,0), 1, 1);
            osg::Geode* geode = new osg::Geode;
            geode->addDrawable(quad);

            osg::MatrixTransform* mt = new osg::MatrixTransform;

            osg::Texture2D* texture = new osg::Texture2D(widgetImage.get());
            texture->setResizeNonPowerOfTwoHint(false);
            texture->setFilter(osg::Texture::MIN_FILTER,osg::Texture::LINEAR);
            texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
            texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
            mt->getOrCreateStateSet()->setTextureAttributeAndModes(0, texture, osg::StateAttribute::ON);

            osgViewer::InteractiveImageHandler* handler; 
            if (inScene)
            {
                mt->setMatrix(osg::Matrix::rotate(osg::Vec3(0,1,0), osg::Vec3(0,0,1)));
                mt->addChild(geode);

                handler = new osgViewer::InteractiveImageHandler(widgetImage.get());
            }
            else    // fullscreen
            {
                // The HUD camera's viewport needs to follow the size of the 
                // window. MyInteractiveImageHandler will make sure of this.
                // As for the quad and the camera's projection, setting the 
                // projection resize policy to FIXED takes care of them, so
                // they can stay the same: (0,1,0,1) with a quad that fits.

                // Set the HUD camera's projection and viewport to match the screen.
                camera = new osg::Camera;
                camera->setProjectionResizePolicy(osg::Camera::FIXED);
                camera->setProjectionMatrix(osg::Matrix::ortho2D(0,1,0,1));
                camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
                camera->setViewMatrix(osg::Matrix::identity());
                camera->setClearMask(GL_DEPTH_BUFFER_BIT);
                camera->setRenderOrder(osg::Camera::POST_RENDER);
                camera->addChild(geode);
                camera->setViewport(0, 0, 1024, 768);

                mt->addChild(camera);

                handler = new osgViewer::InteractiveImageHandler(widgetImage.get(), texture, camera);
            }

            mt->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
            mt->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);
            mt->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
            mt->getOrCreateStateSet()->setAttribute(new osg::Program);

            osg::Group* overlay = new osg::Group;
            overlay->addChild(mt);

            root->addChild(overlay);
            
            quad->setEventCallback(handler);
            quad->setCullCallback(handler);
        }
        else
        {
            // Sanity check, do the same thing as QWidgetImage and 
            // QGraphicsViewAdapter but in a separate Qt window.

            graphicsScene = new QGraphicsScene;
            graphicsScene->addWidget(widget);

            QGraphicsView* graphicsView = new QGraphicsView;
            graphicsView->setScene(graphicsScene);

            QMainWindow* mainWindow = new QMainWindow;
            mainWindow->setCentralWidget(graphicsView);
            mainWindow->setGeometry(50, 50, 1024, 768);
            mainWindow->show();
            mainWindow->raise();
        }

        // Add numFloatingWindows windows to the graphicsScene.
        for (unsigned int i = 0; i < (unsigned int)numFloatingWindows; ++i)
        {
            QWidget* window = new QWidget(0, Qt::Window);
            window->setWindowTitle(QString("Window %1").arg(i));
            window->setLayout(new QVBoxLayout);
            window->layout()->addWidget(new QLabel(QString("This window %1").arg(i)));
            window->layout()->addWidget(new MyPushButton(QString("Button in window %1").arg(i)));
            window->setGeometry(100, 100, 300, 300);

            QGraphicsProxyWidget *proxy = new QGraphicsProxyWidget(0, Qt::Window);
            proxy->setWidget(window);
            proxy->setFlag(QGraphicsItem::ItemIsMovable, true);

            graphicsScene->addItem(proxy);
        }

    }

    root->addChild(osgDB::readNodeFile("cow.osg.(15,0,5).trans.(0.1,0.1,0.1).scale"));

    osg::ref_ptr<osgViewer::Viewer> viewer = new osgViewer::Viewer(arguments);
    viewer->setSceneData(root.get());
    viewer->setCameraManipulator(new osgGA::TrackballManipulator());
    viewer->addEventHandler(new osgGA::StateSetManipulator(root->getOrCreateStateSet()));
    viewer->addEventHandler(new osgViewer::StatsHandler);
    viewer->addEventHandler(new osgViewer::WindowSizeHandler);

    viewer->setUpViewInWindow(50, 50, 1024, 768);
    viewer->getEventQueue()->windowResize(0, 0, 1024, 768);

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
