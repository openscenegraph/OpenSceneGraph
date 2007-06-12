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

#include <osg/ArgumentParser>
#include <osg/ApplicationUsage>
#include <osg/Timer>
#include <osg/CoordinateSystemNode>
#include <osg/Notify>
#include <osg/io_utils>

#include <osgDB/ReadFile>

#include <osgUtil/IntersectionVisitor>
#include <osgUtil/LineSegmentIntersector>

#include <osgSim/LineOfSight>
#include <osgSim/HeightAboveTerrain>
#include <osgSim/ElevationSlice>

#include <iostream>

struct MyReadCallback : public osgUtil::IntersectionVisitor::ReadCallback
{
    virtual osg::Node* readNodeFile(const std::string& filename)
    {
        return osgDB::readNodeFile(filename);
    }
};


int main(int argc, char **argv)
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);
    
    osg::ref_ptr<osg::Node> scene = osgDB::readNodeFiles(arguments);
    
    if (!scene) 
    {
        std::cout<<"No model loaded, please specify a valid model on the command line."<<std::endl;
        return 0;
    }
    
    std::cout<<"Intersection "<<std::endl;
    
    
    osg::BoundingSphere bs = scene->getBound();


    bool useIntersectorGroup = true;
    bool useLineOfSight = true;
    
    //osg::CoordinateSystemNode* csn = dynamic_cast<osg::CoordinateSystemNode*>(scene.get());
    //osg::EllipsoidModel* em = csn ? csn->getEllipsoidModel() : 0;

    if (useLineOfSight)
    {
    
        osg::Vec3d start = bs.center() + osg::Vec3d(0.0,bs.radius(),0.0);
        osg::Vec3d end = bs.center() - osg::Vec3d(0.0, bs.radius(),0.0);
        osg::Vec3d deltaRow( 0.0, 0.0, bs.radius()*0.01);
        osg::Vec3d deltaColumn( bs.radius()*0.01, 0.0, 0.0);

        osgSim::LineOfSight los;
        
#if 1
        unsigned int numRows = 20;
        unsigned int numColumns = 20;
        osgSim::HeightAboveTerrain hat;
        hat.setDatabaseCacheReadCallback(los.getDatabaseCacheReadCallback());

        for(unsigned int r=0; r<numRows; ++r)
        {
            for(unsigned int c=0; c<numColumns; ++c)
            {
                osg::Vec3d s = start + deltaColumn * double(c) + deltaRow * double(r);
                osg::Vec3d e = end + deltaColumn * double(c) + deltaRow * double(r);
                los.addLOS(s,e);
                hat.addPoint(s);
            }
        }


        {
            std::cout<<"Computing LineOfSight"<<std::endl;

            osg::Timer_t startTick = osg::Timer::instance()->tick();

            los.computeIntersections(scene.get());

            osg::Timer_t endTick = osg::Timer::instance()->tick();

            std::cout<<"Completed in "<<osg::Timer::instance()->delta_s(startTick,endTick)<<std::endl;

            for(unsigned int i=0; i<los.getNumLOS(); i++)
            {
                const osgSim::LineOfSight::Intersections& intersections = los.getIntersections(i);
                for(osgSim::LineOfSight::Intersections::const_iterator itr = intersections.begin();
                    itr != intersections.end();
                    ++itr)
                {
                     std::cout<<"  point "<<*itr<<std::endl;
                }
            }
        }
        
        {
            // now do a second traversal to test performance of cache.
            osg::Timer_t startTick = osg::Timer::instance()->tick();

            std::cout<<"Computing HeightAboveTerrain"<<std::endl;

            hat.computeIntersections(scene.get());

            osg::Timer_t endTick = osg::Timer::instance()->tick();

            for(unsigned int i=0; i<hat.getNumPoints(); i++)
            {
                 std::cout<<"  point = "<<hat.getPoint(i)<<" hat = "<<hat.getHeightAboveTerrain(i)<<std::endl;
            }


            std::cout<<"Completed in "<<osg::Timer::instance()->delta_s(startTick,endTick)<<std::endl;
        }
#endif

        {
            // now do a second traversal to test performance of cache.
            osg::Timer_t startTick = osg::Timer::instance()->tick();

            std::cout<<"Computing ElevationSlice"<<std::endl;
            osgSim::ElevationSlice es;
            es.setDatabaseCacheReadCallback(los.getDatabaseCacheReadCallback());

            es.setStartPoint(bs.center()+osg::Vec3d(bs.radius(),0.0,0.0) );
            es.setEndPoint(bs.center()+osg::Vec3d(0.0,0.0, bs.radius()) );

            es.computeIntersections(scene.get());

            osg::Timer_t endTick = osg::Timer::instance()->tick();

            std::cout<<"Completed in "<<osg::Timer::instance()->delta_s(startTick,endTick)<<std::endl;

            typedef osgSim::ElevationSlice::DistanceHeightList DistanceHeightList;
            const DistanceHeightList& dhl = es.getDistanceHeightIntersections();
            std::cout<<"Number of intersections ="<<dhl.size()<<std::endl;
            for(DistanceHeightList::const_iterator dhitr = dhl.begin();
                dhitr != dhl.end();
                ++dhitr)
            {
                 std::cout.precision(10);
                 std::cout<<"  "<<dhitr->first<<" "<<dhitr->second<<std::endl;
            }


        }
    }
    else if (useIntersectorGroup)
    {
        osg::Timer_t startTick = osg::Timer::instance()->tick();
    
        osg::Vec3d start = bs.center() + osg::Vec3d(0.0,bs.radius(),0.0);
        osg::Vec3d end = bs.center();// - osg::Vec3d(0.0, bs.radius(),0.0);
        osg::Vec3d deltaRow( 0.0, 0.0, bs.radius()*0.01);
        osg::Vec3d deltaColumn( bs.radius()*0.01, 0.0, 0.0);
        unsigned int numRows = 20;
        unsigned int numColumns = 20;

        osg::ref_ptr<osgUtil::IntersectorGroup> intersectorGroup = new osgUtil::IntersectorGroup();

        for(unsigned int r=0; r<numRows; ++r)
        {
            for(unsigned int c=0; c<numColumns; ++c)
            {
                osg::Vec3d s = start + deltaColumn * double(c) + deltaRow * double(r);
                osg::Vec3d e = end + deltaColumn * double(c) + deltaRow * double(r);
                osg::ref_ptr<osgUtil::LineSegmentIntersector> intersector = new osgUtil::LineSegmentIntersector(s, e);
                intersectorGroup->addIntersector( intersector.get() );
            }
        }

        
        osgUtil::IntersectionVisitor intersectVisitor( intersectorGroup.get(), new MyReadCallback );
        scene->accept(intersectVisitor);

        osg::Timer_t endTick = osg::Timer::instance()->tick();

        std::cout<<"Completed in "<<osg::Timer::instance()->delta_s(startTick,endTick)<<std::endl;

        if ( intersectorGroup->containsIntersections() )
        {
            std::cout<<"Found intersections "<<std::endl;

            osgUtil::IntersectorGroup::Intersectors& intersectors = intersectorGroup->getIntersectors();
            for(osgUtil::IntersectorGroup::Intersectors::iterator intersector_itr = intersectors.begin();
                intersector_itr != intersectors.end();
                ++intersector_itr)
            {
                osgUtil::LineSegmentIntersector* lsi = dynamic_cast<osgUtil::LineSegmentIntersector*>(intersector_itr->get());
                if (lsi)
                {
                    osgUtil::LineSegmentIntersector::Intersections& intersections = lsi->getIntersections();
                    for(osgUtil::LineSegmentIntersector::Intersections::iterator itr = intersections.begin();
                        itr != intersections.end();
                        ++itr)
                    {
                        const osgUtil::LineSegmentIntersector::Intersection& intersection = *itr;
                        std::cout<<"  ratio "<<intersection.ratio<<std::endl;
                        std::cout<<"  point "<<intersection.localIntersectionPoint<<std::endl;
                        std::cout<<"  normal "<<intersection.localIntersectionNormal<<std::endl;
                        std::cout<<"  indices "<<intersection.indexList.size()<<std::endl;
                        std::cout<<"  primitiveIndex "<<intersection.primitiveIndex<<std::endl;
                        std::cout<<std::endl;
                    }
                }
            }
        
        }

    }
    else
    {
        osg::Timer_t startTick = osg::Timer::instance()->tick();

    #if 1
        osg::Vec3d start = bs.center() + osg::Vec3d(0.0,bs.radius(),0.0);
        osg::Vec3d end = bs.center() - osg::Vec3d(0.0, bs.radius(),0.0);
    #else
        osg::Vec3d start = bs.center() + osg::Vec3d(0.0,0.0, bs.radius());
        osg::Vec3d end = bs.center() - osg::Vec3d(0.0, 0.0, bs.radius());
    #endif

        osg::ref_ptr<osgUtil::LineSegmentIntersector> intersector = new osgUtil::LineSegmentIntersector(start, end);

        osgUtil::IntersectionVisitor intersectVisitor( intersector.get(), new MyReadCallback );

        scene->accept(intersectVisitor);

        osg::Timer_t endTick = osg::Timer::instance()->tick();

        std::cout<<"Completed in "<<osg::Timer::instance()->delta_s(startTick,endTick)<<std::endl;

        if ( intersector->containsIntersections() )
        {
            osgUtil::LineSegmentIntersector::Intersections& intersections = intersector->getIntersections();
            for(osgUtil::LineSegmentIntersector::Intersections::iterator itr = intersections.begin();
                itr != intersections.end();
                ++itr)
            {
                const osgUtil::LineSegmentIntersector::Intersection& intersection = *itr;
                std::cout<<"  ratio "<<intersection.ratio<<std::endl;
                std::cout<<"  point "<<intersection.localIntersectionPoint<<std::endl;
                std::cout<<"  normal "<<intersection.localIntersectionNormal<<std::endl;
                std::cout<<"  indices "<<intersection.indexList.size()<<std::endl;
                std::cout<<"  primitiveIndex "<<intersection.primitiveIndex<<std::endl;
                std::cout<<std::endl;
            }
        }
    }
    
    return 0;
}
