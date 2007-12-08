/* OpenSceneGraph example, osganimate.
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

#if USE_QT4

    #include <QtGui/QApplication>
#else

    #include <qapplication.h>
#endif

#include <osg/ArgumentParser>

#include <iostream>



extern int mainQOSGWidget(QApplication& a, osg::ArgumentParser& arguments);
extern int mainAdapterWidget(QApplication& a, osg::ArgumentParser& arguments);

int main( int argc, char **argv )
{
    QApplication a( argc, argv );
    
    if (argc<2)
    {
        std::cout << argv[0] <<": requires filename argument." << std::endl;
        return 1;
    }

    osg::ArgumentParser arguments(&argc, argv);

    if (arguments.read("--QOSGWidget"))
    {
        // Use QWidget and integrate OSG/OpenGL with it via osgViewer's built in support
        return mainQOSGWidget(a, arguments);
    }
    else
    {
        // Use QGLWidget and integrate OSG with it by adapting osgViewer via its embedded window mode
        return mainAdapterWidget(a, arguments);
    }
}

/*EOF*/
