#include <osg/ShapeDrawable>
#include <osg/Notify>
#include <osg/io_utils>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

using namespace osg;
using namespace osgDB;

// forward declare functions to use later.
bool ShapeDrawable_readLocalData(Object& obj, Input& fr);
bool ShapeDrawable_writeLocalData(const Object& obj, Output& fw);

REGISTER_DOTOSGWRAPPER(ShapeDrawable)
(
    new osg::ShapeDrawable,
    "ShapeDrawable",
    "Object Drawable ShapeDrawable",
    &ShapeDrawable_readLocalData,
    &ShapeDrawable_writeLocalData,
    DotOsgWrapper::READ_AND_WRITE
);

bool ShapeDrawable_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    ShapeDrawable& geom = static_cast<ShapeDrawable&>(obj);

    if (fr.matchSequence("color %f %f %f %f"))
    {
        osg::Vec4 color;
        fr[1].getFloat(color[0]);
        fr[2].getFloat(color[1]);
        fr[3].getFloat(color[2]);
        fr[4].getFloat(color[3]);
    
        geom.setColor(color);
        
        fr+=5;
        iteratorAdvanced = true;
    }

    ref_ptr<Object> readObject = fr.readObjectOfType(type_wrapper<TessellationHints>());
    if (readObject.valid()) {
        TessellationHints* hints = static_cast<TessellationHints*>(readObject.get());
        geom.setTessellationHints(hints);
        iteratorAdvanced = true;
    }

    return iteratorAdvanced;
}

bool ShapeDrawable_writeLocalData(const Object& obj, Output& fw)
{
    const ShapeDrawable& geom = static_cast<const ShapeDrawable&>(obj);

    fw.indent() << "color " << geom.getColor() << std::endl;

    const TessellationHints* hints = geom.getTessellationHints();
    if (hints)
        fw.writeObject(*hints);

    return true;
}
