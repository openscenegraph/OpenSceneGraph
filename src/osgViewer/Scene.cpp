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

#include <osgViewer/Scene>
#include <osgGA/EventVisitor>

using namespace osgViewer;

typedef std::vector< osg::observer_ptr<Scene> >  SceneCache;
static OpenThreads::Mutex s_sceneCacheMutex;
static SceneCache s_sceneCache;

Scene::Scene():
    osg::Referenced(true)
{
    setDatabasePager(osgDB::DatabasePager::create());
    setImagePager(new osgDB::ImagePager);
    
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_sceneCacheMutex);
    s_sceneCache.push_back(this);
}

Scene::~Scene()
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_sceneCacheMutex);
    for(SceneCache::iterator itr = s_sceneCache.begin();
        itr != s_sceneCache.end();
        ++itr)
    {
        Scene* scene = itr->get();
        if (scene==this)
        {
            s_sceneCache.erase(itr);
            break;
        }
    }
}

void Scene::setSceneData(osg::Node* node)
{
    _sceneData = node;
    
    if (_databasePager.valid())
    {    
        // register any PagedLOD that need to be tracked in the scene graph
        if (node) _databasePager->registerPagedLODs(node);
    }
}

osg::Node* Scene::getSceneData()
{
    return _sceneData.get();
}

const osg::Node* Scene::getSceneData() const
{
    return _sceneData.get();
}

void Scene::setDatabasePager(osgDB::DatabasePager* dp)
{
    _databasePager = dp;
}

void Scene::setImagePager(osgDB::ImagePager* ip)
{
    _imagePager = ip;
}

void Scene::updateSceneGraph(osg::NodeVisitor& updateVisitor)
{
    if (!_sceneData) return;

    if (getSceneData())
    {
        updateVisitor.setImageRequestHandler(getImagePager());
        getSceneData()->accept(updateVisitor);
    }

    if (getDatabasePager())
    {
        // synchronize changes required by the DatabasePager thread to the scene graph
        getDatabasePager()->updateSceneGraph((*updateVisitor.getFrameStamp()));
    }

    if (getImagePager())
    {
        // synchronize changes required by the DatabasePager thread to the scene graph
        getImagePager()->updateSceneGraph(*(updateVisitor.getFrameStamp()));
    }


}


Scene* Scene::getScene(osg::Node* node)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_sceneCacheMutex);
    for(SceneCache::iterator itr = s_sceneCache.begin();
        itr != s_sceneCache.end();
        ++itr)
    {
        Scene* scene = itr->get();
        if (scene && scene->getSceneData()==node) return scene;
    }
    return 0;
}

Scene* Scene::getOrCreateScene(osg::Node* node)
{
    if (!node) return 0; 

    osgViewer::Scene* scene = getScene(node);
    if (!scene) 
    {
        scene = new Scene;
        scene->setSceneData(node);
    }

    return scene;
}
