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

#include <osgDB/Archive>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgDB/FileUtils>

#include <iostream>
#include <algorithm>

class LoadDataVisitor : public osg::NodeVisitor
{
public:

    LoadDataVisitor(unsigned int maxNumLevels=0):
        osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
        _maxLevels(maxNumLevels),
        _currentLevel(0) {}
        
    void apply(osg::PagedLOD& plod)
    {
        if (_currentLevel>_maxLevels) return;
    
        ++_currentLevel;
    
        std::cout<<"Found PagedLOD "<<plod.getNumFileNames()<<std::endl;
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
                
                if (node.valid()) node->accept(*this);
            }
        }

        --_currentLevel;
    }
    
protected:

    unsigned int _maxLevels;
    unsigned int _currentLevel;
};


int main( int argc, char **argv )
{
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
    while(arguments.read("-c",fileCache)) {}
    
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

    return 0;
}

