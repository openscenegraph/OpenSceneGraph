/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2004 Robert Osfield 
 * Copyright (C) 2003-2004 3Dlabs Inc. Ltd.
 *
 * This application is open source and may be redistributed and/or modified   
 * freely and without restriction, both in commericial and non commericial applications,
 * as long as this copyright notice is maintained.
 * 
 * This application is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

/* file:	examples/osgshaders/GL2Scene.cpp
 * author:	Mike Weiblen 2004-11-09
 *
 * Compose a scene of several instances of a model, with a different
 * OpenGL Shading Language shader applied to each.
 *
 * See http://www.3dlabs.com/opengl2/ for more information regarding
 * the OpenGL Shading Language.
*/

#include <osg/ShapeDrawable>
#include <osg/PositionAttitudeTransform>
#include <osg/Geode>
#include <osg/Node>
#include <osg/Material>
#include <osg/Notify>
#include <osg/Vec3>
#include <osg/Texture1D>
#include <osg/Texture2D>
#include <osg/Texture3D>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osgUtil/Optimizer>
#include <osgGL2/ProgramObject>

#include "GL2Scene.h"
#include "Noise.h"

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

static osg::Image*
make3DNoiseImage(int texSize)
{
    osg::Image* image = new osg::Image;
    image->setImage(texSize, texSize, texSize,
	    4, GL_RGBA, GL_UNSIGNED_BYTE,
	    new unsigned char[4 * texSize * texSize * texSize],
	    osg::Image::USE_NEW_DELETE);

    const int startFrequency = 4;
    const int numOctaves = 4;

    int f, i, j, k, inc;
    double ni[3];
    double inci, incj, inck;
    int frequency = startFrequency;
    GLubyte *ptr;
    double amp = 0.5;

    osg::notify(osg::INFO) << "creating 3D noise texture... ";

    for (f = 0, inc = 0; f < numOctaves; ++f, frequency *= 2, ++inc, amp *= 0.5)
    {
	SetNoiseFrequency(frequency);
	ptr = image->data();
	ni[0] = ni[1] = ni[2] = 0;

	inci = 1.0 / (texSize / frequency);
	for (i = 0; i < texSize; ++i, ni[0] += inci)
	{
	    incj = 1.0 / (texSize / frequency);
	    for (j = 0; j < texSize; ++j, ni[1] += incj)
	    {
		inck = 1.0 / (texSize / frequency);
		for (k = 0; k < texSize; ++k, ni[2] += inck, ptr += 4)
		{
		    *(ptr+inc) = (GLubyte) (((noise3(ni) + 1.0) * amp) * 128.0);
		}
	    }
	}
    }

    osg::notify(osg::INFO) << "DONE" << std::endl;
    return image;	
}

static osg::Texture3D*
make3DNoiseTexture(int texSize )
{
    osg::Texture3D* noiseTexture = new osg::Texture3D;
    noiseTexture->setFilter(osg::Texture3D::MIN_FILTER, osg::Texture3D::LINEAR);
    noiseTexture->setFilter(osg::Texture3D::MAG_FILTER, osg::Texture3D::LINEAR);
    noiseTexture->setWrap(osg::Texture3D::WRAP_S, osg::Texture3D::REPEAT);
    noiseTexture->setWrap(osg::Texture3D::WRAP_T, osg::Texture3D::REPEAT);
    noiseTexture->setWrap(osg::Texture3D::WRAP_R, osg::Texture3D::REPEAT);
    noiseTexture->setImage( make3DNoiseImage(texSize) );
    return noiseTexture;
}

///////////////////////////////////////////////////////////////////////////

static osg::Image*
make1DSineImage( int texSize )
{
    const float PI = 3.1415927;

    osg::Image* image = new osg::Image;
    image->setImage(texSize, 1, 1,
	    4, GL_RGBA, GL_UNSIGNED_BYTE,
	    new unsigned char[4 * texSize],
	    osg::Image::USE_NEW_DELETE);

    GLubyte* ptr = image->data();
    float inc = 2. * PI / (float)texSize;
    for(int i = 0; i < texSize; i++)
    {
	*ptr++ = (GLubyte)((sinf(i * inc) * 0.5 + 0.5) * 255.);
	*ptr++ = 0;
	*ptr++ = 0;
	*ptr++ = 1;
    }
    return image;	
}

static osg::Texture1D*
make1DSineTexture( int texSize )
{
    osg::Texture1D* sineTexture = new osg::Texture1D;
    sineTexture->setWrap(osg::Texture1D::WRAP_S, osg::Texture1D::REPEAT);
    sineTexture->setFilter(osg::Texture1D::MIN_FILTER, osg::Texture1D::LINEAR);
    sineTexture->setFilter(osg::Texture1D::MAG_FILTER, osg::Texture1D::LINEAR);
    sineTexture->setImage( make1DSineImage(texSize) );
    return sineTexture;
}

///////////////////////////////////////////////////////////////////////////
// OpenGL Shading Language source code for the "microshader" example,
// which simply colors a fragment based on its location.

