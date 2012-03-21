#include <osg/Shape>
#include <osg/Notify>
#include <osg/io_utils>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/ParameterOutput>

using namespace osg;
using namespace osgDB;


//////////////////////////////////////////////////////////////////////////////
// forward declare functions to use later.
bool CompositeShape_readLocalData(Object& obj, Input& fr);
bool CompositeShape_writeLocalData(const Object& obj, Output& fw);

//register the read and write functions with the osgDB::Registry.
REGISTER_DOTOSGWRAPPER(CompositeShape)
(
    new osg::CompositeShape,
    "CompositeShape",
    "Object CompositeShape",
    &CompositeShape_readLocalData,
    &CompositeShape_writeLocalData,
    DotOsgWrapper::READ_AND_WRITE
);

bool CompositeShape_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    CompositeShape& composite = static_cast<CompositeShape&>(obj);

    ref_ptr<Object> readObject;
    if (fr[0].matchWord("Shape"))
    {
        readObject = fr.readObject();
        if (readObject.valid())
        {
            osg::Shape* shape = dynamic_cast<osg::Shape*>(readObject.get());
            if (shape) composite.setShape(shape);
            else notify(WARN)<<"Warning:: "<<readObject->className()<<" loaded but cannot not be attached to Drawable."<<std::endl;
            iteratorAdvanced = true;
        }
    }

    while((readObject=fr.readObjectOfType(type_wrapper<osg::Shape>())).valid())
    {
        osg::Shape* shape = static_cast<osg::Shape*>(readObject.get());
        composite.addChild(shape);
        iteratorAdvanced = true;
    }

    return iteratorAdvanced;
}

bool CompositeShape_writeLocalData(const Object& obj, Output& fw)
{
    const CompositeShape& composite = static_cast<const CompositeShape&>(obj);

    if (composite.getShape())
    {
        fw.indent() << "Shape ";
        fw.writeObject(*composite.getShape());
    }

    for(unsigned int i=0;i<composite.getNumChildren();++i)
    {
        fw.writeObject(*composite.getChild(i));
    }

    return true;
}

