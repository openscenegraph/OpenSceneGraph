/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield 
 *
 * This application is open source and may be redistributed and/or modified   
 * freely and without restriction, both in commericial and non commericial applications,
 * as long as this copyright notice is maintained.
 * 
 * This application is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

#include <osg/Timer>
#include <osg/ArgumentParser>
#include <osg/ApplicationUsage>
#include <osg/CoordinateSystemNode>
#include <osg/io_utils>

#include <osgTerrain/TerrainTile>

#include <osgDB/Archive>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgDB/FileUtils>

#include <iostream>
#include <algorithm>

#include <signal.h>

static bool s_ExitApplication = false;

class LoadDataVisitor : public osg::NodeVisitor
{
public:


    LoadDataVisitor(unsigned int maxNumLevels=0):
        osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
        _maxLevels(maxNumLevels),
        _currentLevel(0) {}
        
    void apply(osg::CoordinateSystemNode& cs)
    {
        std::cout<<"CoordinateSystemNode "<<std::endl;
        
        if (!s_ExitApplication) traverse(cs);
    }
    
    void apply(osg::Group& group)
    {
        osgTerrain::TerrainTile* terrainTile = dynamic_cast<osgTerrain::TerrainTile*>(&group);
        osgTerrain::Locator* locator = terrainTile ? terrainTile->getLocator() : 0;
        if (locator)
        {
            std::cout<<"    Found terrain locator "<<locator<<std::endl;
            osg::Vec3d l00(0.0,0.0,0.0);
            osg::Vec3d l10(1.0,0.0,0.0);
            osg::Vec3d l11(1.0,1.0,0.0);
            osg::Vec3d l01(0.0,1.0,0.0);
            
            osg::Vec3d w00, w10, w11, w01;
            
            locator->convertLocalToModel(l00, w00);
            locator->convertLocalToModel(l10, w10);
            locator->convertLocalToModel(l11, w11);
            locator->convertLocalToModel(l01, w01);
            
            if (locator->getEllipsoidModel() &&
                locator->getCoordinateSystemType()==osgTerrain::Locator::GEOCENTRIC)
            {
                convertXYZToLatLongHeight(locator->getEllipsoidModel(), w00);
                convertXYZToLatLongHeight(locator->getEllipsoidModel(), w10);
                convertXYZToLatLongHeight(locator->getEllipsoidModel(), w11);
                convertXYZToLatLongHeight(locator->getEllipsoidModel(), w01);
            }

            osg::notify(osg::NOTICE)<<"    w00 = "<<w00<<std::endl;
            osg::notify(osg::NOTICE)<<"    w10 = "<<w10<<std::endl;
            osg::notify(osg::NOTICE)<<"    w11 = "<<w11<<std::endl;
            osg::notify(osg::NOTICE)<<"    w01 = "<<w01<<std::endl;
        }
        
        if (!s_ExitApplication) traverse(group);
    }

    void apply(osg::PagedLOD& plod)
    {
        if (_currentLevel>_maxLevels) return;
        
        if (s_ExitApplication) return;
    
        ++_currentLevel;
    
        std::cout<<"Found PagedLOD "<<plod.getNumFileNames()<<std::endl;
        
        // first compute the bounds of this subgraph
        for(unsigned int i=0; i<plod.getNumFileNames(); ++i)
        {
            if (plod.getFileName(i).empty())
            {
                std::cout<<"  search local subgraph"<<std::endl;
                traverse(plod);
            }
        }        
        
        for(unsigned int i=0; i<plod.getNumFileNames(); ++i)
        {
            std::cout<<"   filename["<<i<<"] "<<plod.getFileName(i)<<std::endl;
            if (!plod.getFileName(i).empty())
            {
                std::string filename;
                if (!plod.getDatabasePath().empty()) 
                {
                    filename = plod.getDatabasePath() + plod.getFileName(i);
                }
                else
                {
                    filename = plod.getFileName(i);
                }
                
                std::cout<<"   reading "<<filename<<std::endl;
                
                osg::ref_ptr<osg::Node> node = osgDB::readNodeFile(filename);
                
                if (!s_ExitApplication && node.valid()) node->accept(*this);
            }
        }

        --_currentLevel;
    }
    
protected:

    void convertXYZToLatLongHeight(osg::EllipsoidModel* em, osg::Vec3d& v)
    {
        em->convertXYZToLatLongHeight(v.x(), v.y(), v.z(),
                                      v.x(), v.y(), v.z());
                                      
        v.x() = osg::RadiansToDegrees(v.x());
        v.y() = osg::RadiansToDegrees(v.y());
    }

    unsigned int _maxLevels;
    unsigned int _currentLevel;
};

static void signalHandler(int sig)
{
    printf("\nCaught signal %d, requesting exit...\n\n",sig);
    s_ExitApplication = true;
}

int main( int argc, char **argv )
{
#ifndef _WIN32
    signal(SIGHUP, signalHandler);
    signal(SIGQUIT, signalHandler);
    signal(SIGKILL, signalHandler);
    signal(SIGUSR1, signalHandler);
    signal(SIGUSR2, signalHandler);
#endif
    signal(SIGABRT, signalHandler);
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);


    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);
    
    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setApplicationName(arguments.getApplicationName());
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is an application for collecting a set of seperate files into a single archive file that can be later read in OSG applications..");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] filename ...");
        
    // if user request help write it out to cout.
    if (arguments.read("-h") || arguments.read("--help"))
    {
        arguments.getApplicationUsage()->write(std::cout);
        return 1;
    }
    
    unsigned int maxLevels = 0;
    while(arguments.read("-l",maxLevels)) {}
    
    std::string fileCache;
    while(arguments.read("--file-cache",fileCache) || arguments.read("-c",fileCache)) {}
    
    if (!fileCache.empty())
    {   
        std::string str("OSG_FILE_CACHE=");
        str += fileCache;
        
        putenv(strdup((char*)str.c_str()));
    }

    osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFiles(arguments);
    if (!loadedModel)
    {
        std::cout<<"No data loaded, please specify a database to load"<<std::endl;
        return 1;
    }
    
    LoadDataVisitor ldv(maxLevels);
    
    loadedModel->accept(ldv);

    if (s_ExitApplication)
    {
        std::cout<<"osgfilecache cleanly exited in response to signal."<<std::endl;
    }

    return 0;
}

