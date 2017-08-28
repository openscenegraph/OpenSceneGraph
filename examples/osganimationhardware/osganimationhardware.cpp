/*  -*-c++-*-
 *  Copyright (C) 2009 Cedric Pinson <cedric.pinson@plopbyte.net>
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

#include <iostream>
#include <osgDB/ReadFile>
#include <osgViewer/ViewerEventHandlers>
#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>
#include <osgGA/KeySwitchMatrixManipulator>
#include <osgGA/StateSetManipulator>
#include <osgGA/AnimationPathManipulator>
#include <osgGA/TerrainManipulator>
#include <osg/Drawable>
#include <osg/MatrixTransform>

#include <osgAnimation/BasicAnimationManager>
#include <osgAnimation/RigGeometry>
#include <osgAnimation/RigTransformHardware>
#include <osgAnimation/AnimationManagerBase>
#include <osgAnimation/BoneMapVisitor>

#include <sstream>


static unsigned int getRandomValueinRange(unsigned int v)
{
    return static_cast<unsigned int>((rand() * 1.0 * v)/(RAND_MAX-1));
}


osg::ref_ptr<osg::Program> program;
// show how to override the default RigTransformHardware for customized usage
struct MyRigTransformHardware : public osgAnimation::RigTransformHardware
{

    virtual bool prepareData(osgAnimation::RigGeometry& rig)
    {
        if(!rig.getSkeleton() && !rig.getParents().empty())
        {
            osgAnimation::RigGeometry::FindNearestParentSkeleton finder;
            if(rig.getParents().size() > 1)
                osg::notify(osg::WARN) << "A RigGeometry should not have multi parent ( " << rig.getName() << " )" << std::endl;
            rig.getParents()[0]->accept(finder);

            if(!finder._root.valid())
            {
                osg::notify(osg::WARN) << "A RigGeometry did not find a parent skeleton for RigGeometry ( " << rig.getName() << " )" << std::endl;
                return false;
            }
            rig.setSkeleton(finder._root.get());
        }
        osgAnimation::BoneMapVisitor mapVisitor;
        rig.getSkeleton()->accept(mapVisitor);
        osgAnimation::BoneMap boneMap = mapVisitor.getBoneMap();

        if (!buildPalette(boneMap,rig) )
            return false;

        osg::Geometry& source = *rig.getSourceGeometry();
        osg::Vec3Array* positionSrc = dynamic_cast<osg::Vec3Array*>(source.getVertexArray());
        if (!positionSrc)
        {
            OSG_WARN << "RigTransformHardware no vertex array in the geometry " << rig.getName() << std::endl;
            return false;
        }

        // copy shallow from source geometry to rig
        rig.copyFrom(source);

       // osg::ref_ptr<osg::Program> program ;
        osg::ref_ptr<osg::Shader> vertexshader;
        osg::ref_ptr<osg::StateSet> stateset = rig.getOrCreateStateSet();

        //grab geom source program and vertex shader if _shader is not setted
        if(!_shader.valid() && (program = (osg::Program*)stateset->getAttribute(osg::StateAttribute::PROGRAM)))
        {
            for(unsigned int i=0; i<program->getNumShaders(); ++i)
                if(program->getShader(i)->getType()==osg::Shader::VERTEX)
                {
                    vertexshader=program->getShader(i);
                    program->removeShader(vertexshader);

                }
        }
        else
        {
            program = new osg::Program;
            program->setName("HardwareSkinning");
        }
        //set default source if _shader is not user setted
        if (!vertexshader.valid())
        {
            if (!_shader.valid())
                vertexshader = osg::Shader::readShaderFile(osg::Shader::VERTEX,"shaders/skinning.vert");
            else vertexshader=_shader;
        }


        if (!vertexshader.valid())
        {
            OSG_WARN << "RigTransformHardware can't load VertexShader" << std::endl;
            return false;
        }

        // replace max matrix by the value from uniform
        {
            std::string str = vertexshader->getShaderSource();
            std::string toreplace = std::string("MAX_MATRIX");
            std::size_t start = str.find(toreplace);
            if (std::string::npos != start)
            {
                std::stringstream ss;
                ss << getMatrixPaletteUniform()->getNumElements();
                str.replace(start, toreplace.size(), ss.str());
                vertexshader->setShaderSource(str);
            }
            else
            {
                OSG_INFO<< "MAX_MATRIX not found in Shader! " << str << std::endl;
            }
            OSG_INFO << "Shader " << str << std::endl;
        }

        unsigned int attribIndex = 11;
        unsigned int nbAttribs = getNumVertexAttrib();
        if(nbAttribs==0)
            OSG_WARN << "nbAttribs== " << nbAttribs << std::endl;
        for (unsigned int i = 0; i < nbAttribs; i++)
        {
            std::stringstream ss;
            ss << "boneWeight" << i;
            program->addBindAttribLocation(ss.str(), attribIndex + i);

            if(getVertexAttrib(i)->getNumElements()!=_nbVertexes)
                OSG_WARN << "getVertexAttrib== " << getVertexAttrib(i)->getNumElements() << std::endl;
            rig.setVertexAttribArray(attribIndex + i, getVertexAttrib(i));
            OSG_INFO << "set vertex attrib " << ss.str() << std::endl;
        }


        program->addShader(vertexshader.get());
        stateset->removeUniform("nbBonesPerVertex");
        stateset->addUniform(new osg::Uniform("nbBonesPerVertex",_bonesPerVertex));
        stateset->removeUniform("matrixPalette");
        stateset->addUniform(getMatrixPaletteUniform());

        stateset->removeAttribute(osg::StateAttribute::PROGRAM);
        if(!stateset->getAttribute(osg::StateAttribute::PROGRAM))
            stateset->setAttributeAndModes(program.get());

        _needInit = false;
        return true;
    }

};


struct SetupRigGeometry : public osg::NodeVisitor
{
    bool _hardware;
    SetupRigGeometry( bool hardware = true) : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN), _hardware(hardware) {}

    void apply(osg::Geode& geode)
    {
        for (unsigned int i = 0; i < geode.getNumDrawables(); i++)
            apply(*geode.getDrawable(i));
    }
    void apply(osg::Drawable& geom)
    {
        if (_hardware)
        {
            osgAnimation::RigGeometry* rig = dynamic_cast<osgAnimation::RigGeometry*>(&geom);
            if (rig)
                rig->setRigTransformImplementation(new MyRigTransformHardware);
        }

#if 0
        if (geom.getName() != std::string("BoundingBox")) // we disable compute of bounding box for all geometry except our bounding box
            geom.setComputeBoundingBoxCallback(new osg::Drawable::ComputeBoundingBoxCallback);
//            geom.setInitialBound(new osg::Drawable::ComputeBoundingBoxCallback);
#endif
    }
};

osg::Group* createCharacterInstance(osg::Group* character, bool hardware)
{
    osg::ref_ptr<osg::Group> c ;
    if (hardware)
        c = osg::clone(character, osg::CopyOp::DEEP_COPY_ALL & ~osg::CopyOp::DEEP_COPY_PRIMITIVES & ~osg::CopyOp::DEEP_COPY_ARRAYS);
    else
        c = osg::clone(character, osg::CopyOp::DEEP_COPY_ALL);

    osgAnimation::AnimationManagerBase* animationManager = dynamic_cast<osgAnimation::AnimationManagerBase*>(c->getUpdateCallback());

    osgAnimation::BasicAnimationManager* anim = dynamic_cast<osgAnimation::BasicAnimationManager*>(animationManager);
    const osgAnimation::AnimationList& list = animationManager->getAnimationList();
    int v = getRandomValueinRange(list.size());
    if (list[v]->getName() == std::string("MatIpo_ipo"))
    {
        anim->playAnimation(list[v].get());
        v = (v + 1)%list.size();
    }

    anim->playAnimation(list[v].get());

    SetupRigGeometry switcher(hardware);
    c->accept(switcher);

    return c.release();
}


int main (int argc, char* argv[])
{
    std::cerr << "This example works better with nathan.osg" << std::endl;

    osg::ArgumentParser psr(&argc, argv);

    osgViewer::Viewer viewer(psr);

    bool hardware = true;
    int maxChar = 10;
    while (psr.read("--software"))
    {
        hardware = false;
    }
    while (psr.read("--number", maxChar)) {}

    osg::ref_ptr<osg::Node> node = osgDB::readRefNodeFiles(psr);
    osg::ref_ptr<osg::Group> root = dynamic_cast<osg::Group*>(node.get());
    if (!root)
    {
        std::cout << psr.getApplicationName() <<": No data loaded" << std::endl;
        return 1;
    }

    {
        osgAnimation::AnimationManagerBase* animationManager = dynamic_cast<osgAnimation::AnimationManagerBase*>(root->getUpdateCallback());
        if(!animationManager)
        {
            osg::notify(osg::FATAL) << "no AnimationManagerBase found, updateCallback need to animate elements" << std::endl;
            return 1;
        }
    }


    osg::ref_ptr<osg::Group> scene = new osg::Group;

    // add the state manipulator
    viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );

    // add the thread model handler
    viewer.addEventHandler(new osgViewer::ThreadingHandler);

    // add the window size toggle handler
    viewer.addEventHandler(new osgViewer::WindowSizeHandler);

    // add the stats handler
    viewer.addEventHandler(new osgViewer::StatsHandler);

    // add the help handler
    viewer.addEventHandler(new osgViewer::HelpHandler(psr.getApplicationUsage()));

    // add the LOD Scale handler
    viewer.addEventHandler(new osgViewer::LODScaleHandler);

    // add the screen capture handler
    viewer.addEventHandler(new osgViewer::ScreenCaptureHandler);

    viewer.setSceneData(scene.get());

    viewer.realize();

    double xChar = maxChar;
    double yChar = xChar * 9.0/16;
    for (double  i = 0.0; i < xChar; i++)
    {
        for (double  j = 0.0; j < yChar; j++)
        {

            osg::ref_ptr<osg::Group> c = createCharacterInstance(root.get(), hardware);
            osg::MatrixTransform* tr = new osg::MatrixTransform;
            tr->setMatrix(osg::Matrix::translate( 2.0 * (i - xChar * .5),
                                                  0.0,
                                                  2.0 * (j - yChar * .5)));
            tr->addChild(c.get());
            scene->addChild(tr);
        }
    }
    std::cout << "created " << xChar * yChar << " instance"  << std::endl;


    return viewer.run();
}


