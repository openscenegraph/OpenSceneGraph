// This is public domain software and comes with
// absolutely no warranty. Use of public domain software
// may vary between counties, but in general you are free
// to use and distribute this software for any purpose.


// Example: OSG using EGL.
// The comment block at the end of the source describes building OSG
// for use with OpenGL 3.x.

#include <osgViewer/Viewer>
#include <osg/GraphicsContext>
#include <osg/Camera>
#include <osg/Viewport>
#include <osg/StateSet>
#include <osg/Program>
#include <osg/Shader>

osg::Geode* makeGeometry(float v)
{
	osg::Geode* geode = new osg::Geode();
	osg::Geometry* geom = new osg::Geometry();
	osg::Vec3Array* verts = new osg::Vec3Array();
	verts->push_back(osg::Vec3(v - 1, 0, 0));
	verts->push_back(osg::Vec3(v + 1, 0, 0));
	verts->push_back(osg::Vec3(v, 0, 2));
	geom->setVertexArray(verts);
	geom->setUseVertexBufferObjects(true);
	osg::Vec4Array* colors = new osg::Vec4Array(osg::Array::BIND_OVERALL);
	colors->push_back(osg::Vec4(0, 0, 1, 1));
	geom->setColorArray(colors);
	geom->addPrimitiveSet(new osg::DrawArrays(GL_TRIANGLES, 0, 3));
	geode->addDrawable(geom);
	return geode;
}

int main( int argc, char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );
	osgViewer::Viewer viewer(arguments);
	viewer.setThreadingModel(osgViewer::Viewer::SingleThreaded);

	osg::Geode* geode = makeGeometry(1.0);

	const std::string vertexSource =
		"#version 300 es\n"
		"void main() {                                              \n"
		"  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;  \n"
		"}                                                          \n";

	osg::Shader* vShader = new osg::Shader(osg::Shader::VERTEX, vertexSource);

	const std::string fragmentSource =
		"#version 300 es\n"
		"precision mediump float;\n"
		"out vec4 color;\n"
		"void main() {                             \n"
		"  color = vec4(0.5, 0.3, 0.3, 1.0);\n"
		"}                                         \n";

	osg::Shader* fShader = new osg::Shader(osg::Shader::FRAGMENT, fragmentSource);

	osg::Program* program = new osg::Program;
	program->addShader(vShader);
	program->addShader(fShader);
	geode->getOrCreateStateSet()->setAttributeAndModes(program, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);

	osg::Group* root = new osg::Group();
	root->addChild(geode);

#if 0
    // Create a Camera that uses the above OpenGL context.
    osg::Camera* cam = viewer.getCamera();
    // Must set perspective projection for fovy and aspect.
    cam->setProjectionMatrix( osg::Matrix::perspective( 30., (double)width/(double)height, 1., 100. ) );
    // Unlike OpenGL, OSG viewport does *not* default to window dimensions.
    cam->setViewport( new osg::Viewport( 0, 0, width, height ) );
#endif
    viewer.setSceneData( root );
	    
    return( viewer.run() );
}

/*

Building OSG for OpenGLES3

OSG currently support GLES3 on Windows. This comment block describes the
necessary configuration steps.

 * Add Google Angle libraries and include folder to CMake
   CMake use FindEGL.cmake to find this. The library name of Google-Angle for Windows is "libEGL.dll.lib"

 * Enable the following CMake variable:
     OSG_GLES3_AVAILABLE

 * Disable the following CMake variables:
     OSG_GL1_AVAILABLE
     OSG_GL2_AVAILABLE
     OSG_GLES1_AVAILABLE
     OSG_GLES2_AVAILABLE

*/
