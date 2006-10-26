#include <osg/ArgumentParser>
#include <osg/ApplicationUsage>
#include <osg/Timer>
#include <osg/Notify>
#include <osg/io_utils>

#include <osgDB/ReadFile>

#include <osgUtil/IntersectionVisitor>

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
    
    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is the example which demonstrates use of node tracker.");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName());
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
    
    osg::ref_ptr<osg::Node> root = osgDB::readNodeFiles(arguments);
    
    if (!root) 
    {
        std::cout<<"No model loaded, please specify a valid model on the command line."<<std::endl;
        return 0;
    }
    
    std::cout<<"Intersection "<<std::endl;

    osg::Timer_t startTick = osg::Timer::instance()->tick();
    
    
    osg::BoundingSphere bs = root->getBound();
#if 1
    osg::Vec3d start = bs.center() + osg::Vec3d(0.0,bs.radius(),0.0);
    osg::Vec3d end = bs.center() - osg::Vec3d(0.0, bs.radius(),0.0);
#else
    osg::Vec3d start = bs.center() + osg::Vec3d(0.0,0.0, bs.radius());
    osg::Vec3d end = bs.center() - osg::Vec3d(0.0, 0.0, bs.radius());
#endif

    
    osg::ref_ptr<osgUtil::LineSegmentIntersector> intersector = new osgUtil::LineSegmentIntersector(start, end);
    
    osgUtil::IntersectionVisitor intersectVisitor( intersector.get(), new MyReadCallback );
    
    root->accept(intersectVisitor);
    
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

    return 0;
}
