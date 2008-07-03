/* OpenSceneGraph example, osgintersection.
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

  
#include <osgDB/ReadFile>

#include <osg/ArgumentParser>
#include <osg/ApplicationUsage>
#include <osg/Timer>
#include <osg/CoordinateSystemNode>
#include <osg/Notify>
#include <osg/io_utils>
#include <osg/Geometry>
#include <osg/TriangleIndexFunctor>


#include <osgUtil/IntersectionVisitor>
#include <osgUtil/LineSegmentIntersector>
#include <osgUtil/UpdateVisitor>

#include <osgSim/LineOfSight>
#include <osgSim/HeightAboveTerrain>
#include <osgSim/ElevationSlice>

#include "fixeddivision.h"
#include "variabledivision.h"


int main(int argc, char **argv)
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);
    
    int maxNumLevels = 16;
    int targetNumIndicesPerLeaf = 16;
    bool processTriangles = true;

    while (arguments.read("--max", maxNumLevels)) {}
    while (arguments.read("--leaf", targetNumIndicesPerLeaf)) {}
    while (arguments.read("--points")) processTriangles = false;
    while (arguments.read("--triangles")) processTriangles = true;
    
    osg::ref_ptr<osg::Node> scene = osgDB::readNodeFiles(arguments);
    
    if (!scene) 
    {
        std::cout<<"No model loaded, please specify a valid model on the command line."<<std::endl;
        return 0;
    }


    osgUtil::UpdateVisitor updateVisitor;
    updateVisitor.setFrameStamp(new osg::FrameStamp);
    scene->accept(updateVisitor);
    scene->getBound();

    if (arguments.read("--fd"))
    {
        fixeddivision::KDTreeBuilder builder;

        builder._maxNumLevels = maxNumLevels;
        builder._targetNumIndicesPerLeaf = targetNumIndicesPerLeaf;
        builder._processTriangles = processTriangles;


        osg::Timer_t start = osg::Timer::instance()->tick();


        scene->accept(builder);

        osg::Timer_t end = osg::Timer::instance()->tick();
        double time = osg::Timer::instance()->delta_s(start,end);
        osg::notify(osg::NOTICE)<<"Time to build "<<time*1000.0<<"ms "<<builder._numVerticesProcessed<<std::endl;
        osg::notify(osg::NOTICE)<<"build speed "<<(double(builder._numVerticesProcessed)/time)/1000000.0<<"M vertices per second"<<std::endl;
    }
    else
    {
        variabledivision::KDTreeBuilder builder;

        builder._maxNumLevels = maxNumLevels;
        builder._targetNumTrianglesPerLeaf = targetNumIndicesPerLeaf;
        builder._processTriangles = processTriangles;


        osg::Timer_t start = osg::Timer::instance()->tick();


        scene->accept(builder);

        osg::Timer_t end = osg::Timer::instance()->tick();
        double time = osg::Timer::instance()->delta_s(start,end);
        osg::notify(osg::NOTICE)<<"Time to build "<<time*1000.0<<"ms "<<builder._numVerticesProcessed<<std::endl;
        osg::notify(osg::NOTICE)<<"build speed "<<(double(builder._numVerticesProcessed)/time)/1000000.0<<"M vertices per second"<<std::endl;
    }    
    
    return 0;
}
