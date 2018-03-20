/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2010 Robert Osfield
 *
 * This application is open source and may be redistributed and/or modified
 * freely and without restriction, both in commercial and non commercial applications,
 * as long as this copyright notice is maintained.
 *
 * This application is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

#include <osg/TexGen>

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include <osgGA/StateSetManipulator>

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include <iostream>


bool readShaderArguments(osg::ArgumentParser& arguments, const std::string& option, osg::Program* program, const std::string& fallbackShaderFilename)
{
    bool shaderAssigned = false;
    std::string shaderFilename;
    while(arguments.read(option, shaderFilename))
    {
        osg::ref_ptr<osg::Shader> shader = osgDB::readRefShaderFile(shaderFilename);
        if (shader)
        {
            shaderAssigned = true;
            program->addShader(shader);
        }
        else
        {
            OSG_NOTICE<<"Unable to load shader file : "<<shaderFilename<<std::endl;
        }
    }

    if (shaderAssigned) return true;

    osg::ref_ptr<osg::Shader> shader = osgDB::readRefShaderFile(fallbackShaderFilename);
    if (shader)
    {
        shaderAssigned = true;
        program->addShader(shader);
        return true;
    }
    else
    {
        OSG_NOTICE<<"Unable to load shader file : "<<fallbackShaderFilename<<std::endl;
        return false;
    }
}

osg::Image* createFallbackImage()
{
    osg::Image* image = new osg::Image;
    image->allocateImage(1,1,1,GL_RGBA, GL_UNSIGNED_BYTE);
    //image->setColor(osg::Vec4(1.0,1.0,0.0,1.0), 0, 0, 0);
    *(reinterpret_cast<unsigned int*>(image->data())) = 0xffffffff;
    return image;
}

bool setUpStateSet(osg::ArgumentParser& arguments, osg::StateSet* stateset)
{
    osg::ref_ptr<osg::Program> program = new osg::Program;


    if (!readShaderArguments(arguments, "--vert", program.get(), "shaders/shaderpipeline.vert"))
    {
        return false;
    }

    if (!readShaderArguments(arguments, "--frag", program.get(), "shaders/shaderpipeline.frag"))
    {
        return false;
    }

    unsigned int maxTextureUnits = 1;
    while(arguments.read("--units", maxTextureUnits)) {}

    stateset->setAttribute(program);

    std::stringstream sstream;
    sstream<<maxTextureUnits;
    stateset->setDefine("GL_MAX_TEXTURE_UNITS", sstream.str());

    #define ADD_DEFINE(DEF) \
        sstream.str(""); \
        sstream<<DEF; \
        stateset->setDefine(#DEF, sstream.str());

    if (maxTextureUnits>0)
    {
        osg::ref_ptr<osg::Texture2D> fallbackTexture = new osg::Texture2D(createFallbackImage());
        fallbackTexture->setWrap(osg::Texture2D::WRAP_S, osg::Texture2D::CLAMP_TO_EDGE);
        fallbackTexture->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::CLAMP_TO_EDGE);
        fallbackTexture->setWrap(osg::Texture2D::WRAP_R, osg::Texture2D::CLAMP_TO_EDGE);
        fallbackTexture->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
        fallbackTexture->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);
        for(unsigned int i=0; i<maxTextureUnits;++i)
        {
            stateset->setTextureAttribute(i, fallbackTexture.get());
        }

        ADD_DEFINE(GL_ALPHA);
        ADD_DEFINE(GL_INTENSITY);
        ADD_DEFINE(GL_LUMINANCE);
        ADD_DEFINE(GL_RED);
        ADD_DEFINE(GL_RG);
        ADD_DEFINE(GL_RGB);
        ADD_DEFINE(GL_RGBA);

    }

    osgDB::writeObjectFile(*stateset, "stateset.osgt");

    return true;
}

struct RealizeOperation : public osg::GraphicsOperation
{
    RealizeOperation(osg::ArgumentParser& arguments, osg::StateSet* stateset) :
        osg::GraphicsOperation("RealizeOperation",false)
    {
        _stateset = stateset;
        _useModelViewAndProjectionUniforms = arguments.read("--mv");
        _useVertexAttributeAliasing = arguments.read("--va");
    }

    virtual void operator () (osg::GraphicsContext* gc)
    {
        OSG_NOTICE<<std::endl<<"---------- RealizeOperation() : Pushing StateSet on to GraphicsContext's State. -----"<<std::endl<<std::endl;;

        gc->getState()->setUseModelViewAndProjectionUniforms(_useModelViewAndProjectionUniforms);
        gc->getState()->setUseVertexAttributeAliasing(_useVertexAttributeAliasing);


        gc->getState()->setUseStateAttributeShaders(true);
        gc->getState()->setUseStateAttributeFixedFunction(true);

        if (_stateset.valid()) gc->getState()->setRootStateSet(_stateset.get());
    }

    osg::ref_ptr<osg::StateSet> _stateset;
    bool _useModelViewAndProjectionUniforms;
    bool _useVertexAttributeAliasing;
};

int main(int argc, char** argv)
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    osgViewer::Viewer viewer(arguments);

    // add the state manipulator
    viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );

    // add the stats handler
    viewer.addEventHandler(new osgViewer::StatsHandler);

    osg::ref_ptr<osg::StateSet> stateset = new osg::StateSet;
    stateset->setGlobalDefaults();

    // set up the topmost StateSet with the shader pipeline settings.
    setUpStateSet(arguments, stateset.get());


    bool useRootStateSet = arguments.read("--state");

    viewer.setRealizeOperation(new RealizeOperation(arguments, useRootStateSet ? stateset.get() : 0));

    if (!useRootStateSet)
    {
        viewer.getCamera()->setStateSet(stateset.get());
    }

    viewer.realize();


    // load the data
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readRefNodeFiles(arguments);
    if (!loadedModel)
    {
        std::cout << arguments.getApplicationName() <<": No data loaded" << std::endl;
        return 1;
    }

    viewer.setSceneData(loadedModel);

    return viewer.run();

}
