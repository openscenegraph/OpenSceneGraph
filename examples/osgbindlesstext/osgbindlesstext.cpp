/* OpenSceneGraph example, osgbindlesstex.
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

///\author David A Heitbrink
/// This is an example implementation of the use of bindless textures.
/// "bindless" textures are relatively simple concept, basically
/// you get a texture handle, then ask the driver to keep said
/// handle resident.
///
/// Once the texture has been made resident, we need to upload
/// the handle (a 64 bit unsigned int) to the shader. This can
/// be done in a number of ways, through attributes, uniform
/// buffer objects, shader buffer objects or just plain uniforms.
/// 
/// The basic point of the bindless texture is to remove the need
/// to bind a new texture every time we want to render something
/// with a different texture. Generally speaking in broad terms
/// driver overhead tends to be a huge bottle neck on modern
/// hardware (as of late 2016). By using bindless textures
/// we can remove a lot of calls to the driver to switch active 
/// textures while rendering. What this also allows us to do
/// is to consolidate more objects + draw states as we do
/// not need to change textures, this save us a lot of calls to 
/// the driver.
///
/// This example combines instancing with bindless textures 
/// to draw 1000 cubes, each with a unique texture. This is
/// a pretty simplified example, where each instance ID is 
/// used as a index into the array of textures. 
///
/// One of the powerful things about bindless textures is it allows
/// many more objects to be combined into a single drawable. 
/// However to do this you may need to add an attribute to 
/// use an index into the array of texture handles, and not
/// just use the instance ID like in this example.

#include <osg/Depth>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Material>
#include <osg/Math>
#include <osg/MatrixTransform>
#include <osg/PolygonOffset>
#include <osg/Projection>
#include <osg/ShapeDrawable>
#include <osg/StateSet>
#include <osg/Switch>
#include <osg/Texture2D>
#include <osg/TextureBuffer>
#include <osg/Image>
#include <osg/TexEnv>
#include <osg/VertexProgram>
#include <osg/FragmentProgram>
#include <osg/GLExtensions>
#include <osg/ContextData>

#include <osg/TextureBuffer>
#include <osg/BufferIndexBinding>

#include <osgDB/ReadFile>
#include <osgDB/FileUtils>


#include <osgText/Text>

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include <osgGA/StateSetManipulator>

#include <iostream>
#include <sstream>
//Hard coded constant for number unique textures 
const int TextureCount = 1000;
// To use bindless textures, we need to tell the GPU to 
// enable the use of 64 bit integers and bindless textures
//
//
// At this time (late 2016) NVidia drivers seem to dislike 
// the ARB, GL_ARB_bindless_texture, if you do not have a 
// NVidia driver, you will most likely have to change:  
// GL_NV_gpu_shader5
// to: 
// GL_ARB_gpu_shader5

//the XXX is so we can replace it with a string from TextureCount
std::string vertShader= 
"#version 450 compatibility                                 \n"
"#extension GL_ARB_bindless_texture : require               \n"
"#extension GL_NV_gpu_shader5 : require // uint64_t         \n"
"//#extension GL_ARB_gpu_shader5 : require // uint64_t      \n"
"//#extension GL_ARB_gpu_shader_int64: require // uint64_t  \n"
"in float osg_FrameTime;                                    \n"
"out vec2 TexCoord;                                         \n"
"flat out int textureIndex;                                 \n"
"void main() {                                              \n"
"    mat4 scale =mat4(0.01, 0.00, 0.00, 0.00,               \n"
"                     0.00, 0.01, 0.00, 0.00,               \n"
"                     0.00, 0.00, 0.01, 0.00,               \n"
"                     0.00, 0.00, 0.00, 1.00);              \n"
"    vec4 pos = gl_Vertex*scale;                            \n"
"    pos.x += ((gl_InstanceID%100)/10)*0.015-0.075;         \n"
"    pos.y += (gl_InstanceID/100)*0.015 - 0.075;            \n"
"    pos.z += (gl_InstanceID%10)*0.015 - 0.075;             \n"
"    pos.w = 1;                                             \n"
"    gl_Position = gl_ModelViewProjectionMatrix*pos;        \n"
"    TexCoord = gl_MultiTexCoord0.xy;                       \n"
"    textureIndex = gl_InstanceID%XXX;                      \n"
"}                                                          \n"
;
//we could setup tex to be of type sampler2D, and not have to do
//the type conversion, but I wanted to added code to test if tex
//had a value set. 
//If we get a red cube, we are not getting our handle from our UBO, 
//if we get a black cube, then we are having an issue with the 
//texture handle itself
std::string fragShader = 
"#version 450 compatibility                                    \n"
"#extension GL_ARB_bindless_texture : require                  \n"
"#extension GL_NV_gpu_shader5 : require // uint64_t            \n"
"//#extension GL_ARB_gpu_shader5 : require // uint64_t         \n"
"//#extension GL_ARB_gpu_shader_int64: require // uint64_t     \n"
"uniform sampler2D TextureId;                                  \n"
"in vec2 TexCoord;                                             \n"
"flat in int textureIndex;                                     \n"
"layout (binding = 0, std140) uniform TEXTURE_BLOCK            \n"
"{                                                             \n"
"    uint64_t      tex[XXX];                                   \n"
"};                                                            \n"
"void main() {                                                 \n"
"    int tIndex = (int)(textureIndex);                         \n"
"    sampler2D myText = sampler2D(tex[tIndex]);                \n"
"    gl_FragColor = texture2D(myText,TexCoord);                \n"
"    if (tex[tIndex] == 0) gl_FragColor.r = 1.0;               \n"
"}                                                             \n"
;

///This class provides a basic wraper for a Uniform Buffer Object
///or UBO, and provides the storage for the texture handles
class BindlessBuffer: public osg::Referenced{
public:
    typedef osg::ref_ptr<osg::UniformBufferObject>  UniBufferObjRef;
    typedef osg::ref_ptr<osg::UniformBufferBinding> UniBufferBindingRef;
    typedef osg::ref_ptr<osg::UInt64Array> HandleArrayRef;
    typedef osg::ref_ptr<BindlessBuffer>  BindlessBufferRef;
    static BindlessBufferRef Make(size_t count){
        BindlessBufferRef val = new BindlessBuffer();
        val->_count   = count;
        val->_sbbo    = new osg::UniformBufferObject;
        val->_handles = new osg::UInt64Array();
        val->_handles->resize(count*2,0);
        val->_handles->setBufferObject(val->_sbbo.get());
        val->_ssbb    = new osg::UniformBufferBinding(0, val->_handles.get(), 0, sizeof(GLuint64)*count);
        return val;
    }
    BindlessBuffer& operator  = (const BindlessBuffer& rhs){
        if (this != &rhs){
            _count=rhs._count;
            _sbbo =rhs._sbbo ;
            _ssbb =rhs._ssbb ;
            _handles = rhs._handles;
        }
        return *this;
    }
    BindlessBuffer(const BindlessBuffer& rhs):osg::Referenced(rhs){
        if (this != &rhs){
            _count=rhs._count;
            _sbbo =rhs._sbbo ;
            _ssbb =rhs._ssbb ;
            _handles = rhs._handles;
        }
    }
    UniBufferObjRef& Object(){return _sbbo;}
    UniBufferBindingRef& Binding(){return _ssbb;}
    HandleArrayRef& Handles(){return _handles;}
    int count(){return _count;}
private:
    int _count;
    UniBufferObjRef _sbbo;
    UniBufferBindingRef _ssbb;
    HandleArrayRef _handles;
     
    BindlessBuffer():osg::Referenced(),_count(0){
    }
};

///This class extends a Texture, when this is texture is applied 
///the first time, it will setup all our texture handles, after that
///it will not make any more GL calls until it gets released
class BindlessTexture: public osg::Texture2D
{
public:
    typedef osg::ref_ptr<BindlessBuffer> BufferRef;
    typedef std::vector<osg::ref_ptr<osg::Image> > TextureList;
    typedef std::vector<GLuint64> HandleList;
    typedef osg::ref_ptr< osg::Texture::TextureObject> TextureObjectRef;
    typedef std::vector<TextureObjectRef> TextureObjectList;
    typedef osg::buffered_object<TextureObjectList>  TextureObjectBuffer;

    BindlessTexture(); 
    BindlessTexture(BufferRef, TextureList);
    BindlessTexture(const BindlessTexture& rhs, const osg::CopyOp& copy =osg::CopyOp::SHALLOW_COPY);
    void releaseGLObjects(osg::State* state) const;
    void resizeGLObjectBuffers(unsigned maxSize);
    void setBidlessIndex(unsigned int index);
    META_StateAttribute(osg, BindlessTexture, TEXTURE);

    void apply(osg::State& state) const;
protected:
    void applyOnce(osg::State &state) const;
    mutable osg::buffered_object<HandleList> _handles;
    mutable TextureList _textureList;
    mutable osg::ref_ptr<BindlessBuffer> _buffer;
    mutable std::vector<bool> _isBound;
    mutable TextureObjectBuffer _textureBufferList;
    // array index = texture image unit.
    unsigned int _bindlessIndex;
};


BindlessTexture::BindlessTexture():osg::Texture2D(),_bindlessIndex(0)
{
    _isBound.resize(5,false);
}

BindlessTexture::BindlessTexture(const BindlessTexture& rhs, const osg::CopyOp& copy) :
    osg::Texture2D( rhs, copy )
{
    _isBound.resize(5,false);
    _buffer = rhs._buffer;
    _bindlessIndex = rhs._bindlessIndex;
    for(unsigned i=0; i<rhs._handles.size(); ++i)
        _handles[i] = rhs._handles[i];
}

BindlessTexture::BindlessTexture(BufferRef ref,TextureList textureList) :
    osg::Texture2D( textureList[0] ),
    _textureList(textureList),
    _buffer(ref),
    _bindlessIndex(0)
{
    _isBound.resize(5,false);
}

void BindlessTexture::setBidlessIndex(unsigned int index){
    _bindlessIndex = index;
}
/// Just as the name suggest this should be called once per
/// context, during its lifetime. This basically 
/// just sets up our texture handles, and loads them
/// into our UBO. A good portion of this was copied from
/// Texture2D::apply, this is in no ways a general solution.
void BindlessTexture::applyOnce(osg::State& state) const
{
    if (!_buffer)
        return;
    
    TextureObject* textureObject;
    unsigned contextID = state.getContextID();
    osg::GLExtensions* extensions = osg::GLExtensions::Get( contextID, true );

    osg::ref_ptr<osg::Image> image = _image;
    if (_handles[contextID].size() <  _textureList.size())
        _handles[contextID].resize( _textureList.size(),0);
    if (_textureBufferList[contextID].size() < _textureList.size())
        _textureBufferList[contextID].resize( _textureList.size());
    int txtcount  = _textureList.size();
    if (_buffer->count() < txtcount)
        txtcount = _buffer->count();
    //for each actual texture we have, bind it, get the texture hande, assign the value to our UBO
    for (int i = 0; i <txtcount; i++){
        image = _textureList[i];
        if (_image.valid()) 
            computeInternalFormatWithImage(*image);
        else 
            continue;
        // compute the dimensions of the texture.
        computeRequiredTextureDimensions(state,*image,_textureWidth, _textureHeight, _numMipmapLevels);
        textureObject = generateAndAssignTextureObject(contextID,GL_TEXTURE_2D,_numMipmapLevels,_internalFormat,_textureWidth,_textureHeight,1,_borderWidth);
        textureObject->bind();
        applyTexParameters(GL_TEXTURE_2D,state);

        applyTexImage2D_load(state,GL_TEXTURE_2D,image.get(),
            _textureWidth, _textureHeight, _numMipmapLevels);
        textureObject->setAllocated(true);
        _textureBufferList[contextID][i] = textureObject;

        //Here is where the "magic" happens, we get the texture handle for our texture, copy it to our UBO,
        //and then tell OpenGL to keep the handle resident
        _handles[contextID][i] = extensions->glGetTextureHandle( textureObject->id() );
        std::vector<GLuint64> &vec = _buffer->Handles()->asVector();
        vec[i*2]  = _handles[contextID][i];
        _buffer->Object()->dirty();
        _buffer->Handles()->dirty();
     
        if ( _handles[contextID][i] != 0L || extensions->glIsTextureHandleResident( _handles[contextID][i]) == GL_FALSE)
        {
            extensions->glMakeTextureHandleResident( _handles[contextID][i] );           
        }
    }
    
    // update the modified tag to show that it is up to date.
    getModifiedCount(contextID) = image->getModifiedCount();
}

void BindlessTexture::apply(osg::State& state) const
{
   unsigned contextID = state.getContextID();
   if ( _isBound[contextID] == false )
   {
       applyOnce(state);
       _isBound[contextID] = true;
   }else{
       //we should mostly hit this during the lifetime of this object,
       //note we basically do nothing......
   }
}
/// cleanup, we just need to tell OpenGL to release our texture handle
void BindlessTexture::releaseGLObjects(osg::State* state) const
{
    if (  state )
    {
        unsigned contextID = state->getContextID();
        osg::Texture2D::releaseGLObjects( state );
        osg::GLExtensions* ext = osg::GLExtensions::Get( contextID, true );
       
        for(unsigned i=0; i<_handles[contextID].size(); ++i)
        {
           ext->glMakeTextureHandleNonResident( _handles[contextID][i] );
           _handles[contextID][i] = 0;
        }
        
    }
}

void
BindlessTexture::resizeGLObjectBuffers(unsigned int maxSize)
{
    osg::Texture2D::resizeGLObjectBuffers( maxSize );

    unsigned int handleSize = _handles.size();
    unsigned int txtSize = _textureList.size();
    if ( handleSize < maxSize ) {
        _isBound.resize(maxSize,false);
    }
    if ( handleSize < maxSize ) {
        _handles.resize( maxSize );
        for(unsigned i=handleSize; i<_handles.size(); ++i){
            for(unsigned j=0; j<txtSize; ++j)
                _handles[i][j] = 0;
        }
    }
}

typedef osg::ref_ptr<osg::Image> ImageRef;
///////////////////////////////////////////////////////
///Create an array of images, with checkerboard
///pattern with random color and size
///
void createImageArray(osg::StateSet* attachPnt){
    BindlessTexture::TextureList images;
    images.resize(TextureCount);
    BindlessBuffer::BindlessBufferRef buffer = BindlessBuffer::Make(TextureCount);
    srand (time(NULL));
    for (int i =0; i < TextureCount; i++){
        ImageRef tImage = new osg::Image();
        int powerOf2 = rand()%6+4;
        const unsigned int imageSize = 1<<powerOf2;
        tImage->allocateImage(imageSize,imageSize,1,GL_RGBA,GL_UNSIGNED_BYTE);
        unsigned char* buff = tImage->data();
        const int stride = 4;
        unsigned char primaryColor[4];
        
        int boxWidth = rand()%15+2;
        int boxLength = rand()%15+2;

        //light squares
        primaryColor[0] = rand()%128 + 128;
        primaryColor[1] = rand()%128 + 128;
        primaryColor[2] = rand()%128 + 128;
        //dark squares
        unsigned char secondaryColor[4];
        secondaryColor[0] = rand()%128;
        secondaryColor[1] = rand()%128;
        secondaryColor[2] = rand()%128;
        for (unsigned int x = 0; x < imageSize; x++){
            for (unsigned int y =0; y<imageSize; y++){
                unsigned char* pixel = &buff[(x*imageSize+y)*stride];
                int xSide = x/boxWidth;
                int ySide = y/boxLength;
                bool isPrimaryColor =  (xSide+ySide)%2>0;
                if (isPrimaryColor){
                    pixel[0] = primaryColor[0];
                    pixel[1] = primaryColor[1];
                    pixel[2] = primaryColor[2];
                }else{
                    pixel[0] = secondaryColor[0];
                    pixel[1] = secondaryColor[1];
                    pixel[2] = secondaryColor[2];                
                }
                pixel[3] = 255;
            }
        }
        images[i] = tImage;
        
        std::stringstream sstr;
        sstr<<"Image"<<i;
        tImage->setName(sstr.str());
    }
    BindlessTexture* tex = new BindlessTexture(buffer,images);
    attachPnt->setTextureAttribute(0,tex,osg::StateAttribute::ON);
    attachPnt->setAttributeAndModes(buffer->Binding(), osg::StateAttribute::ON);
}
///Create a cube centered at the origin, with given by size
///
osg::Geometry* createCube(float scale, osg::Vec3 origin = osg::Vec3(0.0f,0.0f,0.0f) )
{
    osg::Geometry* geometry = new osg::Geometry;
    geometry->setName("TexturedCubeArray");

    osg::Vec3Array* vertices = new osg::Vec3Array;
    geometry->setVertexArray(vertices);

    osg::Vec2Array* tcoords = new osg::Vec2Array();
    geometry->setTexCoordArray(0,tcoords);

    origin -= osg::Vec3(scale/2.0f,scale/2.0f,scale/2.0f);
    osg::Vec3 dx(scale,0.0f,0.0f);
    osg::Vec3 dy(0.0f,scale,0.0f);
    osg::Vec3 dz(0.0f,0.0f,scale);

    {
        // front face
        vertices->push_back(origin);
        vertices->push_back(origin+dx);
        vertices->push_back(origin+dx+dz);
        vertices->push_back(origin+dz);

        tcoords->push_back(osg::Vec2(0.0f,0.0f));
        tcoords->push_back(osg::Vec2(1.0f,0.0f));
        tcoords->push_back(osg::Vec2(1.0f,1.0f));
        tcoords->push_back(osg::Vec2(0.0f,1.0f));
    }

    {
        // back face
        vertices->push_back(origin+dy);
        vertices->push_back(origin+dy+dz);
        vertices->push_back(origin+dy+dx+dz);
        vertices->push_back(origin+dy+dx);

        tcoords->push_back(osg::Vec2(0.0f,0.0f));
        tcoords->push_back(osg::Vec2(1.0f,0.0f));
        tcoords->push_back(osg::Vec2(1.0f,1.0f));
        tcoords->push_back(osg::Vec2(0.0f,1.0f));
    }

    {
        // left face
        vertices->push_back(origin+dy);
        vertices->push_back(origin);
        vertices->push_back(origin+dz);
        vertices->push_back(origin+dy+dz);

        tcoords->push_back(osg::Vec2(0.0f,0.0f));
        tcoords->push_back(osg::Vec2(1.0f,0.0f));
        tcoords->push_back(osg::Vec2(1.0f,1.0f));
        tcoords->push_back(osg::Vec2(0.0f,1.0f));
    }

    {
        // right face
        vertices->push_back(origin+dx+dy);
        vertices->push_back(origin+dx+dy+dz);
        vertices->push_back(origin+dx+dz);
        vertices->push_back(origin+dx);

        tcoords->push_back(osg::Vec2(0.0f,0.0f));
        tcoords->push_back(osg::Vec2(1.0f,0.0f));
        tcoords->push_back(osg::Vec2(1.0f,1.0f));
        tcoords->push_back(osg::Vec2(0.0f,1.0f));
    }

    {
        // top face
        vertices->push_back(origin+dz);
        vertices->push_back(origin+dz+dx);
        vertices->push_back(origin+dz+dx+dy);
        vertices->push_back(origin+dz+dy);

        tcoords->push_back(osg::Vec2(0.0f,0.0f));
        tcoords->push_back(osg::Vec2(1.0f,0.0f));
        tcoords->push_back(osg::Vec2(1.0f,1.0f));
        tcoords->push_back(osg::Vec2(0.0f,1.0f));
    }

    {
        // bottom face
        vertices->push_back(origin);
        vertices->push_back(origin+dy);
        vertices->push_back(origin+dx+dy);
        vertices->push_back(origin+dx);

        tcoords->push_back(osg::Vec2(0.0f,0.0f));
        tcoords->push_back(osg::Vec2(1.0f,0.0f));
        tcoords->push_back(osg::Vec2(1.0f,1.0f));
        tcoords->push_back(osg::Vec2(0.0f,1.0f));
    }
    osg::DrawArrays* primSet = new osg::DrawArrays(GL_QUADS, 0, vertices->size());
    geometry->addPrimitiveSet(primSet);

    return geometry;
}
///
///Here we are going to create our scene, basically we need to make sure our
///Bindless texture gets applied before our shader programs. 
///
///
osg::Group* CreateScene(){
    osg::Group* sceneRoot= new osg::Group();
    sceneRoot->setName("Root");
    osg::Geode *geo = new osg::Geode();
    geo->setName("Geo");
    sceneRoot->addChild(geo);
    osg::StateSet* scene_ss  = sceneRoot->getOrCreateStateSet();
    createImageArray(scene_ss);
    scene_ss->setMode(GL_DEPTH_TEST,osg::StateAttribute::ON);
    
    osg::ref_ptr<osg::Geometry> geom = createCube(0.9f);
    osg::PrimitiveSet *prim = geom->getPrimitiveSet(0);
    //instanced elements must use VBOs
    geom->setUseDisplayList(false);
    geom->setUseVertexBufferObjects(true);
    geom->setCullingActive(false);
    prim->setNumInstances(1000);
    prim->dirty();
    sceneRoot->addChild(geo);
    geo->addDrawable(geom.get());
        
    osg::StateSet* ss = geo->getOrCreateStateSet();

    std::string strTextureCount;
    std::stringstream ssconv;
    ssconv<<TextureCount;
    ssconv>>strTextureCount;
    std::string::size_type pos = vertShader.find("XXX");
    vertShader.replace(pos,size_t(3),strTextureCount);

    pos = fragShader.find("XXX");
    fragShader.replace(pos,size_t(3),strTextureCount);

    osg::Program* program = new osg::Program;
    osg::Shader* vertex_shader = new osg::Shader(osg::Shader::VERTEX, vertShader);
    program->addShader(vertex_shader);

    osg::Shader* fragment_shader = new osg::Shader(osg::Shader::FRAGMENT, fragShader);
    program->addShader(fragment_shader);

    ss->setAttributeAndModes(program, osg::StateAttribute::ON);
    
    
    return sceneRoot;
 }
int main(int argc, char** argv)
{
    // set command line options
    osg::ArgumentParser arguments(&argc,argv);

    // construct the viewer.
    osgViewer::Viewer viewer(arguments);

    // add the stats handler
    viewer.addEventHandler(new osgViewer::StatsHandler);
    
    // add model to viewer.
    viewer.setSceneData( CreateScene() );

    viewer.realize();
    
    viewer.getCamera()->getGraphicsContext()->getState()->setUseModelViewAndProjectionUniforms(true);

    return viewer.run();
}

