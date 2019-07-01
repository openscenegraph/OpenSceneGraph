/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield
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

#include <osgSim/LineOfSight>

#include <osg/Notify>
#include <osgDB/ReadFile>
#include <osgUtil/LineSegmentIntersector>

using namespace osgSim;

DatabaseCacheReadCallback::DatabaseCacheReadCallback()
{
    _maxNumFilesToCache = 2000;
}

void DatabaseCacheReadCallback::clearDatabaseCache()
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
    _filenameSceneMap.clear();
}

void DatabaseCacheReadCallback::pruneUnusedDatabaseCache()
{
}

osg::ref_ptr<osg::Node> DatabaseCacheReadCallback::readNodeFile(const std::string& filename)
{
    // first check to see if file is already loaded.
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);

        FileNameSceneMap::iterator itr = _filenameSceneMap.find(filename);
        if (itr != _filenameSceneMap.end())
        {
            OSG_INFO<<"Getting from cache "<<filename<<std::endl;

            return itr->second.get();
        }
    }

    // now load the file.
    osg::ref_ptr<osg::Node> node = osgDB::readRefNodeFile(filename);

    // insert into the cache.
    if (node.valid())
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);

        if (_filenameSceneMap.size() < _maxNumFilesToCache)
        {
            OSG_INFO<<"Inserting into cache "<<filename<<std::endl;

            _filenameSceneMap[filename] = node;
        }
        else
        {
            // for time being implement a crude search for a candidate to chuck out from the cache.
            for(FileNameSceneMap::iterator itr = _filenameSceneMap.begin();
                itr != _filenameSceneMap.end();
                ++itr)
            {
                if (itr->second->referenceCount()==1)
                {
                    OSG_INFO<<"Erasing "<<itr->first<<std::endl;
                    // found a node which is only referenced in the cache so we can discard it
                    // and know that the actual memory will be released.
                    _filenameSceneMap.erase(itr);
                    break;
                }
            }
            OSG_INFO<<"And the replacing with "<<filename<<std::endl;
            _filenameSceneMap[filename] = node;
        }
    }

    return node;
}

LineOfSight::LineOfSight()
{
    setDatabaseCacheReadCallback(new DatabaseCacheReadCallback);
}

void LineOfSight::clear()
{
    _LOSList.clear();
}

unsigned int LineOfSight::addLOS(const osg::Vec3d& start, const osg::Vec3d& end)
{
    unsigned int index = _LOSList.size();
    _LOSList.push_back(LOS(start,end));
    return index;
}

void LineOfSight::computeIntersections(osg::Node* scene, osg::Node::NodeMask traversalMask)
{
    osg::ref_ptr<osgUtil::IntersectorGroup> intersectorGroup = new osgUtil::IntersectorGroup();

    for(LOSList::iterator itr = _LOSList.begin();
        itr != _LOSList.end();
        ++itr)
    {
        osg::ref_ptr<osgUtil::LineSegmentIntersector> intersector = new osgUtil::LineSegmentIntersector(itr->_start, itr->_end);
        intersectorGroup->addIntersector( intersector.get() );
    }

    _intersectionVisitor.reset();
    _intersectionVisitor.setTraversalMask(traversalMask);
    _intersectionVisitor.setIntersector( intersectorGroup.get() );

    scene->accept(_intersectionVisitor);

    unsigned int index = 0;
    osgUtil::IntersectorGroup::Intersectors& intersectors = intersectorGroup->getIntersectors();
    for(osgUtil::IntersectorGroup::Intersectors::iterator intersector_itr = intersectors.begin();
        intersector_itr != intersectors.end();
        ++intersector_itr, ++index)
    {
        osgUtil::LineSegmentIntersector* lsi = dynamic_cast<osgUtil::LineSegmentIntersector*>(intersector_itr->get());
        if (lsi)
        {
            Intersections& intersectionsLOS = _LOSList[index]._intersections;
            _LOSList[index]._intersections.clear();

            osgUtil::LineSegmentIntersector::Intersections& intersections = lsi->getIntersections();

            for(osgUtil::LineSegmentIntersector::Intersections::iterator itr = intersections.begin();
                itr != intersections.end();
                ++itr)
            {
                const osgUtil::LineSegmentIntersector::Intersection& intersection = *itr;
                if (intersection.matrix.valid()) intersectionsLOS.push_back( intersection.localIntersectionPoint * (*intersection.matrix) );
                else intersectionsLOS.push_back( intersection.localIntersectionPoint  );
            }
        }
    }

}

LineOfSight::Intersections LineOfSight::computeIntersections(osg::Node* scene, const osg::Vec3d& start, const osg::Vec3d& end, osg::Node::NodeMask traversalMask)
{
    LineOfSight los;
    unsigned int index = los.addLOS(start,end);
    los.computeIntersections(scene, traversalMask);
    return los.getIntersections(index);
}

void LineOfSight::setDatabaseCacheReadCallback(DatabaseCacheReadCallback* dcrc)
{
    _dcrc = dcrc;
    _intersectionVisitor.setReadCallback(dcrc);
}
