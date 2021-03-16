

// This is public domain software and comes with
// absolutely no warranty. Use of public domain software
// may vary between counties, but in general you are free
// to use and distribute this software for any purpose.


// Example: OSG using an OpenGL 4.3 context.
// This sample demonstrate how to use  binary shaders

/*
run glslangValidator in order to create binary shaders out of source files:
glslangValidator.exe -G shader.frag -o shader_frag.spv
glslangValidator.exe -G shader.vert -o shader_vert.spv
glslangValidator.exe -G pass_through.vert -o pass_through_vert.spv
glslangValidator.exe -G pass_through.frag -o pass_through_frag.spv
pause
*/
#include <osgViewer/Viewer>
#include <osgDB/ReadFile>
#include <osg/GraphicsContext>
#include <osg/Camera>
#include <osg/Viewport>
#include <osg/StateSet>
#include <osg/Program>
#include <osg/Shader>
#include "C:/Program Files/NVIDIA Corporation/NvToolsExt/include/nvToolsExt.h"
#include "osgGA/NodeTrackerManipulator"
#include "osg/MatrixTransform"
#include "osg/LineWidth"
#include "osg/ShapeDrawable"
#include "osg/Texture2D"

#pragma uniform_callback

struct NormalMatrixCallback : public osg::Uniform::Callback {
	NormalMatrixCallback(osg::Camera* camera) :
		_camera(camera) {
	}

	virtual void operator()(osg::Uniform* uniform, osg::NodeVisitor* nv) {		
		osg::Matrixd viewMatrix = _camera->getViewMatrix();
		osg::Matrixd modelMatrix = osg::computeLocalToWorld(nv->getNodePath());

		
		osg::Matrixd normalMatrix = osg::Matrixd::inverse(osg::Matrixd::transpose4x4(modelMatrix));
		uniform->set(normalMatrix);
	}

	osg::Camera* _camera;
};


struct ModelMatrixCallback : public osg::Uniform::Callback {
	ModelMatrixCallback(osg::Camera* camera) :
		_camera(camera) {
	}

	virtual void operator()(osg::Uniform* uniform, osg::NodeVisitor* nv) {

		osg::Matrixd viewMatrix = _camera->getViewMatrix();
		osg::Matrixd modelMatrix = osg::computeLocalToWorld(nv->getNodePath());
		uniform->set(modelMatrix);

	}

	osg::Camera* _camera;
};

#pragma endregion 

struct LightSource
{
	osg::Vec3f position;
	osg::Vec3f color;
};


void loadShadersFiles(osg::StateSet* stateSet, std::string vertSourcePath, std::string fragSourcePath, bool isBinaryShader = true)
{

	osg::Program* program = new osg::Program;

	if (isBinaryShader)
	{
		program->addShader(new osg::Shader(osg::Shader::VERTEX,
			osg::ShaderBinary::readShaderBinaryFile(vertSourcePath.c_str())));

		program->addShader(new osg::Shader(osg::Shader::FRAGMENT,
			osg::ShaderBinary::readShaderBinaryFile(fragSourcePath.c_str())));
	}
	else 
	{
		program->addShader(osg::Shader::readShaderFile(osg::Shader::VERTEX, vertSourcePath.c_str()));
		program->addShader(osg::Shader::readShaderFile(osg::Shader::FRAGMENT, fragSourcePath.c_str()));
	}

	stateSet->setAttribute(program);

}



void loadSourceShaders(osg::StateSet* stateSet, const char* vertSource, const char* fragSource)
{

	osg::Shader* vShader = new osg::Shader(osg::Shader::VERTEX, vertSource);
	osg::Shader* fShader = new osg::Shader(osg::Shader::FRAGMENT, fragSource);

	osg::Program* program = new osg::Program;

	program->addShader(vShader);
	program->addShader(fShader);

	stateSet->setAttribute(program);

}

