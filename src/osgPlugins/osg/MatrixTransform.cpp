#include "osg/MatrixTransform"

#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"

using namespace osg;
using namespace osgDB;

// forward declare functions to use later.
bool MatrixTransform_readLocalData(Object& obj, Input& fr);
bool MatrixTransform_writeLocalData(const Object& obj, Output& fw);

// register the read and write functions with the osgDB::Registry.
RegisterDotOsgWrapperProxy g_MatrixTransformProxy
(
    new osg::MatrixTransform,
    "MatrixTransform",
    "Object Node Transform MatrixTransform Group",
    &MatrixTransform_readLocalData,
    &MatrixTransform_writeLocalData,
    DotOsgWrapper::READ_AND_WRITE
);

// register old style 'DCS' read and write functions with the osgDB::Registry.
RegisterDotOsgWrapperProxy g_DCSProxy
(
    new osg::MatrixTransform,
    "DCS",
    "Object Node Group DCS",
    &MatrixTransform_readLocalData,
    NULL,
    DotOsgWrapper::READ_ONLY
);

bool MatrixTransform_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    MatrixTransform& transform = static_cast<MatrixTransform&>(obj);

    if (fr[0].matchWord("Type"))
    {
        if (fr[1].matchWord("DYNAMIC"))
        {
            transform.setDataVariance(osg::Object::DYNAMIC);
            fr +=2 ;
            iteratorAdvanced = true;
        }
        else if (fr[1].matchWord("STATIC"))
        {
            transform.setDataVariance(osg::Object::STATIC);
            fr +=2 ;
            iteratorAdvanced = true;
        }
        
    }    

    static Matrix s_matrix;
    
    if (Matrix* tmpMatrix = static_cast<Matrix*>(fr.readObjectOfType(s_matrix)))
    {

        transform.setMatrix(*tmpMatrix);

        delete tmpMatrix;

        iteratorAdvanced = true;
    }

    if (fr[0].matchWord("AnimationPath"))
    {
        static osg::ref_ptr<osg::AnimationPath> prototype = new osg::AnimationPath; 
        osg::ref_ptr<osg::Object> object = fr.readObjectOfType(*prototype);
        osg::AnimationPath* path = dynamic_cast<osg::AnimationPath*>(object.get());
        if (path)
        {
            transform.setAnimationPath(path);
        }
        else
        {
            osg::Node* node = dynamic_cast<osg::Node*>(object.get());
            if (node) transform.addChild(node);
        }
    }

    return iteratorAdvanced;
}


bool MatrixTransform_writeLocalData(const Object& obj, Output& fw)
{
    const MatrixTransform& transform = static_cast<const MatrixTransform&>(obj);

    fw.writeObject(transform.getMatrix());

    if (transform.getAnimationPath())
    {
        fw.writeObject(*transform.getAnimationPath());
    }

    return true;
}
