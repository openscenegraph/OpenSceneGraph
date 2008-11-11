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

#include <osg/Notify>
#include <osg/ArgumentParser>
#include <osg/Texture1D>
#include <osg/Texture2D>
#include <osg/Texture3D>

#include <osgViewer/Viewer>

class MemoryTest : public osg::Referenced
{
    public:
};

class GLObject : public osg::Referenced
{
    public:
        virtual void apply(osg::State& state) = 0;
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
                if (_pbuffer) throw "Failed to create PixelBuffer";
                else  throw "Failed to create GraphicsWindow";
            }
            return 0;
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
        
        void apply(osg::State& state)
        {
            _attribute->apply(state);
            
            if (state.checkGLErrors(_attribute.get()))
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

        TextureTest(int width=1, int height=1, int depth=1):
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
            return 0;            
        }
        

    protected:
    
        int     _width;
        int     _height;
        int     _depth;
};

/////////////////////////////////////////////////////////////////////////
//
// Texture test
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

int main( int argc, char **argv )
{
    osg::ArgumentParser arguments(&argc,argv);
    
    typedef std::list< osg::ref_ptr<MemoryTest> > Tests;
    Tests tests;
    
    int width, height, depth;
    while(arguments.read("--pbuffer",width,height)) { tests.push_back(new ContextTest(width, height, true)); }
    while(arguments.read("--pbuffer")) { tests.push_back(new ContextTest(512, 512, true)); }

    while(arguments.read("--window",width,height)) { tests.push_back(new ContextTest(width, height, false)); }
    while(arguments.read("--window")) { tests.push_back(new ContextTest(512,512, false)); }

    while(arguments.read("--texture",width,height,depth)) { tests.push_back(new TextureTest(width,height,depth)); }
    while(arguments.read("--texture",width,height)) { tests.push_back(new TextureTest(width,height,1)); }
    while(arguments.read("--texture",width)) { tests.push_back(new TextureTest(width,1,1)); }

    while(arguments.read("--fbo",width,height,depth)) { tests.push_back(new FboTest(width,height,depth)); }
    while(arguments.read("--fbo",width,height)) { tests.push_back(new FboTest(width,height,2)); }
    while(arguments.read("--fbo")) { tests.push_back(new FboTest(1024,1024,2)); }

    int maxNumContextIterations = 1;
    while(arguments.read("-c",maxNumContextIterations)) {}

    int maxNumGLIterations = 1000;
    while(arguments.read("-g",maxNumGLIterations)) {}

    typedef std::list< osg::ref_ptr<GLMemoryTest> > GLMemoryTests;
    typedef std::list< osg::ref_ptr<ContextTest> > ContextTests;

    
    ContextTests contextTests;
    GLMemoryTests glMemoryTests;
    
    for(Tests::iterator itr = tests.begin();
        itr != tests.end();
        ++itr)
    {
        MemoryTest* test = itr->get();
        if (dynamic_cast<GLMemoryTest*>(test)!=0)
        {
            glMemoryTests.push_back(dynamic_cast<GLMemoryTest*>(test));
        }
        else if (dynamic_cast<ContextTest*>(test)!=0)
        {
            contextTests.push_back(dynamic_cast<ContextTest*>(test));
        }
    }

    typedef std::list< osg::ref_ptr<osg::GraphicsContext> > Contexts;
    typedef std::list< osg::ref_ptr<GLObject> > GLObjects;
    Contexts allocatedContexts;
    GLObjects glObjects;

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
            printf("iteration %i\n",numContextIterations);
            for(ContextTests::iterator itr = contextTests.begin();
                itr != contextTests.end();
                ++itr)
            {
                osg::ref_ptr<osg::GraphicsContext> context = (*itr)->allocate();
                if (context.valid())
                {
                    allocatedContexts.push_back(context);

                    context->makeCurrent();
                    
                    for(GLObjects::iterator gitr = glObjects.begin();
                        gitr != glObjects.end();
                        ++gitr)
                    {
                        (*gitr)->apply(*(context->getState()));
                        ++numGLObjectsApplied;
                    }
                    
                    context->releaseContext();
                }
            }
        }
    }
    catch(const char* errorString)
    {
        printf("Exception caught, contexts completed = %i, gl objects successfully applied =%i, error = %s\n",numContextIterations, numGLObjectsApplied, errorString);
        return 1;
    }
    catch(...)
    {
        printf("Exception caught, contexts completed = %i, gl objects successfully applied =%i\n",numContextIterations, numGLObjectsApplied);
        return 1;
    }

    printf("Successful completion, contexts created = %i, gl objects applied =%i\n",numContextIterations, numGLObjectsApplied);
    
    return 0;
}
