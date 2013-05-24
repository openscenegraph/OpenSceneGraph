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


namespace osgViewer
{
    

struct SceneSingleton
{
    SceneSingleton() {}

    inline void add(Scene* scene)
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
        _cache.push_back(scene);
    }

    inline void remove(Scene* scene)
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
        for(SceneCache::iterator itr = _cache.begin();
            itr != _cache.end();
            ++itr)
        {
            if (scene==itr->get())
            {
                _cache.erase(itr);
                break;
            }
        }
    }

    inline Scene* getScene(osg::Node* node)
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
        for(SceneCache::iterator itr = _cache.begin();
            itr != _cache.end();
            ++itr)
        {
            Scene* scene = itr->get();
            if (scene && scene->getSceneData()==node) return scene;
        }
        return 0;
    }

    typedef std::vector< osg::observer_ptr<Scene> >  SceneCache;
    SceneCache          _cache;
    OpenThreads::Mutex  _mutex;
};

static SceneSingleton& getSceneSingleton()
{
    static SceneSingleton s_sceneSingleton;
    return s_sceneSingleton;
}

// Use a proxy to force the initialization of the SceneSingleton during static initialization
OSG_INIT_SINGLETON_PROXY(SceneSingletonProxy, getSceneSingleton())

}

Scene::Scene():
    osg::Referenced(true)
{
    setDatabasePager(osgDB::DatabasePager::create());
    setImagePager(new osgDB::ImagePager);
    getSceneSingleton().add(this);
}

Scene::~Scene()
{
    getSceneSingleton().remove(this);
}

void Scene::setSceneData(osg::Node* node)
{
    _sceneData = node;
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

    if (getSceneData())
    {
        updateVisitor.setImageRequestHandler(getImagePager());
        getSceneData()->accept(updateVisitor);
    }
}


Scene* Scene::getScene(osg::Node* node)
{
    return getSceneSingleton().getScene(node);
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