osg::Drawable* createAxis(const osg::Vec3& corner, const osg::Vec3& xdir, const osg::Vec3& ydir, const osg::Vec3& zdir)
{// set up the Geometry.
	osg::Geometry* geom = new osg::Geometry;

	osg::Vec3Array* coords = new osg::Vec3Array(6);
	(*coords)[0] = corner;
	(*coords)[1] = corner + xdir;
	(*coords)[2] = corner;
	(*coords)[3] = corner + ydir;
	(*coords)[4] = corner;
	(*coords)[5] = corner + zdir;

	geom->setVertexArray(coords);

	osg::Vec4 x_color(1.0f, 0.5f, 0.5f, 1.0f);
	osg::Vec4 y_color(0.5f, 1.0f, 0.5f, 1.0f);
	osg::Vec4 z_color(0.5f, 0.5f, 1.0f, 1.0f); // color

	osg::Vec4Array* color = new osg::Vec4Array(6);
	(*color)[0] = x_color;
	(*color)[1] = x_color;
	(*color)[2] = y_color;
	(*color)[3] = y_color;
	(*color)[4] = z_color;
	(*color)[5] = z_color;

	geom->setColorArray(color, osg::Array::BIND_PER_VERTEX);

	geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES, 0, 6));

	osg::StateSet* stateset = new osg::StateSet;
	osg::LineWidth* linewidth = new osg::LineWidth();
	linewidth->setWidth(2.0f);
	stateset->setAttributeAndModes(linewidth, osg::StateAttribute::ON);
	stateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	geom->setStateSet(stateset);

	return geom;
}

