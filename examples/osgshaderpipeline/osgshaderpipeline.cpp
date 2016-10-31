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

int main(int argc, char** argv)
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    osgViewer::Viewer viewer(arguments);

    // add the state manipulator
    viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );

    // add the stats handler
    viewer.addEventHandler(new osgViewer::StatsHandler);


    osg::ref_ptr<osg::Program> program = new osg::Program;


    if (!readShaderArguments(arguments, "--vert", program, "shaders/shaderpipeline.vert"))
    {
        return 1;
    }

    if (!readShaderArguments(arguments, "--frag", program, "shaders/shaderpipeline.frag"))
    {
        return 1;
    }

    // assign program to topmost StateSet
    viewer.getCamera()->getOrCreateStateSet()->setAttribute(program);

    // load the data
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readRefNodeFiles(arguments);
    if (!loadedModel)
    {
        std::cout << arguments.getApplicationName() <<": No data loaded" << std::endl;
        return 1;
    }

    viewer.setSceneData(loadedModel);

    osg::ref_ptr<osg::StateSet> stateset = viewer.getCamera()->getOrCreateStateSet();

    unsigned int maxTextureUnits = 1;
    std::stringstream sstream;
    sstream<<maxTextureUnits;
    stateset->setDefine("GL_MAX_TEXTURE_UNITS", sstream.str());

    #define ADD_DEFINE(DEF) \
        sstream.str(""); \
        sstream<<DEF; \
        stateset->setDefine(#DEF, sstream.str());

    if (maxTextureUnits>0)
    {
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


        osg::ref_ptr<osg::Uniform> ACTIVE_TEXTURE = new osg::Uniform(osg::Uniform::BOOL, "GL_ACTIVE_TEXTURE", maxTextureUnits);
        osg::ref_ptr<osg::Uniform> TEXTURE_GEN_S = new osg::Uniform(osg::Uniform::BOOL, "GL_TEXTURE_GEN_S", maxTextureUnits);
        osg::ref_ptr<osg::Uniform> TEXTURE_GEN_T = new osg::Uniform(osg::Uniform::BOOL, "GL_TEXTURE_GEN_T", maxTextureUnits);

        osg::ref_ptr<osg::Uniform> TEXTURE_GEN_MODE = new osg::Uniform(osg::Uniform::INT, "GL_TEXTURE_GEN_MODE", maxTextureUnits);
        osg::ref_ptr<osg::Uniform> TEXTURE_ENV_MODE = new osg::Uniform(osg::Uniform::INT, "GL_TEXTURE_ENV_MODE", maxTextureUnits);


        for(unsigned int i=0; i<maxTextureUnits;++i)
        {
            ACTIVE_TEXTURE->setElement(i, false);
            TEXTURE_GEN_MODE->setElement(i, 0);
            TEXTURE_GEN_S->setElement(i, false);
            TEXTURE_GEN_T->setElement(i, false);
        }

        ACTIVE_TEXTURE->setElement(0, true);
        TEXTURE_GEN_MODE->setElement(0, 0);
        //TEXTURE_GEN_MODE->setElement(0, GL_SPHERE_MAP);
        TEXTURE_GEN_S->setElement(0, true);
        TEXTURE_GEN_T->setElement(0, true);

        stateset->addUniform(ACTIVE_TEXTURE.get());
        stateset->addUniform(TEXTURE_GEN_S.get());
        stateset->addUniform(TEXTURE_GEN_T.get());
        stateset->addUniform(TEXTURE_GEN_MODE.get());
        stateset->addUniform(TEXTURE_ENV_MODE.get());


        for(unsigned int i=0; i<maxTextureUnits;++i)
        {
            sstream.str("");
            sstream<<"sampler"<<i;
            OSG_NOTICE<<"****** texture unit : "<<sstream.str()<<std::endl;
            stateset->addUniform(new osg::Uniform(sstream.str().c_str(), i));


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
            sstream<<"{ TexCoord"<<i<<" = gl_MultiTexCoord"<<i<<"; }";

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


    viewer.realize();

    return viewer.run();

}