static const char *microshaderVertSource = {
    "varying vec4 color;"
    "void main(void)"
    "{"
	"color = gl_Vertex;"
	"gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;"
    "}"
};

static const char *microshaderFragSource = {
    "varying vec4 color;"
    "void main(void)"
    "{"
	"gl_FragColor = clamp( color, 0.0, 1.0 );"
    "}"
};

///////////////////////////////////////////////////////////////////////////

static osg::ref_ptr<osg::Group> rootNode;

// Create some geometry upon which to render GL2 shaders.
static osg::Geode*
CreateModel()
{
    osg::Geode* geode = new osg::Geode();
    geode->addDrawable(new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(0.0f,0.0f,0.0f),1.0f)));
    geode->addDrawable(new osg::ShapeDrawable(new osg::Cone(osg::Vec3(2.2f,0.0f,-0.4f),0.9f,1.8f)));
    geode->addDrawable(new osg::ShapeDrawable(new osg::Cylinder(osg::Vec3(4.4f,0.0f,0.0f),1.0f,1.4f)));
    return geode;
}

// Add a reference to the masterModel at the specified translation, and
// return its StateSet so we can easily attach StateAttributes.
static osg::StateSet*
ModelInstance()
{
    static float zvalue = 0.0f;
    static osg::Node* masterModel = CreateModel();

    osg::PositionAttitudeTransform* xform = new osg::PositionAttitudeTransform();
    xform->setPosition(osg::Vec3( 0.0f, -1.0f, zvalue ));
    zvalue = zvalue + 2.2f;
    xform->addChild(masterModel);
    rootNode->addChild(xform);
    return xform->getOrCreateStateSet();
}

// load source from a file.
static void
LoadShaderSource( osgGL2::ShaderObject* obj, const std::string& fileName )
{
    std::string fqFileName = osgDB::findDataFile(fileName);
    if( fqFileName.length() != 0 )
    {
	obj->loadShaderSourceFromFile( fqFileName.c_str() );
    }
    else
    {
	osg::notify(osg::WARN) << "File \"" << fileName << "\" not found." << std::endl;
    }
}


///////////////////////////////////////////////////////////////////////////
// rude but convenient globals

static osgGL2::ProgramObject* BlockyProgObj;
static osgGL2::ShaderObject*  BlockyVertObj;
static osgGL2::ShaderObject*  BlockyFragObj;

static osgGL2::ProgramObject* ErodedProgObj;
static osgGL2::ShaderObject*  ErodedVertObj;
static osgGL2::ShaderObject*  ErodedFragObj;

static osgGL2::ProgramObject* MarbleProgObj;
static osgGL2::ShaderObject*  MarbleVertObj;
static osgGL2::ShaderObject*  MarbleFragObj;

///////////////////////////////////////////////////////////////////////////
// for demo simplicity, this one callback animates all the shaders.

class AnimateCallback: public osg::NodeCallback
{
    public:
	AnimateCallback() : osg::NodeCallback(), _enabled(true) {}

	virtual void operator() ( osg::Node* node, osg::NodeVisitor* nv )
	{
	    if( _enabled )
	    {
		float angle = 2.0 * nv->getFrameStamp()->getReferenceTime();
		float sine = sinf( angle );	// -1 -> 1
		float v01 = 0.5f * sine + 0.5f;	//  0 -> 1
		float v10 = 1.0f - v01;		//  1 -> 0

		ErodedProgObj->setUniform( "Offset", osg::Vec3(0.505f, 0.8f*v01, 0.0f) );

		MarbleProgObj->setUniform( "Offset", osg::Vec3(0.505f, 0.8f*v01, 0.0f) );

		BlockyProgObj->setUniform( "Sine", sine );
		BlockyProgObj->setUniform( "Color1", osg::Vec3(v10, 0.0f, 0.0f) );
		BlockyProgObj->setUniform( "Color2", osg::Vec3(v01, v01, v10) );
	    }
	    traverse(node, nv);
	}

    private:
	bool _enabled;
};

///////////////////////////////////////////////////////////////////////////
// Compose a scenegraph with examples of GL2 shaders

#define TEXUNIT_SINE	1
#define TEXUNIT_NOISE	2

