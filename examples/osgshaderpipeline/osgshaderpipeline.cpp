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


    if (!readShaderArguments(arguments, "--vert", program, "shaders/shaderpipeline.vert"))
    {
        return false;
    }

    if (!readShaderArguments(arguments, "--frag", program, "shaders/shaderpipeline.frag"))
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


        ADD_DEFINE(GL_EYE_LINEAR);
        ADD_DEFINE(GL_OBJECT_LINEAR);
        ADD_DEFINE(GL_SPHERE_MAP);
        ADD_DEFINE(GL_NORMAL_MAP);
        ADD_DEFINE(GL_REFLECTION_MAP);

        ADD_DEFINE(GL_MODULATE);
        ADD_DEFINE(GL_REPLACE);
        ADD_DEFINE(GL_DECAL);
        ADD_DEFINE(GL_BLEND);
        ADD_DEFINE(GL_ADD);

        ADD_DEFINE(GL_ALPHA);
        ADD_DEFINE(GL_INTENSITY);
        ADD_DEFINE(GL_LUMINANCE);
        ADD_DEFINE(GL_RED);
        ADD_DEFINE(GL_RG);
        ADD_DEFINE(GL_RGB);
        ADD_DEFINE(GL_RGBA);


        osg::ref_ptr<osg::Uniform> ACTIVE_TEXTURE = new osg::Uniform(osg::Uniform::BOOL, "GL_ACTIVE_TEXTURE", maxTextureUnits);
        osg::ref_ptr<osg::Uniform> TEXTURE_GEN_S = new osg::Uniform(osg::Uniform::BOOL, "GL_TEXTURE_GEN_S", maxTextureUnits);
        osg::ref_ptr<osg::Uniform> TEXTURE_GEN_T = new osg::Uniform(osg::Uniform::BOOL, "GL_TEXTURE_GEN_T", maxTextureUnits);

        osg::ref_ptr<osg::Uniform> TEXTURE_GEN_MODE = new osg::Uniform(osg::Uniform::INT, "GL_TEXTURE_GEN_MODE", maxTextureUnits);
        osg::ref_ptr<osg::Uniform> TEXTURE_ENV_MODE = new osg::Uniform(osg::Uniform::INT, "GL_TEXTURE_ENV_MODE", maxTextureUnits);
        osg::ref_ptr<osg::Uniform> TEXTURE_FORMAT = new osg::Uniform(osg::Uniform::INT, "GL_TEXTURE_FORMAT", maxTextureUnits);


        for(unsigned int i=0; i<maxTextureUnits;++i)
        {
            ACTIVE_TEXTURE->setElement(i, false);
            TEXTURE_GEN_MODE->setElement(i, 0);
            TEXTURE_GEN_S->setElement(i, false);
            TEXTURE_GEN_T->setElement(i, false);
            TEXTURE_ENV_MODE->setElement(i, GL_MODULATE);
            TEXTURE_FORMAT->setElement(i, GL_RGBA);
        }

        ACTIVE_TEXTURE->setElement(0, true);
        TEXTURE_GEN_MODE->setElement(0, 0);
        //TEXTURE_GEN_MODE->setElement(0, GL_SPHERE_MAP);
        TEXTURE_GEN_S->setElement(0, true);
        TEXTURE_GEN_T->setElement(0, true);
        //TEXTURE_FORMAT->setElement(0, GL_ALPHA);

        stateset->addUniform(ACTIVE_TEXTURE.get());
        stateset->addUniform(TEXTURE_GEN_S.get());
        stateset->addUniform(TEXTURE_GEN_T.get());
        stateset->addUniform(TEXTURE_GEN_MODE.get());
        stateset->addUniform(TEXTURE_ENV_MODE.get());
        stateset->addUniform(TEXTURE_FORMAT.get());


        for(unsigned int i=0; i<maxTextureUnits;++i)
        {
            sstream.str("");
            sstream<<"sampler"<<i;
            OSG_NOTICE<<"****** texture unit : "<<sstream.str()<<std::endl;
            stateset->addUniform(new osg::Uniform(sstream.str().c_str(), static_cast<int>(i)));


            // fragment shader texture defines
            sstream.str("");
            sstream<<"TEXTURE_VERT_DECLARE"<<i;
            std::string textureVertDeclareDefine = sstream.str();

            sstream.str("");
            sstream<<"varying vec4 TexCoord"<<i<<";";

            stateset->setDefine(textureVertDeclareDefine, sstream.str());


            sstream.str("");
            sstream<<"TEXTURE_VERT_BODY"<<i;
            std::string textureVertBodyDefine = sstream.str();

            sstream.str("");
            sstream<<"{ TexCoord"<<i<<" = gl_MultiTexCoord"<<i<<"; if (GL_TEXTURE_GEN_MODE["<<i<<"]!=0) TexCoord0 = texgen(TexCoord"<<i<<", "<<i<<"); }";

            stateset->setDefine(textureVertBodyDefine, sstream.str());


            // fragment shader texture defines
            sstream.str("");
            sstream<<"TEXTURE_FRAG_DECLARE"<<i;
            std::string textureFragDeclareDefine = sstream.str();

            sstream.str("");
            sstream<<"uniform ";
            sstream<<"sampler2D ";
            sstream<<"sampler"<<i<<"; ";
            sstream<<"varying vec4 TexCoord"<<i<<";";

            stateset->setDefine(textureFragDeclareDefine, sstream.str());


            sstream.str("");
            sstream<<"TEXTURE_FRAG_BODY"<<i;
            std::string textureFragBodyDefine = sstream.str();

            sstream.str("");
            sstream<<"(color) { color = texenv(color, ";
            sstream<<"texture2D( sampler"<<i<<", TexCoord"<<i<<".st)";
            sstream<<", "<<i<<"); }";

            stateset->setDefine(textureFragBodyDefine, sstream.str());

        }

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
    setUpStateSet(arguments, stateset);


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
