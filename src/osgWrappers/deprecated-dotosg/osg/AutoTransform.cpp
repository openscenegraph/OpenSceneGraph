#include <osg/AutoTransform>
#include <osg/io_utils>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

using namespace osg;
using namespace osgDB;

// forward declare functions to use later.
bool AutoTransform_readLocalData(Object& obj, Input& fr);
bool AutoTransform_writeLocalData(const Object& obj, Output& fw);

// register the read and write functions with the osgDB::Registry.
REGISTER_DOTOSGWRAPPER(AutoTransform)
(
    new osg::AutoTransform,
    "AutoTransform",
    "Object Node Transform AutoTransform Group",
    &AutoTransform_readLocalData,
    &AutoTransform_writeLocalData,
    DotOsgWrapper::READ_AND_WRITE
);

bool AutoTransform_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    AutoTransform& transform = static_cast<AutoTransform&>(obj);

    if (fr.matchSequence("position %f %f %f"))
    {
        osg::Vec3 pos;
        fr[1].getFloat(pos[0]);
        fr[2].getFloat(pos[1]);
        fr[3].getFloat(pos[2]);

        transform.setPosition(pos);

        fr += 4;
        iteratorAdvanced = true;
    }

    if (fr.matchSequence("rotation %f %f %f %f"))
    {
        osg::Quat att;
        fr[1].getFloat(att[0]);
        fr[2].getFloat(att[1]);
        fr[3].getFloat(att[2]);
        fr[4].getFloat(att[3]);

        transform.setRotation(att);

        fr += 5;
        iteratorAdvanced = true;
    }

    if (fr.matchSequence("scale %f %f %f"))
    {
        osg::Vec3 scale;
        fr[1].getFloat(scale[0]);
        fr[2].getFloat(scale[1]);
        fr[3].getFloat(scale[2]);

        transform.setScale(scale);

        fr += 4;
        iteratorAdvanced = true;
    }

    if (fr.matchSequence("minimumScale %f"))
    {
        float scale;
        fr[1].getFloat(scale);

        transform.setMinimumScale(scale);

        fr += 2;
        iteratorAdvanced = true;
    }

    if (fr.matchSequence("maximumScale %f"))
    {
        float scale;
        fr[1].getFloat(scale);

        transform.setMaximumScale(scale);

        fr += 2;
        iteratorAdvanced = true;
    }

    if (fr.matchSequence("pivotPoint %f %f %f"))
    {
        osg::Vec3 pivot;
        fr[1].getFloat(pivot[0]);
        fr[2].getFloat(pivot[1]);
        fr[3].getFloat(pivot[2]);

        transform.setPivotPoint(pivot);

        fr += 4;
        iteratorAdvanced = true;
    }

    if (fr.matchSequence("autoUpdateEyeMovementTolerance %f"))
    {
        float f;
        fr[1].getFloat(f);
        transform.setAutoUpdateEyeMovementTolerance(f);
        fr += 2;
        iteratorAdvanced = true;
    }

    if (fr.matchSequence("autoRotateToScreen %w"))
    {
        std::string w(fr[1].getStr());
        transform.setAutoRotateMode((w == "TRUE") ? osg::AutoTransform::ROTATE_TO_SCREEN : osg::AutoTransform::NO_ROTATION);
        fr += 2;
        iteratorAdvanced = true;
    }

    if (fr.matchSequence("autoRotateMode %w"))
    {
        std::string w(fr[1].getStr());
                if (w=="ROTATE_TO_SCREEN") transform.setAutoRotateMode(osg::AutoTransform::ROTATE_TO_SCREEN);
                else if (w=="ROTATE_TO_CAMERA") transform.setAutoRotateMode(osg::AutoTransform::ROTATE_TO_CAMERA);
                else if (w=="NO_ROTATION") transform.setAutoRotateMode(osg::AutoTransform::NO_ROTATION);

        fr += 2;
        iteratorAdvanced = true;
    }

    if (fr.matchSequence("autoScaleToScreen %w"))
    {
        std::string w(fr[1].getStr());
        transform.setAutoScaleToScreen(w == "TRUE");
        fr += 2;
        iteratorAdvanced = true;
    }

    if (fr.matchSequence("autoScaleTransistionWidthRatio %f") ||
        fr.matchSequence("autoScaleTransitionWidthRatio %f"))
    {
        float ratio;
        fr[1].getFloat(ratio);

        transform.setAutoScaleTransitionWidthRatio(ratio);

        fr += 2;
        iteratorAdvanced = true;
    }


    return iteratorAdvanced;
}


bool AutoTransform_writeLocalData(const Object& obj, Output& fw)
{
    const AutoTransform& transform = static_cast<const AutoTransform&>(obj);

    fw.indent()<<"position "<<transform.getPosition()<<std::endl;
    fw.indent()<<"rotation "<<transform.getRotation()<<std::endl;
    fw.indent()<<"scale "<<transform.getScale()<<std::endl;

    if (transform.getMinimumScale()>0.0) fw.indent()<<"minimumScale "<<transform.getMinimumScale()<<std::endl;
    if (transform.getMaximumScale()<FLT_MAX) fw.indent()<<"maximumScale "<<transform.getMaximumScale()<<std::endl;


    fw.indent()<<"pivotPoint "<<transform.getPivotPoint()<<std::endl;
    fw.indent()<<"autoUpdateEyeMovementTolerance "<<transform.getAutoUpdateEyeMovementTolerance()<<std::endl;
    fw.indent()<<"autoRotateMode ";
    switch(transform.getAutoRotateMode())
    {
      case(osg::AutoTransform::ROTATE_TO_SCREEN): fw<<"ROTATE_TO_SCREEN"<<std::endl; break;
      case(osg::AutoTransform::ROTATE_TO_CAMERA): fw<<"ROTATE_TO_CAMERA"<<std::endl; break;
      case(osg::AutoTransform::NO_ROTATION):
      default: fw<<"NO_ROTATION"<<std::endl; break;
    }

    fw.indent()<<"autoScaleToScreen "<<(transform.getAutoScaleToScreen()?"TRUE":"FALSE")<<std::endl;

    if (transform.getAutoScaleTransitionWidthRatio()!=0.25)
    {
            fw.indent()<<"autoScaleTransitionWidthRatio "<<transform.getAutoScaleTransitionWidthRatio()<<std::endl;
    }

    return true;
}
