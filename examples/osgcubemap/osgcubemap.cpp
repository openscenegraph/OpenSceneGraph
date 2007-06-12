/* OpenSceneGraph example, osgcubemap.
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
*  THE SOFTWARE.
*/

#include <osg/Group>
#include <osg/StateSet>
#include <osg/TextureCubeMap>
#include <osg/TexGen>
#include <osg/TexEnvCombine>

#include <osgUtil/ReflectionMapGenerator>
#include <osgUtil/HighlightMapGenerator>
#include <osgUtil/HalfWayMapGenerator>
#include <osgUtil/Optimizer>

#include <osgDB/ReadFile>
#include <osgDB/Registry>

#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>

#include <osgViewer/Viewer>

#include <iostream>
#include <string>
#include <vector>

void create_specular_highlights(osg::Node *node)
{
    osg::StateSet *ss = node->getOrCreateStateSet();
    
    // create and setup the texture object
    osg::TextureCubeMap *tcm = new osg::TextureCubeMap;
    tcm->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP);
    tcm->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP);
    tcm->setWrap(osg::Texture::WRAP_R, osg::Texture::CLAMP);
    tcm->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR);
    tcm->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);    

    // generate the six highlight map images (light direction = [1, 1, -1])
    osgUtil::HighlightMapGenerator *mapgen = new osgUtil::HighlightMapGenerator(
        osg::Vec3(1, 1, -1),            // light direction
        osg::Vec4(1, 0.9f, 0.8f, 1),    // light color
        8);                             // specular exponent
    
    mapgen->generateMap();

    // assign the six images to the texture object
    tcm->setImage(osg::TextureCubeMap::POSITIVE_X, mapgen->getImage(osg::TextureCubeMap::POSITIVE_X));
    tcm->setImage(osg::TextureCubeMap::NEGATIVE_X, mapgen->getImage(osg::TextureCubeMap::NEGATIVE_X));
    tcm->setImage(osg::TextureCubeMap::POSITIVE_Y, mapgen->getImage(osg::TextureCubeMap::POSITIVE_Y));
    tcm->setImage(osg::TextureCubeMap::NEGATIVE_Y, mapgen->getImage(osg::TextureCubeMap::NEGATIVE_Y));
    tcm->setImage(osg::TextureCubeMap::POSITIVE_Z, mapgen->getImage(osg::TextureCubeMap::POSITIVE_Z));
    tcm->setImage(osg::TextureCubeMap::NEGATIVE_Z, mapgen->getImage(osg::TextureCubeMap::NEGATIVE_Z));

    // enable texturing, replacing any textures in the subgraphs
    ss->setTextureAttributeAndModes(0, tcm, osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON);

    // texture coordinate generation
    osg::TexGen *tg = new osg::TexGen;
    tg->setMode(osg::TexGen::REFLECTION_MAP);
    ss->setTextureAttributeAndModes(0, tg, osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON);

    // use TexEnvCombine to add the highlights to the original lighting
    osg::TexEnvCombine *te = new osg::TexEnvCombine;    
    te->setCombine_RGB(osg::TexEnvCombine::ADD);
    te->setSource0_RGB(osg::TexEnvCombine::TEXTURE);
    te->setOperand0_RGB(osg::TexEnvCombine::SRC_COLOR);
    te->setSource1_RGB(osg::TexEnvCombine::PRIMARY_COLOR);
    te->setOperand1_RGB(osg::TexEnvCombine::SRC_COLOR);
    ss->setTextureAttributeAndModes(0, te, osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON);
}

int main(int argc, char *argv[])
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    // construct the viewer.
    osgViewer::Viewer viewer;
    
    // load the nodes from the commandline arguments.
    osg::Node* rootnode = osgDB::readNodeFiles(arguments);

    // if not loaded assume no arguments passed in, try use default mode instead.
    if (!rootnode) rootnode = osgDB::readNodeFile("cessna.osg");

    if (!rootnode)
    {
        osg::notify(osg::NOTICE)<<"Please specify a model filename on the command line."<<std::endl;
        return 1;
    }

    // create specular highlights
    create_specular_highlights(rootnode);
    
    // run optimization over the scene graph
    osgUtil::Optimizer optimzer;
    optimzer.optimize(rootnode);
     
    // add a viewport to the viewer and attach the scene graph.
    viewer.setSceneData(rootnode);
    
    // create the windows and run the threads.
    viewer.realize();

    // now check to see if texture cube map is supported.
    for(unsigned int contextID = 0; 
        contextID<osg::DisplaySettings::instance()->getMaxNumberOfGraphicsContexts();
        ++contextID)
    {
        osg::TextureCubeMap::Extensions* tcmExt = osg::TextureCubeMap::getExtensions(contextID,false);
        if (tcmExt)
        {
            if (!tcmExt->isCubeMapSupported())
            {
                std::cout<<"Warning: texture_cube_map not supported by OpenGL drivers, unable to run application."<<std::endl;
                return 1;
            }
        }
    }

    return viewer.run();
}