osg::ref_ptr<osg::Group>
GL2Scene::buildScene()
{
    osg::Texture3D* noiseTexture = make3DNoiseTexture( 32 /*128*/ );
    osg::Texture1D* sineTexture = make1DSineTexture( 32 /*1024*/ );

    // the root of our scenegraph.
    rootNode = new osg::Group;
    rootNode->setUpdateCallback( new AnimateCallback );

    // the simple Microshader (its source appears earlier in this file)
    {
	osg::StateSet* ss = ModelInstance();
	osgGL2::ProgramObject* progObj = new osgGL2::ProgramObject;
	_progObjList.push_back( progObj );
	progObj->addShader( new osgGL2::ShaderObject(
		    osgGL2::ShaderObject::VERTEX, microshaderVertSource ) );
	progObj->addShader( new osgGL2::ShaderObject(
		    osgGL2::ShaderObject::FRAGMENT, microshaderFragSource ) );
	ss->setAttributeAndModes( progObj, osg::StateAttribute::ON );
    }

    // the "blocky" shader, a simple animation test
    {
	osg::StateSet* ss = ModelInstance();
	BlockyProgObj = new osgGL2::ProgramObject;
	_progObjList.push_back( BlockyProgObj );
	BlockyVertObj = new osgGL2::ShaderObject( osgGL2::ShaderObject::VERTEX );
	BlockyFragObj = new osgGL2::ShaderObject( osgGL2::ShaderObject::FRAGMENT );
	BlockyProgObj->addShader( BlockyFragObj );
	BlockyProgObj->addShader( BlockyVertObj );
	ss->setAttributeAndModes(BlockyProgObj, osg::StateAttribute::ON);
    }

    // the "eroded" shader, uses a noise texture to discard fragments
    {
	osg::StateSet* ss = ModelInstance();
	ss->setTextureAttribute(TEXUNIT_NOISE, noiseTexture);
	ErodedProgObj = new osgGL2::ProgramObject;
	_progObjList.push_back( ErodedProgObj );
	ErodedVertObj = new osgGL2::ShaderObject( osgGL2::ShaderObject::VERTEX );
	ErodedFragObj = new osgGL2::ShaderObject( osgGL2::ShaderObject::FRAGMENT );
	ErodedProgObj->addShader( ErodedFragObj );
	ErodedProgObj->addShader( ErodedVertObj );
	ss->setAttributeAndModes(ErodedProgObj, osg::StateAttribute::ON);
    }

    // the "marble" shader, uses two textures
    {
	osg::StateSet* ss = ModelInstance();
	ss->setTextureAttribute(TEXUNIT_NOISE, noiseTexture);
	ss->setTextureAttribute(TEXUNIT_SINE, sineTexture);
	MarbleProgObj = new osgGL2::ProgramObject;
	_progObjList.push_back( MarbleProgObj );
	MarbleVertObj = new osgGL2::ShaderObject( osgGL2::ShaderObject::VERTEX );
	MarbleFragObj = new osgGL2::ShaderObject( osgGL2::ShaderObject::FRAGMENT );
	MarbleProgObj->addShader( MarbleFragObj );
	MarbleProgObj->addShader( MarbleVertObj );
	ss->setAttributeAndModes(MarbleProgObj, osg::StateAttribute::ON);
    }

#ifdef INTERNAL_3DLABS //[
    // regular GL 1.x texturing for comparison.
    osg::StateSet* ss = ModelInstance();
    osg::Texture2D* tex0 = new osg::Texture2D;
    tex0->setImage( osgDB::readImageFile( "images/3dl-ge100.png" ) );
    ss->setTextureAttributeAndModes(0, tex0, osg::StateAttribute::ON);
#endif //]

    reloadShaderSource();

#ifdef INTERNAL_3DLABS //[
    // add logo overlays
    rootNode->addChild( osgDB::readNodeFile( "3dl_ogl.logo" ) );
#endif //]

    return rootNode;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

GL2Scene::GL2Scene()
{
    _rootNode = buildScene();
    _shadersEnabled = true;
}

GL2Scene::~GL2Scene()
{
}

// mew 2003-09-19 : This way of configuring the shaders is temporary,
// pending a move to an osgFX-based approach.
void
GL2Scene::reloadShaderSource()
{
    osg::notify(osg::WARN) << "reloadShaderSource()" << std::endl;

    LoadShaderSource( BlockyVertObj, "shaders/blocky.vert" );
    LoadShaderSource( BlockyFragObj, "shaders/blocky.frag" );

    LoadShaderSource( ErodedVertObj, "shaders/eroded.vert" );
    LoadShaderSource( ErodedFragObj, "shaders/eroded.frag" );
    ErodedProgObj->setUniform( "LightPosition", osg::Vec3(0.0f, 0.0f, 4.0f) );
    ErodedProgObj->setUniform( "Scale", 1.0f );
    ErodedProgObj->setSampler( "sampler3d", TEXUNIT_NOISE );

    LoadShaderSource( MarbleVertObj, "shaders/marble.vert" );
    LoadShaderSource( MarbleFragObj, "shaders/marble.frag" );
    MarbleProgObj->setSampler( "Noise", TEXUNIT_NOISE );
    MarbleProgObj->setSampler( "Sine", TEXUNIT_SINE );
}


// mew 2003-09-19 : TODO Need to revisit how to better control
// osgGL2::ProgramObject enable state in OSG core.  glProgramObjects are
// different enough from other GL state that StateSet::setAttributeAndModes()
// doesn't fit well, so came up with a local implementation.
void
GL2Scene::toggleShaderEnable()
{
    _shadersEnabled = ! _shadersEnabled;
    osg::notify(osg::WARN) << "shader enable = " <<
	    ((_shadersEnabled) ? "ON" : "OFF") << std::endl;
    for( unsigned int i = 0; i < _progObjList.size(); i++ )
    {
	_progObjList[i]->enable( _shadersEnabled );
    }
}

/*EOF*/

