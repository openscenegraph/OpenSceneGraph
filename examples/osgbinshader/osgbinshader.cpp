

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
#include "osgGA/NodeTrackerManipulator"
#include "osg/MatrixTransform"
#include "osg/LineWidth"
#include "osg/ShapeDrawable"
#include "osg/Texture2D"


#pragma region uniform_callback

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


static void usage(const char* prog, const char* msg)
{
	if (msg)
	{
		osg::notify(osg::NOTICE) << std::endl;
		osg::notify(osg::NOTICE) << msg << std::endl;
	}

	// basic usage
	osg::notify(osg::NOTICE) << std::endl;
	osg::notify(osg::NOTICE) << "example usage:" << std::endl;
	osg::notify(osg::NOTICE) << "    " << prog << "<path/to/model> -b -m <path/to/shader.vert> <path/to/shader.frag> -l <path/to/shader.vert> <path/to/shader.frag> --tex <path/to/texture>" << std::endl;
	osg::notify(osg::NOTICE) << "    " << prog << "for example:\n\t../../../examples/osgbinshader/cube.obj\n\t-b\n\t-l ../../../examples/osgbinshader/pass_through.vert.spv ../../../examples/osgbinshader/pass_through.frag.spv\n\t-m ../../../examples/osgbinshader/shader.vert.spv ../../../examples/osgbinshader/shader.frag.spv\n\t--tex ../../../examples/osgbinshader/rockwall.png" << std::endl;	
	osg::notify(osg::NOTICE) << std::endl;

}
int main(int argc, char** argv)
{
	osg::ArgumentParser arguments(&argc, argv);
	
	if (arguments.argc() <= 1 || arguments.read("-h") || arguments.read("--help"))
	{
		osg::setNotifyLevel(osg::NOTICE);
		usage(arguments.getApplicationName().c_str(), 0);
		return 1;
	}

	osg::ref_ptr<osg::Group> root = new osg::Group;
	osgViewer::Viewer viewer;

	bool isBinaryShader = false;
	std::string lampVertSourcePath;
	std::string lampFragSourcePath;
	std::string modelVertSourcePath;
	std::string modelFragtSourcePath;
	std::string img_texture_path;

	if (arguments.read("-b") || arguments.read("--use-binary"))
	{
		isBinaryShader = true;
	}
	
	arguments.read("--model-shader", modelVertSourcePath, modelFragtSourcePath) || arguments.read("-m", modelVertSourcePath, modelFragtSourcePath);
	arguments.read("--lamp-shader", lampVertSourcePath, lampFragSourcePath) || arguments.read("-l", lampVertSourcePath, lampFragSourcePath);
	arguments.read("--tex", img_texture_path) || arguments.read("-t", img_texture_path);

	osg::ref_ptr<osg::Node> loadedModel = osgDB::readRefNodeFiles(arguments);
	if (!loadedModel)
	{
		std::cout << arguments.getApplicationName() << ": No data loaded" << std::endl;
		return 1;
	}

	osg::Image* image = osgDB::readImageFile(img_texture_path);
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
	loadShadersFiles(loadedModel->getOrCreateStateSet(), modelVertSourcePath, modelFragtSourcePath, isBinaryShader);
	
	// axis
	osg::Geode* axis = new osg::Geode();
	axis->addDrawable(createAxis(
		osg::Vec3(0.0f, 0.0f, 0.0f),
		osg::Vec3(5.0f, 0.0f, 0.0f),
		osg::Vec3(0.0f, 5.0f, 0.0f),
		osg::Vec3(0.0f, 0.0f, 5.0f)));
	loadShadersFiles(axis->getOrCreateStateSet(), lampVertSourcePath, lampFragSourcePath, isBinaryShader);

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
		loadShadersFiles(lightTransform->getOrCreateStateSet(), lampVertSourcePath, lampFragSourcePath, isBinaryShader);

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
