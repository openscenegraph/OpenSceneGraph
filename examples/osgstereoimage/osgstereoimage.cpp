/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2003 Robert Osfield 
 *
 * This library is open source and may be redistributed and/or modified under  
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or 
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * OpenSceneGraph Public License for more details.
*/

#include <osgProducer/Viewer>
#include <osgDB/ReadFile>
#include <osgUtil/Optimizer>

#include <osg/Geode>
#include <osg/Notify>
#include <osg/MatrixTransform>


int main( int argc, char **argv )
{

    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);
    
    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getProgramName()+" [options] image_file_left_eye image_file_right_eye");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
    

    // construct the viewer.
    osgProducer::Viewer viewer(arguments);

    // set up the value with sensible default event handlers.
    viewer.setUpViewer(osgProducer::Viewer::STANDARD_SETTINGS);

    // get details on keyboard and mouse bindings used by the viewer.
    viewer.getUsage(*arguments.getApplicationUsage());

    // if user request help write it out to cout.
    if (arguments.read("-h") || arguments.read("--help"))
    {
        arguments.getApplicationUsage()->write(std::cout);
        return 1;
    }

    // any option left unread are converted into errors to write out later.
    arguments.reportRemainingOptionsAsUnrecognized();

    // report any errors if they have occured when parsing the program aguments.
    if (arguments.errors())
    {
        arguments.writeErrorMessages(std::cout);
        return 1;
    }

    // extract the filenames from the arguments list.
    std::string fileLeft,fileRight;
    for(int pos=1;pos<arguments.argc();++pos)
    {
        if (arguments.isString(pos))
        {
            if (fileLeft.empty()) fileLeft = arguments[pos];
            else if (fileRight.empty()) fileRight = arguments[pos];
        }
    }


    if (fileLeft.empty() || fileRight.empty())
    {
        std::cout << arguments.getProgramName() <<": Please specify two image files required for stereo imaging."<<std::endl;
        return 1;
    }
    
    osg::Image* imageLeft = osgDB::readImageFile(fileLeft);
    osg::Image* imageRight = osgDB::readImageFile(fileRight);
    
    if (!imageLeft || !imageRight)
    {    
        std::cout << arguments.getProgramName() <<": Unable to load two image files required for stereo imaging."<<std::endl;
        return 1;
    }
    
    float average_s = (imageLeft->s()+imageRight->s())*0.5f;
    float average_t = (imageLeft->t()+imageRight->t())*0.5f;

    osg::Geode* geodeLeft = osg::createGeodeForImage(imageLeft,average_s,average_t);
    geodeLeft->setNodeMask(0x01);

    osg::Geode* geodeRight = osg::createGeodeForImage(imageRight,average_s,average_t);
    geodeRight->setNodeMask(0x02);

    osg::ref_ptr<osg::Group> rootNode = new osg::Group;
    rootNode->addChild(geodeLeft);
    rootNode->addChild(geodeRight);

    // set the scene to render
    viewer.setSceneData(rootNode.get());

    // create the windows and run the threads.
    viewer.realize(Producer::CameraGroup::ThreadPerCamera);

    // set all the sceneview's up so that their left and right add cull masks are set up.
    for(osgProducer::OsgCameraGroup::SceneHandlerList::iterator itr=viewer.getSceneHandlerList().begin();
        itr!=viewer.getSceneHandlerList().end();
        ++itr)
    {
        osgUtil::SceneView* sceneview = *itr;
        sceneview->setCullMask(0xffffffff);
        sceneview->setCullMaskLeft(0x00000001);
        sceneview->setCullMaskRight(0x00000002);
    }


    while( !viewer.done() )
    {
        // wait for all cull and draw threads to complete.
        viewer.sync();

        // update the scene by traversing it with the the update visitor which will
        // call all node update callbacks and animations.
        viewer.update();
         
        // fire off the cull and draw traversals of the scene.
        viewer.frame();
        
    }
    
    return 0;
}

