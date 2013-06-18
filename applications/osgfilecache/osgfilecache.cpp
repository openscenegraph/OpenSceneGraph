/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield 
 *
 * This application is open source and may be redistributed and/or modified   
 * freely and without restriction, both in commercial and non commercial applications,
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
#include <osg/Geometry>
#include <osg/PagedLOD>
#include <osg/io_utils>

#include <osgTerrain/TerrainTile>

#include <osgDB/Archive>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgDB/FileUtils>
#include <osgDB/FileCache>
#include <osgDB/FileNameUtils>

#include <iostream>
#include <algorithm>

#include <signal.h>
#include <stdlib.h>

static bool s_ExitApplication = false;

struct Extents
{

    Extents():
        _maxLevel(0),
        _min(DBL_MAX,DBL_MAX),
        _max(-DBL_MAX,-DBL_MAX) {}

    Extents(unsigned int maxLevel, double minX, double minY, double maxX, double maxY):
        _maxLevel(maxLevel),
        _min(minX, minY),
        _max(maxX, maxY) {}
    
    Extents(const Extents& extents):
        _maxLevel(extents._maxLevel),
        _min(extents._min),
        _max(extents._max) {}
        
    Extents& operator = (const Extents& rhs)
    {
        if (&rhs == this) return *this;
        
        _maxLevel = rhs._maxLevel;
        _min = rhs._min;
        _max = rhs._max;
                
        return *this;
    }

    bool intersects(unsigned level, osg::Vec2d& in_min, osg::Vec2d& in_max)
    {
        osg::notify(osg::INFO)<<"intersects("<<level<<", min="<<in_min<<" max="<<in_max<<")"<<std::endl;
        osg::notify(osg::INFO)<<"  _maxLevel="<<_maxLevel<<", _min="<<_min<<" _max="<<_max<<std::endl;

        if (level>_maxLevel) return false;
        
        osg::Vec2d union_min, union_max;

        // handle mins        
        if (_min.x()!=DBL_MAX && in_min.x()!=DBL_MAX)
        {
            // both _min.x() and in_min.x() are defined so use max of two
            union_min.x() = _min.x()>in_min.x() ? _min.x() : in_min.x();
        }
        else
        {
            // one or both _min.x() and in_min.x() aren't defined so use min of two
            union_min.x() = _min.x()<in_min.x() ? _min.x() : in_min.x();
        }

        if (_min.y()!=DBL_MAX && in_min.y()!=DBL_MAX)
        {
            // both _min.y() and in_min.y() are defined so use max of two
            union_min.y() = _min.y()>in_min.y() ? _min.y() : in_min.y();
        }
        else
        {
            // one or both _min.y() and in_min.y() aren't defined so use min of two
            union_min.y() = _min.y()<in_min.y() ? _min.y() : in_min.y();
        }

        // handle maxs        
        if (_max.x()!=-DBL_MAX && in_max.x()!=-DBL_MAX)
        {
            // both _max.x() and in_max.x() are defined so use max of two
            union_max.x() = _max.x()<in_max.x() ? _max.x() : in_max.x();
        }
        else
        {
            // one or both _max.x() and in_max.x() aren't defined so use max of two
            union_max.x() = _max.x()>in_max.x() ? _max.x() : in_max.x();
        }

        if (_max.y()!=-DBL_MAX && in_max.y()!=-DBL_MAX)
        {
            // both _max.y() and in_max.y() are defined so use max of two
            union_max.y() = _max.y()<in_max.y() ? _max.y() : in_max.y();
        }
        else
        {
            // one or both _max.y() and in_max.y() aren't defined so use max of two
            union_max.y() = _max.y()>in_max.y() ? _max.y() : in_max.y();
        }

        bool result = union_min.x()<union_max.x() && union_min.y()<union_max.y() ;

        osg::notify(osg::INFO)<<"  union_min="<<union_min<<" union_max="<<union_max<<" result = "<<result<<std::endl;
        
        return result;
    }

    unsigned int    _maxLevel;
    osg::Vec2d      _min;
    osg::Vec2d      _max;
};

class LoadDataVisitor : public osg::NodeVisitor
{
public:


    LoadDataVisitor():
        osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
        _currentLevel(0) {}

    void setFileCache(osgDB::FileCache* fileCache) { _fileCache = fileCache; }
        
    void addExtents(unsigned int maxLevel, double minX, double minY, double maxX, double maxY)
    {
        _extentsList.push_back(Extents(maxLevel, minX, minY, maxX, maxY));
    }

