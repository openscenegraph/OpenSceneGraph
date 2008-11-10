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

class GLMemoryTest : public MemoryTest
{
    public:
        virtual void allocate(osg::State& state) = 0;
};

class ContextTest : public MemoryTest
{
    public:
        virtual osg::GraphicsContext* allocate() = 0;
};

/////////////////////////////////////////////////////////////////////////
//
// PBuffer test
class PBufferTest : public ContextTest
{
    public:
        PBufferTest(int width, int height):
            _width(width),
            _height(height) {}
        
        virtual bool requiresContext() { return false; }
        virtual osg::GraphicsContext* allocate()
        {
            osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
            traits->width = _width;
            traits->height = _height;
            traits->pbuffer = true;
            
            osg::ref_ptr<osg::GraphicsContext> pbuffer = osg::GraphicsContext::createGraphicsContext(traits.get());
            if (pbuffer.valid()) 
            {
                if (pbuffer->realize())
                {
                    pbuffer->makeCurrent();
                    pbuffer->releaseContext();

                    return pbuffer.release();
                }
                else
                {
                    throw "Failed to realize Pixelbuffer";
                }
            }
            else
            {
                throw "Failed to created PixelBuffer";
            }
            return 0;
        }
        

    protected:
    
        int     _width;
        int     _height;
};

/////////////////////////////////////////////////////////////////////////
//
// Window test
class WindowTest : public ContextTest
{
    public:
        WindowTest(int width, int height):
            _width(width),
            _height(height) {}
        
        virtual bool requiresContext() { return false; }
        virtual osg::GraphicsContext* allocate()
        {
            osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
            traits->width = _width;
            traits->height = _height;
            traits->windowDecoration = true;
            
            osg::ref_ptr<osg::GraphicsContext> window = osg::GraphicsContext::createGraphicsContext(traits.get());
            if (window.valid()) 
            {
                if (window->realize())
                {
                    window->makeCurrent();
                    window->releaseContext();

                    return window.release();
                }
                else
                {
                    throw "Failed to realize GraphicsWindow";
                }
            }
            else
            {
                throw "Failed to created GraphicsWindow";
            }
            return 0;
        }
        

    protected:
    
        int     _width;
        int     _height;
};


/////////////////////////////////////////////////////////////////////////
//
// Window test
class TextureTest : public GLMemoryTest
{
    public:
        TextureTest(int width=1, int height=1, int depth=1):
            _width(width),
            _height(height),
            _depth(depth) {}
        
        virtual bool requiresContext() { return true; }
        
        virtual void allocate(osg::State& state)
        {
            if (_depth>1)
            {
                osg::ref_ptr<osg::Image> image = new osg::Image;
                image->allocateImage(_width, _height, _depth, GL_RGBA, GL_UNSIGNED_BYTE);
                
                osg::ref_ptr<osg::Texture3D> texture = new osg::Texture3D;
                texture->setImage(image.get());
                
                texture->apply(state);

                _textures.push_back(texture.get());
                
            }
            if (_height>1)
            {
                osg::ref_ptr<osg::Image> image = new osg::Image;
                image->allocateImage(_width, _height, 1, GL_RGBA, GL_UNSIGNED_BYTE);
                
                osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
                texture->setImage(image.get());
                
                texture->apply(state);

                _textures.push_back(texture.get());
            }
            if (_width>1)
            {
                osg::ref_ptr<osg::Image> image = new osg::Image;
                image->allocateImage(_width, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE);
                
                osg::ref_ptr<osg::Texture1D> texture = new osg::Texture1D;
                texture->setImage(image.get());
                
                texture->apply(state);

                _textures.push_back(texture.get());
            }
            else
            {
                throw "Invalid texture size of 0,0,0.";
            }
            
        }
        

    protected:
    
        virtual ~TextureTest()
        {
        }
        
        typedef std::list< osg::ref_ptr<osg::Texture> > Textures;
        Textures _textures;
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
    while(arguments.read("--pbuffer",width,height)) { tests.push_back(new PBufferTest(width,height)); }
    while(arguments.read("--pbuffer")) { tests.push_back(new PBufferTest(1024,1024)); }

    while(arguments.read("--window",width,height)) { tests.push_back(new WindowTest(width,height)); }
    while(arguments.read("--window")) { tests.push_back(new WindowTest(1024,1024)); }

    while(arguments.read("--texture",width,height,depth)) { tests.push_back(new TextureTest(width,height,depth)); }
    while(arguments.read("--texture",width,height)) { tests.push_back(new TextureTest(width,height,1)); }
    while(arguments.read("--texture",width)) { tests.push_back(new TextureTest(width,1,1)); }

    int maxNumContextIterations = 1000;
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
    Contexts allocatedContexts;

    int numContextIterations = 0;
    try
    {
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
                    context->makeCurrent();
                    context->releaseContext();
                    
                    allocatedContexts.push_back(context);
                }
            }
        }
    }
    catch(const char* errorString)
    {
        printf("Exception caught, number of iterations completed = %i, error = %s\n",numContextIterations, errorString);
        return 1;
    }
    catch(...)
    {
        printf("Exception caught, number of iterations completed = %i\n",numContextIterations);
        return 1;
    }

    printf("Successful completion, number of iterations completed = %i\n",numContextIterations);
    
    return 0;
}
