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

#include <osg/io_utils>

#include<osgDB/ClassInterface>

int main(int argc, char** argv)
{
    osg::ArgumentParser arguments(&argc, argv);

    osgViewer::Viewer viewer(arguments);

#if 0
    typedef std::list< osg::ref_ptr<osg::Script> > Scripts;
    Scripts scripts;

    osg::ref_ptr<osg::Group> model = new osg::Group;

    std::string filename;
    while(arguments.read("--script",filename))
    {
        osg::ref_ptr<osg::Script> script = osgDB::readScriptFile(filename);
        if (script.valid()) scripts.push_back(script.get());
    }

    // assign script engine to scene graphs
    model->getOrCreateUserDataContainer()->addUserObject(osgDB::readFile<osg::ScriptEngine>("ScriptEngine.lua"));
    model->getOrCreateUserDataContainer()->addUserObject(osgDB::readFile<osg::ScriptEngine>("ScriptEngine.python"));
    model->getOrCreateUserDataContainer()->addUserObject(osgDB::readFile<osg::ScriptEngine>("ScriptEngine.js"));

    // assign scripts to scene graph
    for(Scripts::iterator itr = scripts.begin();
        itr != scripts.end();
        ++itr)
    {
       model->addUpdateCallback(new osg::ScriptNodeCallback(itr->get()));
    }

    std::string str;
    osg::ref_ptr<osg::ScriptEngine> luaScriptEngine = osgDB::readFile<osg::ScriptEngine>("ScriptEngine.lua");
    if (luaScriptEngine.valid())
    {
        while (arguments.read("--lua", str))
        {
            osg::ref_ptr<osg::Script> script = osgDB::readScriptFile(str);
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
            osg::ref_ptr<osg::Script> script = osgDB::readScriptFile(str);
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
            osg::ref_ptr<osg::Script> script = osgDB::readScriptFile(str);
            if (script.valid())
            {
                pythonScriptEngine->run(script.get());
            }
        }
    }

    return 0;
#endif

#if 1

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

#if 0
    osgPresentation::PrintSupportedProperties psp(std::cout);
    presentation->accept(psp);

    osgPresentation::PrintProperties pp(std::cout);
    presentation->accept(pp);
#endif

    osgPresentation::LoadAction load;
    presentation->accept( load );

    viewer.setSceneData( presentation.get() );


    osgDB::writeNodeFile(*presentation, "pres.osgt");

    osgDB::ClassInterface pi;

    pi.getWhiteList()["osgPresentation::Presentation"]["filename"]=osgDB::BaseSerializer::RW_STRING;
    pi.getBlackList()["osgPresentation::Presentation"]["Children"];
    pi.getBlackList()["osgPresentation::Presentation"]["UserDataContainer"];
    pi.getBlackList()["osgPresentation::Presentation"]["UserData"];
    pi.getBlackList()["osgPresentation::Presentation"]["CullCallback"];
    pi.getBlackList()["osgPresentation::Presentation"]["ComputeBoundingSphereCallback"];

#if 0
    osgDB::ObjectWrapperManager* owm = osgDB::Registry::instance()->getObjectWrapperManager();
    if (owm)
    {
        const osgDB::ObjectWrapperManager::WrapperMap& wrapperMap = owm->getWrapperMap();
        for(osgDB::ObjectWrapperManager::WrapperMap::const_iterator itr = wrapperMap.begin();
            itr != wrapperMap.end();
            ++itr)
        {
            osgDB::ObjectWrapper* ow = itr->second.get();

            OSG_NOTICE<<std::endl<<"Wrapper : "<<itr->first<<", Domain="<<ow->getDomain()<<", Name="<<ow->getName()<<std::endl;

            const osgDB::StringList& associates = ow->getAssociates();
            for(osgDB::StringList::const_iterator aitr = associates.begin();
                aitr != associates.end();
                ++aitr)
            {
                OSG_NOTICE<<"    associate = "<<*aitr<<std::endl;
            }


            osgDB::StringList properties;
            osgDB::ObjectWrapper::TypeList types;
            ow->writeSchema(properties, types);
            OSG_NOTICE<<"  properties.size() = "<<properties.size()<<", types.size() = "<<types.size()<<std::endl;
            unsigned int numProperties = std::min(properties.size(), types.size());
            for(unsigned int i=0; i<numProperties; ++i)
            {
                OSG_NOTICE<<"     property = "<<properties[i]<<", type = "<<types[i]<<", typeName = "<<pi.getTypeName(types[i])<<std::endl;
            }



        }
#if 1
        osgDB::ObjectWrapperManager::IntLookupMap& intLookupMap = owm->getLookupMap();
        for(osgDB::ObjectWrapperManager::IntLookupMap::iterator itr = intLookupMap.begin();
            itr != intLookupMap.end();
            ++itr)
        {
            OSG_NOTICE<<std::endl<<"IntLookMap["<<itr->first<<"]"<<std::endl;
            osgDB::IntLookup::StringToValue& stv = itr->second.getStringToValue();
            for(osgDB::IntLookup::StringToValue::iterator sitr = stv.begin();
                sitr != stv.end();
                ++sitr)
            {
                OSG_NOTICE<<"   "<<sitr->first<<", "<<sitr->second<<std::endl;
            }
        }
#endif
    }
#endif



    presentation->setName("[this is a test]");

#if 0

    if (pi.setProperty(presentation.get(), "Name", std::string("[this is new improved test]")))
    {
        OSG_NOTICE<<"setProperty(presentation.get(), Name) succeeded."<<std::endl;
    }
    else
    {
        OSG_NOTICE<<"setProperty(presentation.get(), Name) failed."<<std::endl;
    }

    std::string name;
    if (pi.getProperty(presentation.get(), "Name", name))
    {
        OSG_NOTICE<<"getProperty(presentation.get(), Name) succeeded, Name = "<<name<<std::endl;
    }
    else
    {
        OSG_NOTICE<<"getProperty(presentation.get(), Name) failed."<<std::endl;
    }


    OSG_NOTICE<<std::endl;
    // presentation->setDataVariance(osg::Object::DYNAMIC);

    int variance = 1234;
    if (pi.getProperty(presentation.get(), "DataVariance", variance))
    {
        OSG_NOTICE<<"getProperty(presentation.get(), DataVariance) succeeded, variance = "<<variance<<std::endl;
    }
    else
    {
        OSG_NOTICE<<"getProperty(presentation.get(), DataVariance) failed."<<std::endl;
    }

    OSG_NOTICE<<std::endl;


    if (pi.setProperty(presentation.get(), "DataVariance", 1))
    {
        OSG_NOTICE<<"setProperty(presentation.get(), DataVariance) succeeded."<<std::endl;
    }
    else
    {
        OSG_NOTICE<<"setProperty(presentation.get(), DataVariance) failed."<<std::endl;
    }

    OSG_NOTICE<<std::endl;

    if (pi.getProperty(presentation.get(), "DataVariance", variance))
    {
        OSG_NOTICE<<"2nd getProperty(presentation.get(), DataVariance) succeeded, variance = "<<variance<<std::endl;
    }
    else
    {
        OSG_NOTICE<<"2nd getProperty(presentation.get(), DataVariance) failed."<<std::endl;
    }

    OSG_NOTICE<<std::endl;

    presentation->setMatrix(osg::Matrixd::translate(osg::Vec3d(1.0,2.0,3.0)));

//    if (pi.setProperty(presentation.get(), "Matrix", osg::Matrixd::scale(1.0,2.0,2.0)))
    if (pi.setProperty(presentation.get(), "Matrix", osg::Matrixd::scale(2.0,2.0,2.0)))
    {
        OSG_NOTICE<<"setProperty(..,Matrix) succeeded."<<std::endl;
    }
    else
    {
        OSG_NOTICE<<"setProperty(..,Matrix) failed."<<std::endl;
    }

    osg::Matrixd matrix;
    if (pi.getProperty(presentation.get(), "Matrix", matrix))
    {
        OSG_NOTICE<<"getProperty(presentation.get(), ...) succeeded, Matrix = "<<matrix<<std::endl;
    }
    else
    {
        OSG_NOTICE<<"getProperty(presentation.get(), ...) failed."<<std::endl;
    }
#if 1

    osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
    osg::ref_ptr<osg::Node> node = new osg::Node;
    osgDB::ClassInterface::PropertyMap properties;
    if (pi.getSupportedProperties(presentation.get(), properties, true))
    {
        OSG_NOTICE<<"Have supported properites found."<<std::endl;
        for(osgDB::ClassInterface::PropertyMap::iterator itr = properties.begin();
            itr != properties.end();
            ++itr)
        {
            OSG_NOTICE<<"   Property "<<itr->first<<", "<<pi.getTypeName(itr->second)<<std::endl;
        }
    }
    else
    {
        OSG_NOTICE<<"No supported properites found."<<std::endl;
    }


    OSG_NOTICE<<"Type(float) = "<<osgDB::getTypeEnum<float>()<<", "<<osgDB::getTypeString<float>()<<std::endl;
    OSG_NOTICE<<"Type(bool) = "<<osgDB::getTypeEnum<bool>()<<", "<<osgDB::getTypeString<bool>()<<std::endl;
    OSG_NOTICE<<"Type(osg::Vec3) = "<<osgDB::getTypeEnum<osg::Vec3>()<<", "<<osgDB::getTypeString<osg::Vec3>()<<std::endl;
    OSG_NOTICE<<"Type(osg::Matrixd) = "<<osgDB::getTypeEnum<osg::Matrixd>()<<", "<<osgDB::getTypeString<osg::Matrixd>()<<std::endl;
    OSG_NOTICE<<"Type(osg::Vec2ui) = "<<osgDB::getTypeEnum<osg::Vec2ui>()<<", "<<osgDB::getTypeString<osg::Vec2ui>()<<std::endl;
    OSG_NOTICE<<"Type(GLenum) = "<<osgDB::getTypeEnum<GLenum>()<<", "<<osgDB::getTypeString<GLenum>()<<std::endl;
    OSG_NOTICE<<"Type(int) = "<<osgDB::getTypeEnum<int>()<<", "<<osgDB::getTypeString<int>()<<std::endl;
    OSG_NOTICE<<"Type(osg::Image*) = "<<osgDB::getTypeEnum<osg::Image*>()<<", "<<osgDB::getTypeString<osg::Image*>()<<std::endl;
    OSG_NOTICE<<"Type(osg::Object*) = "<<osgDB::getTypeEnum<osg::Object*>()<<", "<<osgDB::getTypeString<osg::Object*>()<<std::endl;
    OSG_NOTICE<<"Type(osg::Referenced*) = "<<osgDB::getTypeEnum<osg::Referenced*>()<<", "<<osgDB::getTypeString<osg::Referenced*>()<<std::endl;

    osg::Object* ptr = presentation.get();
    OSG_NOTICE<<"Type(ptr) = "<<osgDB::getTypeEnumFromPtr(ptr)<<", "<<osgDB::getTypeStringFromPtr(ptr)<<std::endl;
    OSG_NOTICE<<"Type(presentation) = "<<osgDB::getTypeEnumFromPtr(presentation.get())<<", "<<osgDB::getTypeStringFromPtr(presentation.get())<<std::endl;

    osg::Image* image2  = 0;
    OSG_NOTICE<<"Type(image) = "<<osgDB::getTypeEnumFromPtr(image2)<<", "<<osgDB::getTypeStringFromPtr(image2)<<std::endl;

    osg::Vec3 pos;
    OSG_NOTICE<<"Type(pos) = "<<osgDB::getTypeEnumFrom(pos)<<", "<<osgDB::getTypeStringFrom(pos)<<std::endl;

    OSG_NOTICE<<"Type(std::string) = "<<osgDB::getTypeEnum<std::string>()<<", "<<osgDB::getTypeString<std::string>()<<std::endl;

    osgDB::BaseSerializer::Type type;
    if (pi.getPropertyType(presentation.get(), "Name", type))
    {
        OSG_NOTICE<<"Property Type, Name = "<< type<<std::endl;
    }
#endif

    osg::Matrixd mymatrix = osg::Matrix::translate(-1,2,3);
    pi.setProperty(presentation.get(), "mymatrix", mymatrix);

    osg::Matrixd copyofmatrix;
    if (pi.getProperty(presentation.get(), "mymatrix", copyofmatrix))
    {
        OSG_NOTICE<<"mymatrix = "<<copyofmatrix<<std::endl;
    }

    if (pi.getProperty(presentation.get(), "Matrix", copyofmatrix))
    {
        OSG_NOTICE<<"Matrix = "<<copyofmatrix<<std::endl;
    }

    std::string teststring="Another test";
    pi.setProperty(presentation.get(),"mystring",teststring);

    std::string astring;
    if (pi.getProperty(presentation.get(),"mystring",astring))
    {
        OSG_NOTICE<<"mystring = "<<astring<<std::endl;
    }
    else
    {
        OSG_NOTICE<<"failed to get mystring"<<std::endl;
    }

    #define PRINT_TYPE(O,PN) \
    { \
        osgDB::BaseSerializer::Type type; \
        if (pi.getPropertyType(O, #PN, type)) \
        { \
            OSG_NOTICE<<#PN<<" : type "<<type<<", "<<pi.getTypeName(type)<<std::endl; \
        } \
        else \
        { \
            OSG_NOTICE<<#PN<<" : failed to get type"<<std::endl; \
        } \
    }


    PRINT_TYPE(presentation.get(), Name)
    PRINT_TYPE(presentation.get(), Matrix)
    PRINT_TYPE(presentation.get(), DataVariance)
    PRINT_TYPE(presentation.get(), mystring)
    PRINT_TYPE(presentation.get(), mymatrix)

    osg::ref_ptr<osgGA::GUIEventAdapter> event = new osgGA::GUIEventAdapter;
    if (pi.getSupportedProperties(event.get(), properties, true))
    {
        OSG_NOTICE<<"Have supported properites found."<<std::endl;
        for(osgDB::ClassInterface::PropertyMap::iterator itr = properties.begin();
            itr != properties.end();
            ++itr)
        {
            OSG_NOTICE<<"   Property "<<itr->first<<", "<<pi.getTypeName(itr->second)<<std::endl;
        }
    }
    else
    {
        OSG_NOTICE<<"No supported properites found."<<std::endl;
    }
#endif

    osg::Vec3f pos(1.5,3.0,4.5);
    presentation->setProperty("position",pos);

    osg::Vec2f texcoord(0.5f,0.20f);
    presentation->setProperty("texcoord",texcoord);

    osg::ref_ptr<osg::ScriptEngine> luaScriptEngine = osgDB::readFile<osg::ScriptEngine>("ScriptEngine.lua");
    if (luaScriptEngine.valid())
    {
        presentation->getOrCreateUserDataContainer()->addUserObject(luaScriptEngine.get());
        std::string str;
        while (arguments.read("--lua", str))
        {
            osg::ref_ptr<osg::Script> script = osgDB::readScriptFile(str);
            if (script.valid())
            {
                presentation->addUpdateCallback(new osg::ScriptNodeCallback(script.get(),"update"));
            }
        }


        if (arguments.read("--test", str))
        {
            osg::ref_ptr<osg::Script> script = osgDB::readScriptFile(str);
            if (script.valid())
            {
                osg::Parameters inputParameters;
                osg::Parameters outputParameters;

                inputParameters.push_back(new osg::StringValueObject("string","my very first string input"));
                inputParameters.push_back(new osg::DoubleValueObject("double",1.234));
                inputParameters.push_back(new osg::MatrixfValueObject("matrix",osg::Matrixf()));

                osg::ref_ptr<osg::MatrixdValueObject> svo = new osg::MatrixdValueObject("return", osg::Matrixd());
                outputParameters.push_back(svo.get());

                if (luaScriptEngine->run(script.get(), "test", inputParameters, outputParameters))
                {
                    OSG_NOTICE<<"Successfully ran script : return value = "<<svo->getValue()<<std::endl;
                }
                else
                {
                    OSG_NOTICE<<"script run failed"<<std::endl;
                }
            }
        }
    }


    osg::ref_ptr<osg::Object> obj = pi.createObject("osgVolume::VolumeTile");
    if (obj.valid()) { OSG_NOTICE<<"obj created "<<obj->getCompoundClassName()<<std::endl; }
    else { OSG_NOTICE<<"obj creation failed "<<std::endl; }
    osgDB::ClassInterface::PropertyMap properties;

    if (pi.getSupportedProperties(obj.get(), properties, true))
    {
        OSG_NOTICE<<"Have supported properites found."<<std::endl;
        for(osgDB::ClassInterface::PropertyMap::iterator itr = properties.begin();
            itr != properties.end();
            ++itr)
        {
            OSG_NOTICE<<"   Property "<<itr->first<<", "<<pi.getTypeName(itr->second)<<std::endl;
        }
    }
    else
    {
        OSG_NOTICE<<"No supported properites found."<<std::endl;
    }

    //return 0;

    return viewer.run();

#endif
}
