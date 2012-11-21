
// Mike Weiblen 2006-05-14

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

// reuse from Geometry.cpp
bool Array_writeLocalData(const Array& array,Output& fw);
Array* Array_readLocalData(Input& fr);

// forward declare functions to use later.
bool Uniform_readLocalData(Object& obj, Input& fr);
bool Uniform_writeLocalData(const Object& obj, Output& fw);

// register the read and write functions with the osgDB::Registry.
REGISTER_DOTOSGWRAPPER(Uniform)
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

    if (fr[0].matchWord("type"))
    {
        // post-May 2006 format (OSG versions > 1.0)

        ++fr;

        if (fr.matchSequence("unsigned int"))
        {
            uniform.setType( Uniform::getTypeId( "unsigned int" ) );
            fr += 2;
        }
        else
        {
            uniform.setType( Uniform::getTypeId( fr[0].getStr() ) );
            ++fr;
        }

        unsigned int numElements;
        fr[0].getUInt(numElements);
        uniform.setNumElements( numElements );

        ++fr;
        iteratorAdvanced = true;

        Array* data = Array_readLocalData(fr);
        uniform.setArray( dynamic_cast<FloatArray*>(data) );
        uniform.setArray( dynamic_cast<DoubleArray*>(data) );
        uniform.setArray( dynamic_cast<IntArray*>(data) );
        uniform.setArray( dynamic_cast<UIntArray*>(data) );
    }
#if 1 //[
// Deprecated; for backwards compatibility only.
// when can we safely delete this code, I wonder...
    else
    {
        // pre-May 2006 format (OSG versions <= 1.0)
        uniform.setType( Uniform::getTypeId(fr[0].getStr()) );
        fr+=1;
        iteratorAdvanced = true;

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
                break;
        }
    }
#endif //]

    while (fr.matchSequence("UpdateCallback {"))
    {
        //int entry = fr[0].getNoNestedBrackets();
        fr += 2;
        Uniform::Callback* callback = fr.readObjectOfType<Uniform::Callback>();
        if (callback) {
            uniform.setUpdateCallback(callback);
        }
        iteratorAdvanced = true;
    }

    while (fr.matchSequence("EventCallback {"))
    {
        //int entry = fr[0].getNoNestedBrackets();
        fr += 2;
        Uniform::Callback* callback = fr.readObjectOfType<Uniform::Callback>();
        if (callback) {
            uniform.setEventCallback(callback);
        }
        iteratorAdvanced = true;
    }

    return iteratorAdvanced;
}

bool Uniform_writeLocalData(const Object& obj,Output& fw)
{
    const Uniform& uniform = static_cast<const Uniform&>(obj);

    // post-May 2006 format (OSG versions > 1.0)
    fw.indent() << "type "
        << Uniform::getTypename( uniform.getType() ) << " "
        << uniform.getNumElements() << " ";

    if( uniform.getFloatArray() ) Array_writeLocalData( *uniform.getFloatArray(), fw );
    if( uniform.getIntArray() )   Array_writeLocalData( *uniform.getIntArray(), fw );
    if( uniform.getUIntArray() )   Array_writeLocalData( *uniform.getUIntArray(), fw );

    if (uniform.getUpdateCallback())
    {
        fw.indent() << "UpdateCallback {" << std::endl;
        fw.moveIn();
        fw.writeObject(*uniform.getUpdateCallback());
        fw.moveOut();
        fw.indent() << "}" << std::endl;
    }

    if (uniform.getEventCallback())
    {
        fw.indent() << "EventCallback {" << std::endl;
        fw.moveIn();
        fw.writeObject(*uniform.getEventCallback());
        fw.moveOut();
        fw.indent() << "}" << std::endl;
    }

    return true;
}