    void addExtents(unsigned int maxLevel)
    {
        _extentsList.push_back(Extents(maxLevel, DBL_MAX, DBL_MAX, -DBL_MAX, -DBL_MAX));
    }

    void apply(osg::CoordinateSystemNode& cs)
    {
        _csnStack.push_back(&cs);
        
        if (!s_ExitApplication) traverse(cs);

        _csnStack.pop_back();
    }
    
    void apply(osg::Group& group)
    {
        if (s_ExitApplication) return;
    
        osgTerrain::TerrainTile* terrainTile = dynamic_cast<osgTerrain::TerrainTile*>(&group);
        osgTerrain::Locator* locator = terrainTile ? terrainTile->getLocator() : 0;
        if (locator)
        {
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

            updateBound(w00);
            updateBound(w10);
            updateBound(w11);
            updateBound(w01);

            return;
        }
        
        traverse(group);
    }

    void apply(osg::Transform& transform)
    {
        osg::Matrix matrix;
        if (!_matrixStack.empty()) matrix = _matrixStack.back();

        transform.computeLocalToWorldMatrix(matrix,this);

        pushMatrix(matrix);

        traverse(transform);

        popMatrix();
    }

    void apply(osg::PagedLOD& plod)
    {
        if (s_ExitApplication) return;
    
        ++_currentLevel;
        
        initBound();
        
        // first compute the bounds of this subgraph
        for(unsigned int i=0; i<plod.getNumFileNames(); ++i)
        {
            if (plod.getFileName(i).empty())
            {
                traverse(plod);
            }
        }
        
        if (intersects())
        {
            for(unsigned int i=0; i<plod.getNumFileNames(); ++i)
            {
                osg::notify(osg::INFO)<<"   filename["<<i<<"] "<<plod.getFileName(i)<<std::endl;
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


                    osg::ref_ptr<osg::Node> node = readNodeFileAndWriteToCache(filename);

                    if (!s_ExitApplication && node.valid()) node->accept(*this);
                }
            }
        }
        
        --_currentLevel;
    }
    
    void apply(osg::Geode& geode)
    {
        for(unsigned int i=0; i<geode.getNumDrawables(); ++i)
        {
            osg::Geometry* geom = geode.getDrawable(i)->asGeometry();
            if (geom)
            {
                osg::Vec3Array* vertices = dynamic_cast<osg::Vec3Array*>(geom->getVertexArray());
                if (vertices) updateBound(*vertices);
            }
        }
    }
    
    osg::Node* readNodeFileAndWriteToCache(const std::string& filename)
    {
        osg::Node* node = 0;
        if (_fileCache.valid() )
        {
            if (_fileCache->existsInCache(filename))
            {
                osg::notify(osg::NOTICE)<<"reading from FileCache: "<<filename<<std::endl;
                node = _fileCache->readNode(filename, osgDB::Registry::instance()->getOptions()).takeNode();
            }
            else
            {
                osg::notify(osg::NOTICE)<<"reading : "<<filename<<std::endl;

                node = osgDB::readNodeFile(filename);
                if (node)
                {
                    osg::notify(osg::NOTICE)<<"write to FileCache : "<<filename<<std::endl;

                    _fileCache->writeNode(*node, filename, osgDB::Registry::instance()->getOptions());
                }
            }
        }
        else
        {
            osg::notify(osg::NOTICE)<<"reading : "<<filename<<std::endl;
            node = osgDB::readNodeFile(filename);
        }
        return node;
    }


