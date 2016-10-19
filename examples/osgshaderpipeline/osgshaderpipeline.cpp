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

#include <osgDB/ReadFile>

#include <osgGA/StateSetManipulator>

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include <iostream>


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


    bool vertexShadersAssigned = false;
    std::string vertexShaderFilename;
    while(arguments.read("--vert", vertexShaderFilename))
    {
        osg::ref_ptr<osg::Shader> vertexShader = osgDB::readRefShaderFile(vertexShaderFilename);
        if (vertexShader)
        {
            vertexShadersAssigned = true;
            program->addShader(vertexShader);
        }
        else
        {
            OSG_NOTICE<<"Unable to load vertex shader file : "<<vertexShaderFilename<<std::endl;
        }
    }

    if (!vertexShadersAssigned)
    {
        vertexShaderFilename = "shaders/shaderpipeline.vert";
        osg::ref_ptr<osg::Shader> vertexShader = osgDB::readRefShaderFile(vertexShaderFilename);
        if (vertexShader)
        {
            vertexShadersAssigned = true;
            program->addShader(vertexShader);
        }
        else
        {
            OSG_NOTICE<<"Unable to load vertex shader file : "<<vertexShaderFilename<<std::endl;
            return 1;
        }
    }



    bool fragmentShadersAssigned = false;
    std::string fragmentShaderFilename;
    while(arguments.read("--frag", fragmentShaderFilename))
    {
        osg::ref_ptr<osg::Shader> fragmentShader = osgDB::readRefShaderFile(fragmentShaderFilename);
        if (fragmentShader)
        {
            fragmentShadersAssigned = true;
            program->addShader(fragmentShader);
        }
        else
        {
            OSG_NOTICE<<"Unable to load fragment shader file : "<<fragmentShaderFilename<<std::endl;
        }
    }

    if (!fragmentShadersAssigned)
    {
        fragmentShaderFilename = "shaders/shaderpipeline.vert";
        osg::ref_ptr<osg::Shader> fragmentShader = osgDB::readRefShaderFile(fragmentShaderFilename);
        if (fragmentShader)
        {
            fragmentShadersAssigned = true;
            program->addShader(fragmentShader);
        }
        else
        {
            OSG_NOTICE<<"Unable to load fragment shader file : "<<fragmentShaderFilename<<std::endl;
            return 1;
        }
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

    viewer.realize();

    return viewer.run();

}
