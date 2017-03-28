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


    if (!readShaderArguments(arguments, "--vert", program.get(), "shaders/shaderpipeline.vert"))
    {
        return 1;
    }

    if (!readShaderArguments(arguments, "--frag", program.get(), "shaders/shaderpipeline.frag"))
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

    viewer.realize();

    return viewer.run();

}
