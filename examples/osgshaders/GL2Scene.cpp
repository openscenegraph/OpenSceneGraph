/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2003 Robert Osfield 
 * Copyright (C) 2003 3Dlabs Inc. Ltd.
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
 * author:	Mike Weiblen 2003-07-14
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
#include <osg/Notify>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osgUtil/Optimizer>
#include <osgGL2/ProgramObject>


///////////////////////////////////////////////////////////////////////////
// OpenGL Shading Language source code for the "microshader" example.

static const char *microshaderVertSource = {
    "varying vec3 color;"
    "void main(void)"
    "{"
	"gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;"
	"color = gl_Vertex.zyx * 1.0;"
    "}"
};

static const char *microshaderFragSource = {
    "varying vec3 color;"
    "void main(void)"
    "{"
	"gl_FragColor = vec4(color, 1.0);"
    "}"
};

///////////////////////////////////////////////////////////////////////////

static osg::Group* rootNode;
static osg::Node* masterModel;

// Add a reference to the masterModel at the specified translation, and
// return its StateSet so we can easily attach StateAttributes.
static osg::StateSet*
CloneMaster(float x, float y, float z )
{
    osg::PositionAttitudeTransform* xform = new osg::PositionAttitudeTransform();
    xform->setPosition(osg::Vec3(x, y, z));
    xform->addChild(masterModel);
    rootNode->addChild(xform);
    return xform->getOrCreateStateSet();
}

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

// read vert & frag shader source code from a pair of files.
static void
LoadShaderSource( osgGL2::ProgramObject* progObj, std::string baseFileName )
{
    std::string vertFileName = osgDB::findDataFile(baseFileName + ".vert");
    if( vertFileName.length() != 0 )
    {
	osgGL2::ShaderObject* vertObj = new osgGL2::ShaderObject( osgGL2::ShaderObject::VERTEX );
	vertObj->loadShaderSourceFromFile( vertFileName.c_str() );
	progObj->addShader( vertObj );
    }
    else
    {
	osg::notify(osg::WARN) << "Warning: file \"" << baseFileName+".vert" << "\" not found." << std::endl;
    }

    std::string fragFileName = osgDB::findDataFile(baseFileName + ".frag");
    if( fragFileName.length() != 0 )
    {
	osgGL2::ShaderObject* fragObj = new osgGL2::ShaderObject( osgGL2::ShaderObject::FRAGMENT );
	fragObj->loadShaderSourceFromFile( fragFileName.c_str() );
	progObj->addShader( fragObj );
    }
    else
    {
	osg::notify(osg::WARN) << "Warning: file \"" << baseFileName+".frag" << "\" not found." << std::endl;
    }
}

///////////////////////////////////////////////////////////////////////////
// Compose a scenegraph with examples of GL2 shaders

#define ZGRID 2.2

osg::Node*
GL2Scene()
{
    osg::StateSet* ss;
    osgGL2::ProgramObject* progObj;

    // the rootNode of our created graph.
    rootNode = new osg::Group;
    ss = rootNode->getOrCreateStateSet();

    // attach an "empty" ProgramObject to the rootNode as a default
    // StateAttribute.  An empty ProgramObject (ie without any attached
    // ShaderObjects) is a special case, which means to use the
    // OpenGL 1.x "fixed functionality" rendering pipeline.
    progObj = new osgGL2::ProgramObject;
    ss->setAttributeAndModes(progObj, osg::StateAttribute::ON);

    // put the unadorned masterModel at the origin for comparison.
    masterModel = CreateModel();
    rootNode->addChild(masterModel);

    // add logo overlays
    //rootNode->addChild( osgDB::readNodeFile( "3dl_ogl.logo" ) );

    //
    // create references to the masterModel and attach shaders
    //

    // apply the simple microshader example
    // (the shader sources are hardcoded above in this .cpp file)
    ss = CloneMaster(0,0,ZGRID*1);
    progObj = new osgGL2::ProgramObject;
    progObj->addShader( new osgGL2::ShaderObject( osgGL2::ShaderObject::VERTEX, microshaderVertSource ) );
    progObj->addShader( new osgGL2::ShaderObject( osgGL2::ShaderObject::FRAGMENT, microshaderFragSource ) );
    ss->setAttributeAndModes(progObj, osg::StateAttribute::ON);

    // load the "specular brick" shader from a pair of source files.
    ss = CloneMaster(0,0,ZGRID*2);
    progObj = new osgGL2::ProgramObject;
    LoadShaderSource( progObj, "shaders/brick" );
    ss->setAttributeAndModes(progObj, osg::StateAttribute::ON);

    // load the "gold screen" shader from a pair of source files.
    ss = CloneMaster(0,0,ZGRID*3);
    progObj = new osgGL2::ProgramObject;
    LoadShaderSource( progObj, "shaders/screen" );
    ss->setAttributeAndModes(progObj, osg::StateAttribute::ON);

    return rootNode;
}


void
GL2Update()
{
    /* TODO : update uniform values for shader animation */
}

/*EOF*/