protected:

    inline void pushMatrix(osg::Matrix& matrix) { _matrixStack.push_back(matrix); }
    
    inline void popMatrix() { _matrixStack.pop_back(); }

    void convertXYZToLatLongHeight(osg::EllipsoidModel* em, osg::Vec3d& v)
    {
        em->convertXYZToLatLongHeight(v.x(), v.y(), v.z(),
                                      v.y(), v.x(), v.z());
                                      
        v.x() = osg::RadiansToDegrees(v.x());
        v.y() = osg::RadiansToDegrees(v.y());
    }
    
    void initBound()
    {
        _min.set(DBL_MAX, DBL_MAX);
        _max.set(-DBL_MAX, -DBL_MAX);
    }

    void updateBound(osg::Vec3d& v)
    {
        if (v.x() < _min.x()) _min.x() = v.x();
        if (v.y() < _min.y()) _min.y() = v.y();
        if (v.x() > _max.x()) _max.x() = v.x();
        if (v.y() > _max.y()) _max.y() = v.y();
    }
    
    void updateBound(osg::Vec3Array& vertices)
    {
        // set up matrix
        osg::Matrix matrix;
        if (!_matrixStack.empty()) matrix = _matrixStack.back();
        
        // set up ellipsoid model
        osg::EllipsoidModel* em = !_csnStack.empty() ?  _csnStack.back()->getEllipsoidModel() : 0;
        
        for(osg::Vec3Array::iterator itr = vertices.begin();
            itr != vertices.end();
            ++itr)
        {
            osg::Vec3d v = osg::Vec3d(*itr) * matrix;
            if (em) convertXYZToLatLongHeight(em, v);

            updateBound(v);
        }
    }
    
    bool intersects()
    {
        osg::notify(osg::INFO)<<"intersects() _min = "<<_min<<" _max = "<<_max<<std::endl;
        for(ExtentsList::iterator itr = _extentsList.begin();
            itr != _extentsList.end();
            ++itr)
        {
            if (itr->intersects(_currentLevel, _min, _max)) return true;
        }
        
        return false;
    }

    typedef std::vector<Extents>                    ExtentsList;
    typedef std::vector<osg::Matrix>                MatrixStack;
    typedef std::vector<osg::CoordinateSystemNode*> CSNStack;

    osg::ref_ptr<osgDB::FileCache>  _fileCache;

    ExtentsList     _extentsList;
    unsigned int    _currentLevel;
    MatrixStack     _matrixStack;
    CSNStack        _csnStack;
    
    osg::Vec2d      _min;
    osg::Vec2d      _max;
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
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is an application for collecting a set of separate files into a single archive file that can be later read in OSG applications..");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] filename ...");
    arguments.getApplicationUsage()->addCommandLineOption("-l level","Read down to level across the whole database.");
    arguments.getApplicationUsage()->addCommandLineOption("-e level minX minY maxX maxY","Read down to <level> across the extents minX, minY to maxY, maxY.  Note, for geocentric datase X and Y are longitude and latitude respectively.");
    arguments.getApplicationUsage()->addCommandLineOption("-c directory","Shorthand for --file-cache directory.");
    arguments.getApplicationUsage()->addCommandLineOption("--file-cache directory","Set directory as to place cache download files.");
        
    // if user request help write it out to cout.
    if (arguments.read("-h") || arguments.read("--help"))
    {
        arguments.getApplicationUsage()->write(std::cout);
        return 1;
    }
    
    LoadDataVisitor ldv;

    std::string fileCachePath;
    while(arguments.read("--file-cache",fileCachePath) || arguments.read("-c",fileCachePath)) {}
    
    if (fileCachePath.empty())
    {   
        const char* env_fileCachePath = getenv("OSG_FILE_CACHE");
        if (env_fileCachePath)
        {
            fileCachePath = env_fileCachePath;
        }
    }

    if (fileCachePath.empty())
    {
        std::cout<<"No path to the file cache defined, please set OSG_FILE_PATH env var, or use --file-cache <directory> to set a suitable directory for the file cache."<<std::endl;
        return 1;
    }
    
    ldv.setFileCache(new osgDB::FileCache(fileCachePath));

    unsigned int maxLevels = 0;
    while(arguments.read("-l",maxLevels))
    {
        ldv.addExtents(maxLevels);
    }
    
    double minX, maxX, minY, maxY;
    while(arguments.read("-e",maxLevels, minX, minY, maxX, maxY))
    {
        ldv.addExtents(maxLevels, minX, minY, maxX, maxY);
    }

    
    std::string filename;
    for(int i=1; i<arguments.argc(); ++i)
    {
        if (!arguments.isOption(i))
        {
            filename = arguments[i];
            break;
        }
    }
    
    if (filename.empty()) 
    {
        std::cout<<"No file to load specified."<<std::endl;
        return 1;
    }
    
    osg::ref_ptr<osg::Node> loadedModel = ldv.readNodeFileAndWriteToCache(filename);
    if (!loadedModel)
    {
        std::cout<<"No data loaded, please specify a database to load"<<std::endl;
        return 1;
    }
    
    
    loadedModel->accept(ldv);

    if (s_ExitApplication)
    {
        std::cout<<"osgfilecache cleanly exited in response to signal."<<std::endl;
    }

    return 0;
}

