#include "osg/Uniform"
#include "osg/io_utils"
#include "osg/Notify"

#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"

#include "Matrix.h"

using namespace osg;
using namespace osgDB;
using namespace std;

// forward declare functions to use later.
bool Uniform_readLocalData(Object& obj, Input& fr);
bool Uniform_writeLocalData(const Object& obj, Output& fw);

// register the read and write functions with the osgDB::Registry.
RegisterDotOsgWrapperProxy g_UniformProxy
(
    new osg::Uniform,
    "Uniform",
    "Object Uniform",
    &Uniform_readLocalData,
    &Uniform_writeLocalData
);


bool Uniform_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    Uniform& uniform = static_cast<Uniform&>(obj);


    if (fr.matchSequence("name %s"))
    {
        uniform.setName(fr[1].getStr());
        fr+=2;
        iteratorAdvanced = true;
    }

    if (fr[0].isWord())
    {
        uniform.setType( Uniform::getTypeId(fr[0].getStr()) );
        fr+=1;
        iteratorAdvanced = true;
    }
    
    switch( Uniform::getGlApiType(uniform.getType()) )
    {
        case(osg::Uniform::FLOAT):
        {
            float value;
            if (fr[0].getFloat(value)) 
            {
                uniform.set(value);
                fr+=1;
                iteratorAdvanced = true;
            }
            break;
        }
        case(osg::Uniform::FLOAT_VEC2):
        {
            osg::Vec2 value;
            if (fr[0].getFloat(value[0]) && fr[1].getFloat(value[1])) 
            {
                uniform.set(value);
                fr+=2;
                iteratorAdvanced = true;
            }
            break;
        }
        case(osg::Uniform::FLOAT_VEC3):
        {
            osg::Vec3 value;
            if (fr[0].getFloat(value[0]) && fr[1].getFloat(value[1]) && fr[2].getFloat(value[2])) 
            {
                uniform.set(value);
                fr+=3;
                iteratorAdvanced = true;
            }
            break;
        }
        case(osg::Uniform::FLOAT_VEC4):
        {
            osg::Vec4 value;
            if (fr[0].getFloat(value[0]) && fr[1].getFloat(value[1]) && fr[2].getFloat(value[2]) && fr[3].getFloat(value[3])) 
            {
                uniform.set(value);
                fr+=4;
                iteratorAdvanced = true;
            }
            break;
        }
        case(osg::Uniform::INT):
        {
            int value;
            if (fr[0].getInt(value)) 
            {
                uniform.set(value);
                fr+=1;
                iteratorAdvanced = true;
            }
            break;
        }
        case(osg::Uniform::INT_VEC2):
        {
            int value[2];
            if (fr[0].getInt(value[0]) && fr[1].getInt(value[1]))
            {
                uniform.set(value[0],value[1]);
                fr+=2;
                iteratorAdvanced = true;
            }
            break;
        }
        case(osg::Uniform::INT_VEC3):
        {
            int value[3];
            if (fr[0].getInt(value[0]) && fr[1].getInt(value[1]) && fr[2].getInt(value[2]))
            {
                uniform.set(value[0],value[1],value[2]);
                fr+=3;
                iteratorAdvanced = true;
            }
            break;
        }
        case(osg::Uniform::INT_VEC4):
        {
            int value[4];
            if (fr[0].getInt(value[0]) && fr[1].getInt(value[1]) && fr[2].getInt(value[2]) && fr[3].getInt(value[3]))
            {
                uniform.set(value[0],value[1],value[2],value[3]);
                fr+=4;
                iteratorAdvanced = true;
            }
            break;
        }
        case(osg::Uniform::FLOAT_MAT2):
        {
            osg::Matrix2 value;
            if (fr[0].getFloat(value[0]) && fr[1].getFloat(value[1]) &&
                fr[2].getFloat(value[2]) && fr[3].getFloat(value[3])) 
            {
                uniform.set(value);
                fr+=4;
                iteratorAdvanced = true;
            }
            break;
        }
        case(osg::Uniform::FLOAT_MAT3):
        {
            osg::Matrix3 value;
            if (fr[0].getFloat(value[0]) && fr[1].getFloat(value[1]) && fr[2].getFloat(value[2]) &&
                fr[3].getFloat(value[3]) && fr[4].getFloat(value[4]) && fr[5].getFloat(value[5]) &&
                fr[6].getFloat(value[6]) && fr[7].getFloat(value[7]) && fr[8].getFloat(value[8]))
            {
                uniform.set(value);
                fr+=9;
                iteratorAdvanced = true;
            }
            break;
        }
        case(osg::Uniform::FLOAT_MAT4):
        {
            Matrix value;
            if( readMatrix(value,fr) )
            {
                uniform.set(value);
               iteratorAdvanced = true;
            }
            break;
        }
        default:
        {
            break;
        }
    }

    return iteratorAdvanced;
}


bool Uniform_writeLocalData(const Object& obj,Output& fw)
{
    const Uniform& uniform = static_cast<const Uniform&>(obj);


    fw.indent() << "name "<< fw.wrapString(uniform.getName()) << std::endl;

    fw.indent() << Uniform::getTypename( uniform.getType() ) << " ";
    
    switch( Uniform::getGlApiType(uniform.getType()) )
    {
        case(osg::Uniform::FLOAT):
        {
            float value = 0.0f;
            uniform.get(value);
            fw << value;            
            break;
        }
        case(osg::Uniform::FLOAT_VEC2):
        {
            Vec2 value;
            uniform.get(value);
            fw << value;            
            break;
        }
        case(osg::Uniform::FLOAT_VEC3):
        {
            Vec3 value;
            uniform.get(value);
            fw << value;            
            break;
        }
        case(osg::Uniform::FLOAT_VEC4):
        {
            Vec4 value;
            uniform.get(value);
            fw << value;            
            break;
        }
        case(osg::Uniform::INT):
        {
            int value = 0;
            uniform.get(value);
            fw << value;            
            break;
        }
        case(osg::Uniform::INT_VEC2):
        {
            int value[2];
            uniform.get(value[0],value[1]);
            fw << value[0]<<" "<<value[1];
            break;
        }
        case(osg::Uniform::INT_VEC3):
        {
            int value[3];
            uniform.get(value[0],value[1],value[2]);
            fw << value[0]<<" "<<value[1]<<" "<<value[2];
            break;
        }
        case(osg::Uniform::INT_VEC4):
        {
            int value[4];
            uniform.get(value[0],value[1],value[2],value[3]);
            fw << value[0]<<" "<<value[1]<<" "<<value[2]<<" "<<value[3];
            break;
        }
        case(osg::Uniform::FLOAT_MAT2):
        {
            osg::Matrix2 value;
            uniform.get(value);
            fw << value[0]<<" "<<value[1]<<" "
                <<value[2]<<" "<<value[3];
            break;
        }
        case(osg::Uniform::FLOAT_MAT3):
        {
            osg::Matrix3 value;
            uniform.get(value);
            fw << value[0]<<" "<<value[1]<<" "<<value[2]<<" "
                <<value[3]<<" "<<value[4]<<" "<<value[5]<<" "
                <<value[6]<<" "<<value[7]<<" "<<value[8];
            break;
        }
        case(osg::Uniform::FLOAT_MAT4):
        {
            Matrix value;
            uniform.get(value);
            writeMatrix(value,fw);
            break;
        }
        default:
        {
            break;
        }
    }
    
    fw << std::endl;
    
    return true;
}
