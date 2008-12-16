#include "osg/CoordinateSystemNode"

#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"

using namespace osg;
using namespace osgDB;

// forward declare functions to use later.
bool CoordinateSystemNode_readLocalData(Object& obj, Input& fr);
bool CoordinateSystemNode_writeLocalData(const Object& obj, Output& fw);

// register the read and write functions with the osgDB::Registry.
REGISTER_DOTOSGWRAPPER(CoordinateSystemNode)
(
    new osg::CoordinateSystemNode,
    "CoordinateSystemNode",
    "Object Node CoordinateSystemNode Group",
    &CoordinateSystemNode_readLocalData,
    &CoordinateSystemNode_writeLocalData
);

bool CoordinateSystemNode_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    CoordinateSystemNode& csn = static_cast<CoordinateSystemNode&>(obj);

    if (fr.matchSequence("Format %s"))
    {
        const char* str = fr[1].getStr();
        if (str) csn.setFormat(str);

        iteratorAdvanced = true;
        fr+=2;
    }

    if (fr.matchSequence("CoordinateSystem %s"))
    {
        const char* str = fr[1].getStr();
        if (str) csn.setCoordinateSystem(str);

        iteratorAdvanced = true;
        fr+=2;
    }

    static ref_ptr<EllipsoidModel> s_ellipsoidModel = new EllipsoidModel;
    
    EllipsoidModel* em = static_cast<EllipsoidModel*>(fr.readObjectOfType(*s_ellipsoidModel));
    if (em) csn.setEllipsoidModel(em);

    return iteratorAdvanced;
}


bool CoordinateSystemNode_writeLocalData(const Object& obj, Output& fw)
{
    const CoordinateSystemNode& csn = static_cast<const CoordinateSystemNode&>(obj);

    fw.indent()<<"Format "<<fw.wrapString(csn.getFormat())<<std::endl;
    fw.indent()<<"CoordinateSystem "<<fw.wrapString(csn.getCoordinateSystem())<<std::endl;
    
    if (csn.getEllipsoidModel()) 
    {
        fw.writeObject(*csn.getEllipsoidModel());
    }

    return true;
}
