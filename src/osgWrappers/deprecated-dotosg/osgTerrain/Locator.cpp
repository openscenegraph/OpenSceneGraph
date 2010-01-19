#include <osgTerrain/TerrainTile>

#include <iostream>
#include <string>

#include <osg/Vec3>
#include <osg/Vec4>
#include <osg/io_utils>

#include <osgDB/ReadFile>
#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>
#include <osgDB/ParameterOutput>

bool Locator_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool Locator_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

osgDB::RegisterDotOsgWrapperProxy Locator_Proxy
(
    new osgTerrain::Locator,
    "Locator",
    "Object Locator",
    Locator_readLocalData,
    Locator_writeLocalData
);


bool Locator_readLocalData(osg::Object& obj, osgDB::Input &fr)
{
    osgTerrain::Locator& locator = static_cast<osgTerrain::Locator&>(obj);

    bool itrAdvanced = false;

    if (fr.matchSequence("Format %w") || fr.matchSequence("Format %s") )
    {
        locator.setFormat(fr[1].getStr());

        fr += 2;
        itrAdvanced = true;
    }

    if (fr.matchSequence("CoordinateSystemType %w"))
    {
        if (fr[1].matchWord("GEOCENTRIC")) locator.setCoordinateSystemType(osgTerrain::Locator::GEOCENTRIC);
        else if (fr[1].matchWord("GEOGRAPHIC")) locator.setCoordinateSystemType(osgTerrain::Locator::GEOGRAPHIC);
        else locator.setCoordinateSystemType(osgTerrain::Locator::PROJECTED);

        fr += 2;
        itrAdvanced = true;
    }

    if (fr.matchSequence("CoordinateSystem %w") || fr.matchSequence("CoordinateSystem %s") )
    {
        locator.setCoordinateSystem(fr[1].getStr());

        fr += 2;
        itrAdvanced = true;
    }

    if (fr.matchSequence("TransformScaledByResolution %w"))
    {
        locator.setTransformScaledByResolution(fr[1].matchWord("TRUE") || fr[1].matchWord("True") || fr[1].matchWord("true"));
        fr += 2;
        itrAdvanced = true;
    }

    if (fr.matchSequence("Transform {"))
    {
        int tansform_entry = fr[0].getNoNestedBrackets();

        fr += 2;

        int row=0;
        int col=0;
        double v;
        osg::Matrixd matrix;
        while (!fr.eof() && fr[0].getNoNestedBrackets()>tansform_entry)
        {
            if (fr[0].getFloat(v))
            {
                matrix(row,col)=v;
                ++col;
                if (col>=4)
                {
                    col = 0;
                    ++row;
                }
                ++fr;
            }
            else fr.advanceOverCurrentFieldOrBlock();
        }

        locator.setTransform(matrix);

        ++fr;
        itrAdvanced = true;
    }


    if (fr.matchSequence("Extents %f %f %f %f"))
    {
        double minX,minY,maxX,maxY;
        fr[1].getFloat(minX);
        fr[2].getFloat(minY);
        fr[3].getFloat(maxX);
        fr[4].getFloat(maxY);

        locator.setTransformAsExtents(minX, minY, maxX, maxY);

        fr += 5;
        itrAdvanced = true;
    }

    return itrAdvanced;
}

bool Locator_writeLocalData(const osg::Object& obj, osgDB::Output& fw)
{
    const osgTerrain::Locator& locator = static_cast<const osgTerrain::Locator&>(obj);

    if (!locator.getFormat().empty()) fw.indent()<<"Format "<<fw.wrapString(locator.getFormat())<<std::endl;
    if (!locator.getCoordinateSystem().empty()) fw.indent()<<"CoordinateSystem "<<fw.wrapString(locator.getCoordinateSystem())<<std::endl;

    fw.indent()<<"CoordinateSystemType ";
    switch(locator.getCoordinateSystemType())
    {
        case(osgTerrain::Locator::GEOCENTRIC):
        {        
            fw<<"GEOCENTRIC"<<std::endl;
            break;
        }
        case(osgTerrain::Locator::GEOGRAPHIC):
        {        
            fw<<"GEOGRAPHIC"<<std::endl;
            break;
        }
        case(osgTerrain::Locator::PROJECTED):
        {        
            fw<<"PROJECTED"<<std::endl;
            break;
        }
    }   
    
    fw.indent()<<"TransformScaledByResolution " << (locator.getTransformScaledByResolution() ? "TRUE":"FALSE") <<std::endl;

    const osg::Matrixd& matrix = locator.getTransform();
    fw.indent() << "Transform {" << std::endl;
    fw.moveIn();
    fw.indent() << matrix(0,0) << " " << matrix(0,1) << " " << matrix(0,2) << " " << matrix(0,3) << std::endl;
    fw.indent() << matrix(1,0) << " " << matrix(1,1) << " " << matrix(1,2) << " " << matrix(1,3) << std::endl;
    fw.indent() << matrix(2,0) << " " << matrix(2,1) << " " << matrix(2,2) << " " << matrix(2,3) << std::endl;
    fw.indent() << matrix(3,0) << " " << matrix(3,1) << " " << matrix(3,2) << " " << matrix(3,3) << std::endl;
    fw.moveOut();
    fw.indent() << "}"<< std::endl;

    return true;
}
