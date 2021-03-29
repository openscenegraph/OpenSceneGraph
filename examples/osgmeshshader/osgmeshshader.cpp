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
#include <osgUtil/Optimizer>

class DrawMeshTasks : public osg::Drawable
{
public:

    DrawMeshTasks() :
        first(0),
        count(0)
    {
    }

    DrawMeshTasks(GLuint in_first, GLuint in_count) :
        first(in_first),
        count(in_count)
    {
    }

    GLuint first;
    GLuint count;

    virtual void drawImplementation(osg::RenderInfo& renderInfo) const
    {
        const osg::GLExtensions* extensions = renderInfo.getState()->get<osg::GLExtensions>();
        if (extensions->isMeshShaderSupported && extensions->glDrawMeshTasksNV)
        {
            extensions->glDrawMeshTasksNV(first, count);
        }
        else
        {
            OSG_NOTICE<<"glDrawMeshTasksNV not supported. "<<std::endl;
        }
    }
};

int main( int argc, char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );


    static const char* meshSource = \
        "#version 450 \n"
        "#extension GL_NV_mesh_shader : enable\n"
        "layout(local_size_x = 3) in;"
        "layout(max_vertices = 64) out;"
        "layout(max_primitives = 126) out;"
        "layout(triangles) out;"
        "const vec3 vertices[3] = {vec3(-1,-1,0), vec3(1,-1,0), vec3(0,1,0)};"
        "void main()"
        "{"
            "uint id = gl_LocalInvocationID.x;"
            "gl_MeshVerticesNV[id].gl_Position = vec4(vertices[id], 2);"
            "gl_PrimitiveIndicesNV[id] = id;"
            "gl_PrimitiveCountNV = 1;"
        "}";


    static const char* fragmentSource = \
        "#version 450 \n"
        "#extension GL_NV_fragment_shader_barycentric : enable\n"
        "out vec4 color;"
        "void main()"
        "{"
            "color = vec4(gl_BaryCoordNV, 1.0);"
        "}";

    osg::ref_ptr<osg::Shader> vShader = new osg::Shader( osg::Shader::MESH, meshSource );
    osg::ref_ptr<osg::Shader> fShader = new osg::Shader( osg::Shader::FRAGMENT, fragmentSource );

    osg::ref_ptr<osg::Program> program = new osg::Program;
    program->addShader( vShader.get() );
    program->addShader( fShader.get() );

    osg::ref_ptr<osg::Node> drawMesh = new DrawMeshTasks(0, 1);
    drawMesh->getOrCreateStateSet()->setAttribute( program.get() );


    const int width( 800 ), height( 450 );
    const std::string version( "4.6" );
    osg::ref_ptr< osg::GraphicsContext::Traits > traits = new osg::GraphicsContext::Traits();
    traits->x = 20; traits->y = 30;
    traits->width = width; traits->height = height;
    traits->windowDecoration = true;
    traits->doubleBuffer = true;
    traits->glContextVersion = version;
    traits->readDISPLAY();
    traits->setUndefinedScreenDetailsToDefaultScreen();
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

    viewer.setSceneData( drawMesh );
    viewer.setLight( 0 );
    viewer.setLightingMode( osg::View::NO_LIGHT );

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

To verify your application is using a pure OpenGL 3.x context, set
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
