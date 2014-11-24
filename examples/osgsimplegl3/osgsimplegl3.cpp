// This is public domain software and comes with
// absolutely no warranty. Use of public domain software
// may vary between counties, but in general you are free
// to use and distribute this software for any purpose.


// Example: OSG using an OpenGL 3.1 context.
// The comment block at the end of the source describes building OSG
// for use with OpenGL 3.x.

#include <osgViewer/Viewer>
#include <osgDB/ReadFile>
#include <osg/GraphicsContext>
#include <osg/Camera>
#include <osg/Viewport>
#include <osg/StateSet>
#include <osg/Program>
#include <osg/Shader>


void configureShaders( osg::StateSet* stateSet )
{
    const std::string vertexSource =
        "#version 140 \n"
        " \n"
        "uniform mat4 osg_ModelViewProjectionMatrix; \n"
        "uniform mat3 osg_NormalMatrix; \n"
        "uniform vec3 ecLightDir; \n"
        " \n"
        "in vec4 osg_Vertex; \n"
        "in vec3 osg_Normal; \n"
        "out vec4 color; \n"
        " \n"
        "void main() \n"
        "{ \n"
        "    vec3 ecNormal = normalize( osg_NormalMatrix * osg_Normal ); \n"
        "    float diffuse = max( dot( ecLightDir, ecNormal ), 0. ); \n"
        "    color = vec4( vec3( diffuse ), 1. ); \n"
        " \n"
        "    gl_Position = osg_ModelViewProjectionMatrix * osg_Vertex; \n"
        "} \n";
    osg::Shader* vShader = new osg::Shader( osg::Shader::VERTEX, vertexSource );

    const std::string fragmentSource =
        "#version 140 \n"
        " \n"
        "in vec4 color; \n"
        "out vec4 fragData; \n"
        " \n"
        "void main() \n"
        "{ \n"
        "    fragData = color; \n"
        "} \n";
    osg::Shader* fShader = new osg::Shader( osg::Shader::FRAGMENT, fragmentSource );

    osg::Program* program = new osg::Program;
    program->addShader( vShader );
    program->addShader( fShader );
    stateSet->setAttribute( program );

    osg::Vec3f lightDir( 0., 0.5, 1. );
    lightDir.normalize();
    stateSet->addUniform( new osg::Uniform( "ecLightDir", lightDir ) );
}

int main( int argc, char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );

    osg::Node* root = osgDB::readNodeFiles( arguments );
    if( root == NULL )
    {
        osg::notify( osg::FATAL ) << "Unable to load model from command line." << std::endl;
        return( 1 );
    }
    configureShaders( root->getOrCreateStateSet() );

    const int width( 800 ), height( 450 );
    const std::string version( "3.1" );
    osg::ref_ptr< osg::GraphicsContext::Traits > traits = new osg::GraphicsContext::Traits();
    traits->x = 20; traits->y = 30;
    traits->width = width; traits->height = height;
    traits->windowDecoration = true;
    traits->doubleBuffer = true;
    traits->glContextVersion = version;
    osg::ref_ptr< osg::GraphicsContext > gc = osg::GraphicsContext::createGraphicsContext( traits.get() );
    if( !gc.valid() )
    {
        osg::notify( osg::FATAL ) << "Unable to create OpenGL v" << version << " context." << std::endl;
        return( 1 );
    }

    osgViewer::Viewer viewer;

    // Create a Camera that uses the above OpenGL context.
    osg::Camera* cam = viewer.getCamera();
    cam->setGraphicsContext( gc.get() );
    // Must set perspective projection for fovy and aspect.
    cam->setProjectionMatrix( osg::Matrix::perspective( 30., (double)width/(double)height, 1., 100. ) );
    // Unlike OpenGL, OSG viewport does *not* default to window dimensions.
    cam->setViewport( new osg::Viewport( 0, 0, width, height ) );

    viewer.setSceneData( root );

    // for non GL3/GL4 and non GLES2 platforms we need enable the osg_ uniforms that the shaders will use,
    // you don't need thse two lines on GL3/GL4 and GLES2 specific builds as these will be enable by default.
    gc->getState()->setUseModelViewAndProjectionUniforms(true);
    gc->getState()->setUseVertexAttributeAliasing(true);

    return( viewer.run() );
}

/*

Building OSG for OpenGL 3.x

OSG currently support OpenGL 3.x on Windows. This comment block describes the
necessary configuration steps.

Get the draft gl3.h header file from OpenGL.org and put it in a folder called
“GL3” somewhere on your hard drive. OSG includes this header as <GL3/gl3.h>. Get
gl3.h from here:
http://www.opengl.org/registry/

Open the cmake-gui and load OSG's top-level CmakeLists.txt. You'll need to make
several changes.

 * Add the path to <GL3/gl3.h> to the CMake compiler flags, CMAKE_CXX_FLAGS and
   CMAKE_CXX_FLAGS_DEBUG (for release and debug builds; others if you use other
   build configurations). The text to add should look something like this:
     /I “C:\GLHeader”
   The folder GLHeader should contain a subfolder GL3, which in turn contains
   gl3.h.

 * Enable the following CMake variable:
     OSG_GL3_AVAILABLE

 * Disable the following CMake variables:
     OSG_GL1_AVAILABLE
     OSG_GL2_AVAILABLE
     OSG_GLES1_AVAILABLE
     OSG_GLES2_AVAILABLE
     OSG_GL_DISPLAYLISTS_AVAILABLE
     OSG_GL_FIXED_FUNCTION_AVAILABLE
     OSG_GL_MATRICES_AVAILABLE
     OSG_GL_VERTEX_ARRAY_FUNCS_AVAILABLE
     OSG_GL_VERTEX_FUNCS_AVAILABLE

Create your project files in cmake-gui as usual, and build OSG as usual.

If you have an external project that will depend on OSG built for OpenGL 3.x,
you'll need to ensure your external project also uses the compiler include
directives to find <GL3/gl3.h>.

To berify your application is using a pure OpenGL 3.x context, set
OSG_NOTIFY_LEVEL=INFO in the environment and check the console output. Context
creation displays output such as the following:
    GL3: Attempting to create OpenGL3 context.
    GL3: version: 3.1
    GL3: context flags: 0
    GL3: profile: 0
    GL3: context created successfully.

When your app begins rendering, it displays information about the actual context
it is using:
    glVersion=3.1, isGlslSupported=YES, glslLanguageVersion=1.4

*/
