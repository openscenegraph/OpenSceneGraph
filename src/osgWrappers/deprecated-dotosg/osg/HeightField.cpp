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
bool HeightField_readLocalData(Object& obj, Input& fr);
bool HeightField_writeLocalData(const Object& obj, Output& fw);

//register the read and write functions with the osgDB::Registry.
REGISTER_DOTOSGWRAPPER(HeightField)
(
    new osg::HeightField,
    "HeightField",
    "Object HeightField",
    &HeightField_readLocalData,
    &HeightField_writeLocalData,
    DotOsgWrapper::READ_AND_WRITE
);

//register the read and write functions with the osgDB::Registry.
REGISTER_DOTOSGWRAPPER(Grid)
(
    new osg::HeightField,
    "Grid",
    "Object HeightField",
    0,
    0,
    DotOsgWrapper::READ_AND_WRITE
);

bool HeightField_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    HeightField& heightfield = static_cast<HeightField&>(obj);

    if (fr.matchSequence("Origin %f %f %f"))
    {
        osg::Vec3 origin;
        fr[1].getFloat(origin.x());
        fr[2].getFloat(origin.y());
        fr[3].getFloat(origin.z());
        heightfield.setOrigin(origin);
        fr+=4;
    }

    if (fr.matchSequence("XInterval %f"))
    {
        float interval;
        fr[1].getFloat(interval);
        heightfield.setXInterval(interval);
        fr+=2;
        iteratorAdvanced = true;
    }

    if (fr.matchSequence("YInterval %f"))
    {
        float interval;
        fr[1].getFloat(interval);
        heightfield.setYInterval(interval);
        fr+=2;
        iteratorAdvanced = true;
    }

    if (fr.matchSequence("SkirtHeight %f"))
    {
        float height;
        fr[1].getFloat(height);
        heightfield.setSkirtHeight(height);
        fr+=2;
        iteratorAdvanced = true;
    }

    if (fr.matchSequence("BorderWidth %i"))
    {
        unsigned int width;
        fr[1].getUInt(width);
        heightfield.setBorderWidth(width);
        fr+=2;
        iteratorAdvanced = true;
    }

    if (fr.matchSequence("Rotation %f %f %f %f"))
    {
        osg::Quat rotation;
        fr[1].getFloat(rotation.x());
        fr[2].getFloat(rotation.y());
        fr[3].getFloat(rotation.z());
        fr[4].getFloat(rotation.w());
        heightfield.setRotation(rotation);
        fr+=5;
        iteratorAdvanced = true;
    }

    if (fr.matchSequence("NumColumnsAndRows %i %i"))
    {
        int numcolumns,numrows;
        fr[1].getInt(numcolumns);
        fr[2].getInt(numrows);
        heightfield.allocate(numcolumns,numrows);
        fr+=3;
        iteratorAdvanced = true;
    }

    if (fr.matchSequence("Heights {"))
    {

        int entry = fr[0].getNoNestedBrackets();

        fr += 2;

        float height;
        unsigned int row = 0;
        unsigned int column = 0;

        while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
        {
            if (fr.readSequence(height))
            {
                heightfield.setHeight(column,row,height);
                ++column;
                if (column>=heightfield.getNumColumns())
                {
                    column = 0;
                    ++row;
                }
            }
            else
            {
                ++fr;
            }
        }

        iteratorAdvanced = true;
        ++fr;

    }

    return iteratorAdvanced;
}

bool HeightField_writeLocalData(const Object& obj, Output& fw)
{
    const HeightField& heightfield = static_cast<const HeightField&>(obj);

    int prec = fw.precision();
    fw.precision(15);
    fw.indent()<<"Origin "<<heightfield.getOrigin().x()<<" "<<heightfield.getOrigin().y()<<" "<<heightfield.getOrigin().z()<<std::endl;
    fw.indent()<<"XInterval "<<heightfield.getXInterval()<<std::endl;
    fw.indent()<<"YInterval "<<heightfield.getYInterval()<<std::endl;
    fw.indent()<<"SkirtHeight "<<heightfield.getSkirtHeight()<<std::endl;
    fw.indent()<<"BorderWidth "<<heightfield.getBorderWidth()<<std::endl;
    fw.indent()<<"Rotation "<<heightfield.getRotation()<<std::endl;
    fw.precision(prec);

    fw.indent()<<"NumColumnsAndRows "<<heightfield.getNumColumns()<<" "<<heightfield.getNumRows()<<std::endl;

    fw.indent()<<"Heights"<<std::endl;

    ParameterOutput po(fw);
    po.begin();
    for(unsigned int row=0;row<heightfield.getNumRows();++row)
    {
        for(unsigned int column=0;column<heightfield.getNumColumns();++column)
        {
            po.write(heightfield.getHeight(column,row));
        }
        po.newLine();
    }
    po.end();

    return true;
}

