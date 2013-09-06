/* Copyright Robert Osfield, Licensed under the GPL
 *
 * Experimental base for refactor of Present3D
 *
*/

#include <osg/Geode>
#include <osg/Geometry>
#include <osg/ScriptEngine>
#include <osg/UserDataContainer>

#include <osgPresentation/Presentation>
#include <osgPresentation/Slide>
#include <osgPresentation/Layer>
#include <osgPresentation/Element>
#include <osgPresentation/Model>
#include <osgPresentation/Volume>
#include <osgPresentation/Image>
#include <osgPresentation/Movie>
#include <osgPresentation/Text>
#include <osgPresentation/Audio>

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgDB/FileUtils>

#include <osgGA/TrackballManipulator>
#include <osgViewer/Viewer>


int main(int argc, char** argv)
{
    osg::ArgumentParser arguments(&argc, argv);

    osgViewer::Viewer viewer(arguments);

#if 0
    typedef std::list< osg::ref_ptr<osg::Script> > Scripts;
    Scripts scripts;

    std::string filename;
    while(arguments.read("--script",filename))
    {
        osg::ref_ptr<osg::Script> script = osgDB::readFile<osg::Script>(filename);
        if (script.valid()) scripts.push_back(script.get());
    }

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


    osg::ref_ptr<osgPresentation::Presentation> presentation = new osgPresentation::Presentation;
    osg::ref_ptr<osgPresentation::Slide> slide = new osgPresentation::Slide;
    osg::ref_ptr<osgPresentation::Layer> layer = new osgPresentation::Layer;
    osg::ref_ptr<osgPresentation::Group> group = new osgPresentation::Group;
    osg::ref_ptr<osgPresentation::Element> element = new osgPresentation::Element;
    osg::ref_ptr<osgPresentation::Text> text = new osgPresentation::Text;
    osg::ref_ptr<osgPresentation::Model> model = new osgPresentation::Model;
    osg::ref_ptr<osgPresentation::Audio> audio = new osgPresentation::Audio;
    osg::ref_ptr<osgPresentation::Image> image = new osgPresentation::Image;
    osg::ref_ptr<osgPresentation::Movie> movie = new osgPresentation::Movie;
    osg::ref_ptr<osgPresentation::Volume> volume = new osgPresentation::Volume;
    presentation->addChild(slide.get());
    slide->addChild(layer.get());
    //layer->addChild(element.get());
    //layer->addChild(group.get());
    layer->addChild(element.get());
    // layer->addChild(model.get());
    layer->addChild(text.get());
    layer->addChild(audio.get());
    layer->addChild(image.get());
    layer->addChild(movie.get());
    layer->addChild(volume.get());

    text->setProperty("string",std::string("This is a first test"));
    text->setProperty("font",std::string("times.ttf"));
    text->setProperty("character_size",2.2);
    text->setProperty("width",std::string("103.2"));

    model->setProperty("filename", std::string("dumptruck.osgt"));

    image->setProperty("filename", std::string("Images/lz.rgb"));
    image->setProperty("scale",0.75);

    movie->setProperty("filename", std::string("/home/robert/Data/Movie/big_buck_bunny_1080p_stereo.ogg"));
    movie->setProperty("scale",0.75);

    volume->setProperty("filename", std::string("/home/robert/Data/MaleVisibleHumanHead"));
    volume->setProperty("scale",0.75);
    volume->setProperty("technique",std::string("iso-surface"));

    presentation->setProperty("scale",1.0);


    osgPresentation::PrintSupportedProperties psp(std::cout);
    presentation->accept(psp);

    osgPresentation::PrintProperties pp(std::cout);
    presentation->accept(pp);

    osgPresentation::LoadAction load;
    presentation->accept( load );

    viewer.setSceneData( presentation.get() );


    osgDB::writeNodeFile(*presentation, "pres.osgt");

    return viewer.run();
}
