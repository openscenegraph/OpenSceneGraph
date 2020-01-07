/* OpenSceneGraph example, osgmemorytest.
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

#include <osg/Notify>
#include <osg/Timer>
#include <osg/ArgumentParser>
#include <osg/Texture1D>
#include <osg/Texture2D>
#include <osg/Texture3D>
#include <osg/Geometry>
#include <osg/FrameBufferObject>

#include <osgViewer/Version>

#include <stdio.h>
#include <iostream>

class MemoryTest : public osg::Referenced
{
    public:
};

class GLObject : public osg::Referenced
{
    public:
        virtual void apply(osg::RenderInfo& renderInfo) = 0;
};

class GLMemoryTest : public MemoryTest
{
    public:
        virtual GLObject* allocate() = 0;
};

/////////////////////////////////////////////////////////////////////////
//
// Context test
class ContextTest : public MemoryTest
{
    public:
        ContextTest(int width, int height, bool pbuffer):
            _width(width),
            _height(height),
            _pbuffer(pbuffer) {}

        virtual osg::GraphicsContext* allocate()
        {
            osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
            traits->width = _width;
            traits->height = _height;
            traits->windowDecoration = true;
            traits->pbuffer = _pbuffer;
            traits->readDISPLAY();
            traits->setUndefinedScreenDetailsToDefaultScreen();

            osg::ref_ptr<osg::GraphicsContext> window = osg::GraphicsContext::createGraphicsContext(traits.get());
            if (window.valid())
            {
                if (window->realize())
                {
                    return window.release();
                }
                else
                {
                    if (_pbuffer) throw "Failed to realize PixelBuffer";
                    else  throw "Failed to realize GraphicsWindow";
                }
            }
            else
            {
                std::cerr<<"Error: Unable to create graphics context, problem with running osgViewer-"<<osgViewerGetVersion()<<", cannot create windows/pbuffers."<<std::endl;

                if (_pbuffer) throw "Failed to create PixelBuffer";
                else  throw "Failed to create GraphicsWindow";
            }
        }


    protected:

        int     _width;
        int     _height;
        bool    _pbuffer;
};

////////////////////////////////////////////////////////////////////////
//
// Wrap StateAttribute
class StateAttributeObject : public GLObject
{
    public:

        StateAttributeObject(osg::StateAttribute* sa): _attribute(sa) {}

        void apply(osg::RenderInfo& renderInfo)
        {
            _attribute->apply(*renderInfo.getState());

            if (renderInfo.getState()->checkGLErrors(_attribute.get()))
            {
                throw "OpenGL error";
            }
        }

        osg::ref_ptr<osg::StateAttribute> _attribute;
};

/////////////////////////////////////////////////////////////////////////
//
// Texture test
class TextureTest : public GLMemoryTest
{
    public:

        TextureTest(int width=256, int height=256, int depth=1):
            _width(width),
            _height(height),
            _depth(depth) {}

        virtual GLObject* allocate()
        {
            if (_depth>1)
            {
                osg::ref_ptr<osg::Image> image = new osg::Image;
                image->allocateImage(_width, _height, _depth, GL_RGBA, GL_UNSIGNED_BYTE);

                osg::ref_ptr<osg::Texture3D> texture = new osg::Texture3D;
                texture->setImage(image.get());
                texture->setResizeNonPowerOfTwoHint(false);

                return new StateAttributeObject(texture.get());
            }
            if (_height>1)
            {
                osg::ref_ptr<osg::Image> image = new osg::Image;
                image->allocateImage(_width, _height, 1, GL_RGBA, GL_UNSIGNED_BYTE);

                osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
                texture->setImage(image.get());
                texture->setResizeNonPowerOfTwoHint(false);

                return new StateAttributeObject(texture.get());
            }
            if (_width>1)
            {
                osg::ref_ptr<osg::Image> image = new osg::Image;
                image->allocateImage(_width, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE);

                osg::ref_ptr<osg::Texture1D> texture = new osg::Texture1D;
                texture->setImage(image.get());
                texture->setResizeNonPowerOfTwoHint(false);

                return new StateAttributeObject(texture.get());
            }
            else
            {
                throw "Invalid texture size of 0,0,0";
            }
        }


    protected:

        int     _width;
        int     _height;
        int     _depth;
};


/////////////////////////////////////////////////////////////////////////
//
// FrameBufferObject test
class FboTest : public GLMemoryTest
{
    public:

        FboTest(int width=1024, int height=1024, int depth=2):
            _width(width),
            _height(height),
            _depth(depth) {}

        virtual GLObject* allocate()
        {
            osg::ref_ptr<osg::FrameBufferObject> fbo = new osg::FrameBufferObject;

            if (_depth>=1) fbo->setAttachment(osg::Camera::COLOR_BUFFER, osg::FrameBufferAttachment(new osg::RenderBuffer(_width, _height, GL_RGBA)));
            if (_depth>=2) fbo->setAttachment(osg::Camera::DEPTH_BUFFER, osg::FrameBufferAttachment(new osg::RenderBuffer(_width, _height, GL_DEPTH_COMPONENT24)));

            return new StateAttributeObject(fbo.get());
        }


    protected:

        int     _width;
        int     _height;
        int     _depth;
};



////////////////////////////////////////////////////////////////////////
//
// Wrap Drawable
class DrawableObject : public GLObject
{
    public:

        DrawableObject(osg::Drawable* drawable): _drawable(drawable) {}

        void apply(osg::RenderInfo& renderInfo)
        {
            _drawable->draw(renderInfo);

            if (renderInfo.getState()->checkGLErrors("Drawable"))
            {
                throw "OpenGL error";
            }
        }

        osg::ref_ptr<osg::Drawable> _drawable;
};

/////////////////////////////////////////////////////////////////////////
//
// Geometry test
class GeometryTest : public GLMemoryTest
{
    public:

        enum GLObjectType
        {
            VERTEX_ARRAY,
            DISPLAY_LIST,
            VERTEX_BUFFER_OBJECT
        };


        GeometryTest(GLObjectType type, int width=64, int height=64):
            _glObjectType(type),
            _width(width),
            _height(height) {}

        virtual GLObject* allocate()
        {
            unsigned int numVertices = _width * _height;
            osg::Vec3Array* vertices = new osg::Vec3Array(numVertices);
            for(int j=0; j<_height; ++j)
            {
                for(int i=0; i<_width; ++i)
                {
                    (*vertices)[i+j*_width].set(float(i),float(j),0.0f);
                }
            }

            unsigned int numIndices = (_width-1) * (_height-1) * 4;
            osg::DrawElementsUShort* quads = new osg::DrawElementsUShort(GL_QUADS);
            quads->reserve(numIndices);
            for(int j=0; j<_height-1; ++j)
            {
                for(int i=0; i<_width-1; ++i)
                {
                    quads->push_back(i   + j*_width);
                    quads->push_back(i+1 + j*_width);
                    quads->push_back(i+1 + (j+1)*_width);
                    quads->push_back(i   + (j+1)*_width);
                }
            }

            osg::Geometry* geometry = new osg::Geometry;
            geometry->setVertexArray(vertices);
            geometry->addPrimitiveSet(quads);

            switch(_glObjectType)
            {
                case(VERTEX_ARRAY):
                    geometry->setUseDisplayList(false);
                    geometry->setUseVertexBufferObjects(false);
                    break;
                case(DISPLAY_LIST):
                    geometry->setUseDisplayList(true);
                    geometry->setUseVertexBufferObjects(false);
                    break;
                case(VERTEX_BUFFER_OBJECT):
                    geometry->setUseDisplayList(false);
                    geometry->setUseVertexBufferObjects(true);
                    break;
            }

            return new DrawableObject(geometry);
        }


    protected:

        GLObjectType    _glObjectType;
        int             _width;
        int             _height;
};

int main( int argc, char **argv )
{
    osg::ArgumentParser arguments(&argc,argv);

    arguments.getApplicationUsage()->setApplicationName(arguments.getApplicationName());
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" tests OpenGL and Windowing memory scalability..");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","List command line options.");
    arguments.getApplicationUsage()->addCommandLineOption("--pbuffer","Create a 512x512 pixel buffer.");
    arguments.getApplicationUsage()->addCommandLineOption("--pbuffer <width> <height>","Create a pixel buffer of specified dimensions.");
    arguments.getApplicationUsage()->addCommandLineOption("--window","Create a 512x512 graphics window.");
    arguments.getApplicationUsage()->addCommandLineOption("--window <width> <height>","Create a graphics window of specified dimensions.");
    arguments.getApplicationUsage()->addCommandLineOption("--delay <micoseconds>","Set a delay in microseconds before all OpenGL object operations.");
    arguments.getApplicationUsage()->addCommandLineOption("--texture <width> <height> <depth>","Allocate a 3D texture of specified dimensions.");
    arguments.getApplicationUsage()->addCommandLineOption("--texture <width> <height>","Allocate a 2D texture of specified dimensions.");
    arguments.getApplicationUsage()->addCommandLineOption("--texture <width>","Allocate a 1D texture of specified dimensions.");
    arguments.getApplicationUsage()->addCommandLineOption("--geometry <width> <height>","Allocate a osg::Geometry representing a grid of specified size, using OpenGL Display Lists.");
    arguments.getApplicationUsage()->addCommandLineOption("--geometry-va <width> <height>","Allocate a osg::Geometry representing a grid of specified size, using Vertex Arrays.");
    arguments.getApplicationUsage()->addCommandLineOption("--geometry-vbo <width> <height>","Allocate a osg::Geometry representing a grid of specified size, using Vertex Buffer Objects.");
    arguments.getApplicationUsage()->addCommandLineOption("--fbo <width> <height>","Allocate a FrameBufferObject of specified dimensions.");
    arguments.getApplicationUsage()->addCommandLineOption("-c <num>","Set the number of contexts to create of each type specified.");
    arguments.getApplicationUsage()->addCommandLineOption("-g <num>","Set the number of GL objects to create of each type specified.");

    if (arguments.read("-h") || arguments.read("--help"))
    {
        arguments.getApplicationUsage()->write(std::cout, osg::ApplicationUsage::COMMAND_LINE_OPTION);
        return 1;
    }

    if (arguments.argc()<=1)
    {
        arguments.getApplicationUsage()->write(std::cout,osg::ApplicationUsage::COMMAND_LINE_OPTION);
        return 1;
    }


    typedef std::list< osg::ref_ptr<GLMemoryTest> > GLMemoryTests;
    typedef std::list< osg::ref_ptr<ContextTest> > ContextTests;

    ContextTests contextTests;
    GLMemoryTests glMemoryTests;

    int width, height, depth;
    while(arguments.read("--pbuffer",width,height)) { contextTests.push_back(new ContextTest(width, height, true)); }
    while(arguments.read("--pbuffer")) { contextTests.push_back(new ContextTest(512, 512, true)); }

    while(arguments.read("--window",width,height)) { contextTests.push_back(new ContextTest(width, height, false)); }
    while(arguments.read("--window")) { contextTests.push_back(new ContextTest(512,512, false)); }

    while(arguments.read("--texture",width,height,depth)) { glMemoryTests.push_back(new TextureTest(width,height,depth)); }
    while(arguments.read("--texture",width,height)) { glMemoryTests.push_back(new TextureTest(width,height,1)); }
    while(arguments.read("--texture",width)) { glMemoryTests.push_back(new TextureTest(width,1,1)); }

    while(arguments.read("--fbo",width,height,depth)) { glMemoryTests.push_back(new FboTest(width,height,depth)); }
    while(arguments.read("--fbo",width,height)) { glMemoryTests.push_back(new FboTest(width,height,2)); }
    while(arguments.read("--fbo")) { glMemoryTests.push_back(new FboTest(1024,1024,2)); }

    while(arguments.read("--geometry",width,height)) { glMemoryTests.push_back(new GeometryTest(GeometryTest::DISPLAY_LIST,width,height)); }
    while(arguments.read("--geometry")) { glMemoryTests.push_back(new GeometryTest(GeometryTest::DISPLAY_LIST,64,64)); }

    while(arguments.read("--geometry-vbo",width,height)) { glMemoryTests.push_back(new GeometryTest(GeometryTest::VERTEX_BUFFER_OBJECT,width,height)); }
    while(arguments.read("--geometry-vbo")) { glMemoryTests.push_back(new GeometryTest(GeometryTest::VERTEX_BUFFER_OBJECT,64,64)); }

    while(arguments.read("--geometry-va",width,height)) { glMemoryTests.push_back(new GeometryTest(GeometryTest::VERTEX_ARRAY,width,height)); }
    while(arguments.read("--geometry-va")) { glMemoryTests.push_back(new GeometryTest(GeometryTest::VERTEX_ARRAY,64,64)); }

    unsigned int sleepTime = 0;
    while(arguments.read("--delay",sleepTime)) {}

    int maxNumContextIterations = 1;
    while(arguments.read("-c",maxNumContextIterations)) {}

    int maxNumGLIterations = 1000;
    while(arguments.read("-g",maxNumGLIterations)) {}

#if 0
    // any option left unread are converted into errors to write out later.
    arguments.reportRemainingOptionsAsUnrecognized();

    // report any errors if they have occurred when parsing the program arguments.
    if (arguments.errors())
    {
        arguments.writeErrorMessages(std::cout);
        return 1;
    }
#endif

    typedef std::list< osg::ref_ptr<osg::GraphicsContext> > Contexts;
    typedef std::list< osg::ref_ptr<GLObject> > GLObjects;
    Contexts allocatedContexts;
    GLObjects glObjects;

    if (contextTests.empty())
    {
        if (glMemoryTests.empty())
        {
            std::cout<<"No tests specified, please specify test using the command line options below."<<std::endl<<std::endl;

            arguments.getApplicationUsage()->write(std::cout,osg::ApplicationUsage::COMMAND_LINE_OPTION);
            return 1;
        }
        else
        {
            contextTests.push_back(new ContextTest(512,512, false));
        }
    }

    osg::Timer_t startTick = osg::Timer::instance()->tick();

    // use printf's below as C++'s ostream classes use more memory and are more likely to fail when everything
    // goes wrong with memory allocations.

    int numContextIterations = 0;
    int numGLObjectIterations = 0;
    int numGLObjectsApplied = 0;
    try
    {
        for(; numGLObjectIterations<maxNumGLIterations; ++numGLObjectIterations)
        {
            for(GLMemoryTests::iterator itr = glMemoryTests.begin();
                itr != glMemoryTests.end();
                ++itr)
            {
                osg::ref_ptr<GLObject> glObject = (*itr)->allocate();
                if (glObject.valid()) glObjects.push_back(glObject.get());
            }
        }

        for(;numContextIterations<maxNumContextIterations; ++numContextIterations)
        {
            printf("GraphicsContext %i\n",numContextIterations);
            for(ContextTests::iterator itr = contextTests.begin();
                itr != contextTests.end();
                ++itr)
            {
                osg::ref_ptr<osg::GraphicsContext> context = (*itr)->allocate();
                if (context.valid())
                {
                    allocatedContexts.push_back(context);

                    context->makeCurrent();

                    osg::RenderInfo renderInfo;
                    renderInfo.setState(context->getState());

                    for(GLObjects::iterator gitr = glObjects.begin();
                        gitr != glObjects.end();
                        ++gitr)
                    {
                        if (sleepTime>0) OpenThreads::Thread::microSleep( sleepTime );

                        printf("%i ",numGLObjectsApplied);fflush(stdout);

                        (*gitr)->apply(renderInfo);
                        ++numGLObjectsApplied;
                    }

                    context->releaseContext();

                    printf("\n\n"); fflush(stdout);
                }
            }
        }
    }
    catch(const char* errorString)
    {
        printf("\nException caught, contexts completed = %i, gl objects successfully applied = %i, error = %s\n\n",numContextIterations, numGLObjectsApplied, errorString);
        return 1;
    }
    catch(...)
    {
        printf("\nException caught, contexts completed = %i, gl objects successfully applied = %i\n\n",numContextIterations, numGLObjectsApplied);
        return 1;
    }

    osg::Timer_t endTick = osg::Timer::instance()->tick();

    printf("\nSuccessful completion, contexts created = %i, gl objects applied = %i\n",numContextIterations, numGLObjectsApplied);
    printf("Duration = %f seconds.\n\n",osg::Timer::instance()->delta_s(startTick, endTick));


    return 0;
}
