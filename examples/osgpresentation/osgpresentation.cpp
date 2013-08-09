/* Copyright Robert Osfield, Licensed under the GPL
 *
 * Experimental base for refactor of Present3D
 *
*/

#include <osg/Geode>
#include <osg/Geometry>
#include <osg/ScriptEngine>
#include <osg/UserDataContainer>

#include <osgDB/ReadFile>
#include <osgDB/FileUtils>

#include <osgGA/TrackballManipulator>
#include <osgViewer/Viewer>


int main(int argc, char** argv)
{
    osg::ArgumentParser arguments(&argc, argv);

    osgViewer::Viewer viewer(arguments);

    typedef std::list< osg::ref_ptr<osg::Script> > Scripts;
    Scripts scripts;

    std::string filename;
    while(arguments.read("--script",filename))
    {
        osg::ref_ptr<osg::Script> script = osgDB::readFile<osg::Script>(filename);
        if (script.valid()) scripts.push_back(script.get());
    }

    // create the model
    osg::ref_ptr<osg::Node> model = osgDB::readNodeFiles(arguments);
    if (!model) return 1;

    // assgin script engine to scene graphs
    model->getOrCreateUserDataContainer()->addUserObject(osgDB::readFile<osg::ScriptEngine>("ScriptEngine.lua"));
    model->getOrCreateUserDataContainer()->addUserObject(osgDB::readFile<osg::ScriptEngine>("ScriptEngine.python"));
    model->getOrCreateUserDataContainer()->addUserObject(osgDB::readFile<osg::ScriptEngine>("ScriptEngine.js"));

    // assign scripts to scene graph
    for(Scripts::iterator itr = scripts.begin();
        itr != scripts.end();
        ++itr)
    {
       model->addUpdateCallback(new osg::ScriptCallback(itr->get()));
    }

#if 0
    std::string str;
    osg::ref_ptr<osg::ScriptEngine> luaScriptEngine = osgDB::readFile<osg::ScriptEngine>("ScriptEngine.lua");
    if (luaScriptEngine.valid())
    {
        while (arguments.read("--lua", str))
        {
            osg::ref_ptr<osg::Script> script = osgDB::readFile<osg::Script>(str);
            if (script.valid())
            {
                luaScriptEngine->run(script.get());
            }
        }
    }

    osg::ref_ptr<osg::ScriptEngine> v8ScriptEngine = osgDB::readFile<osg::ScriptEngine>("ScriptEngine.V8");
    if (v8ScriptEngine.valid())
    {
        while (arguments.read("--js",str))
        {
            osg::ref_ptr<osg::Script> script = osgDB::readFile<osg::Script>(str);
            if (script.valid())
            {
                v8ScriptEngine->run(script.get());
            }
        }
    }


    osg::ref_ptr<osg::ScriptEngine> pythonScriptEngine = osgDB::readFile<osg::ScriptEngine>("ScriptEngine.python");
    if (pythonScriptEngine.valid())
    {
        while (arguments.read("--python",str))
        {
            osg::ref_ptr<osg::Script> script = osgDB::readFile<osg::Script>(str);
            if (script.valid())
            {
                pythonScriptEngine->run(script.get());
            }
        }
    }
#endif


    viewer.setSceneData( model.get() );
    return viewer.run();
}