int main(int argc, char** argv)
{
	osg::ArgumentParser arguments(&argc, argv);
	osg::ref_ptr<osg::Group> root = new osg::Group;
	osgViewer::Viewer viewer;

	bool isBinaryShader = true;
	std::string passthroughVertSourcePath;
	std::string passthroughFragSourcePath;
	std::string modelVertSourcePath;
	std::string modelFragtSourcePath;

	osg::ref_ptr<osg::Node> loadedModel =
		osgDB::readRefNodeFile("../../../examples/osgbinshader/cube.obj");
	if (loadedModel == NULL)
	{
		osg::notify(osg::FATAL) << "Unable to load model from command line." << std::endl;
		return(1);
	}
	
	osg::Image* image = osgDB::readImageFile("../../../examples/osgbinshader/rockwall.png");
	if (image)
	{
		osg::Texture2D* txt = new osg::Texture2D;
		txt->setImage(image);
		txt->setWrap(osg::Texture::WRAP_S, osg::Texture::REPEAT);
		txt->setWrap(osg::Texture::WRAP_T, osg::Texture::REPEAT);
		txt->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
		txt->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
		loadedModel->getOrCreateStateSet()->setTextureAttributeAndModes(0, txt, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

		osg::Uniform* baseTextureSampler = new osg::Uniform("baseTexture", 0);
		loadedModel->getOrCreateStateSet()->addUniform(baseTextureSampler);
	}
	

	osg::ref_ptr<osg::MatrixTransform> modelTransform = new osg::MatrixTransform;
	modelTransform->setMatrix(osg::Matrix::scale(1.0f, 1.0f, 1.0f) * osg::Matrix::translate(0.0f, 0.0f, 0.0f));
	modelTransform->addChild(loadedModel);

	if (isBinaryShader)
	{
		passthroughVertSourcePath = "../../../examples/osgbinshader/pass_through_vert.spv";
		passthroughFragSourcePath = "../../../examples/osgbinshader/pass_through_frag.spv";
		modelVertSourcePath = "../../../examples/osgbinshader/shader_vert.spv";
		modelFragtSourcePath = "../../../examples/osgbinshader/shader_frag.spv";
	}
	else
	{
		// must have absulute path
		passthroughVertSourcePath = "../../../examples/osgbinshader/pass_through.vert";
		passthroughFragSourcePath = "../../../examples/osgbinshader/pass_through.frag";
		modelVertSourcePath = "../../../examples/osgbinshader/shader.vert";
		modelFragtSourcePath = "../../../examples/osgbinshader/shader.frag";
	}

	loadShadersFiles(loadedModel->getOrCreateStateSet(), modelVertSourcePath, modelFragtSourcePath, isBinaryShader);
	
	// axis
	osg::Geode* axis = new osg::Geode();
	axis->addDrawable(createAxis(
		osg::Vec3(0.0f, 0.0f, 0.0f),
		osg::Vec3(5.0f, 0.0f, 0.0f),
		osg::Vec3(0.0f, 5.0f, 0.0f),
		osg::Vec3(0.0f, 0.0f, 5.0f)));
	loadShadersFiles(axis->getOrCreateStateSet(), passthroughVertSourcePath, passthroughFragSourcePath, isBinaryShader);

	// light		
	LightSource lightSources[2]{};
	lightSources[0].position = osg::Vec3f(-3.0f, 2.0f, .0f);
	lightSources[0].color = osg::Vec3f(1.f, .5f, .0f);
	lightSources[1].position = osg::Vec3f(2.0f, -1.0f, 1.0f);
	lightSources[1].color = osg::Vec3f(.5f, .0f, 1.f);
	int numOfLightSources = sizeof(lightSources) / sizeof(LightSource);

	for (int i = 0; i <= numOfLightSources - 1; i++)
	{
		osg::Geode* lightGeode = new osg::Geode;
		osg::ShapeDrawable* drawable = new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(0.0f, 0.0f, 0.0f), .3f));
		drawable->setColor(osg::Vec4(lightSources[i].color, 1.0f));
		lightGeode->addDrawable(drawable);
		osg::ref_ptr<osg::MatrixTransform> lightTransform = new osg::MatrixTransform;
		lightTransform->setMatrix(osg::Matrix::scale(1.0f, 1.0f, 1.0f) * osg::Matrix::translate(lightSources[i].position));
		lightTransform->addChild(lightGeode);
		loadShadersFiles(lightTransform->getOrCreateStateSet(), passthroughVertSourcePath, passthroughFragSourcePath, isBinaryShader);

		std::string uniformPosName = "lightSource[" + std::to_string(i) + "].position";
		std::string uniformColorName = "lightSource[" + std::to_string(i) + "].color";
		root->getOrCreateStateSet()->addUniform(new osg::Uniform(uniformPosName.c_str(), lightSources[i].position));
		root->getOrCreateStateSet()->addUniform(new osg::Uniform(uniformColorName.c_str(), lightSources[i].color));

		root->addChild(lightTransform);
	}
	osg::Uniform* numOfLightSourcesUniform = new osg::Uniform(osg::Uniform::INT, "numOfLightSources");
	numOfLightSourcesUniform->set(numOfLightSources);
	root->getOrCreateStateSet()->addUniform(numOfLightSourcesUniform);

	const int width(800), height(450);
	const std::string version("4.3");
	osg::ref_ptr< osg::GraphicsContext::Traits > traits = new osg::GraphicsContext::Traits();
	traits->x = 20; traits->y = 30;
	traits->width = width; traits->height = height;
	traits->windowDecoration = true;
	traits->doubleBuffer = true;
	traits->glContextVersion = version;
	osg::ref_ptr< osg::GraphicsContext > gc = osg::GraphicsContext::createGraphicsContext(traits.get());
	if (!gc.valid())
	{
		osg::notify(osg::FATAL) << "Unable to create OpenGL v" << version << " context." << std::endl;
		return(1);
	}

	// Create a Camera that uses the above OpenGL context.
	osg::Camera* cam = viewer.getCamera();
	cam->setClearColor({});

	cam->setGraphicsContext(gc.get());
	// Must set perspective projection for fovy and aspect.
	cam->setProjectionMatrix(osg::Matrix::perspective(30., (double)width / (double)height, 1., 100.));
	// Unlike OpenGL, OSG viewport does *not* default to window dimensions.
	cam->setViewport(new osg::Viewport(0, 0, width, height));


	// adding camera manipulator
	osgGA::NodeTrackerManipulator* nodeTrackerManipulator = new osgGA::NodeTrackerManipulator;
	nodeTrackerManipulator->setTrackerMode(osgGA::NodeTrackerManipulator::NODE_CENTER_AND_ROTATION);
	viewer.setCameraManipulator(nodeTrackerManipulator);


	osg::Uniform* modelMatrix = new osg::Uniform(osg::Uniform::FLOAT_MAT4, "ModelMatrix");
	modelMatrix->setUpdateCallback(new ModelMatrixCallback(cam));
	root->getOrCreateStateSet()->addUniform(modelMatrix);

	osg::Uniform* normalMatrix = new osg::Uniform(osg::Uniform::FLOAT_MAT4, "NormalMatrix");
	normalMatrix->setUpdateCallback(new NormalMatrixCallback(cam));
	root->getOrCreateStateSet()->addUniform(normalMatrix);


	// enable the osg_ uniforms that the shaders will use,	
	gc->getState()->setUseModelViewAndProjectionUniforms(true);
	gc->getState()->setUseVertexAttributeAliasing(true);



	root->addChild(axis);
	root->addChild(modelTransform);



	viewer.setSceneData(root);

	do
	{		
		viewer.frame();		

	} while (!viewer.done());
}
