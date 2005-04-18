#include "osg/Uniform"
#include "osg/io_utils"
#include "osg/Notify"

#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"

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

    // TODO read uniform value based on type

    if (fr.matchSequence("type %w"))
    {
	uniform.setType( Uniform::getTypeId(fr[1].getStr()) );
	fr+=2;
	iteratorAdvanced = true;
    }
    
    switch(uniform.getType())
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
        case(osg::Uniform::BOOL):
        {
            int value;
            if (fr[0].getInt(value)) 
            {
                uniform.set(value!=0 ? true:false);
	        fr+=1;
	        iteratorAdvanced = true;
            }
            break;
        }
        case(osg::Uniform::BOOL_VEC2):
        {
            int value[2];
            if (fr[0].getInt(value[0]) && fr[1].getInt(value[1]))
            {
                uniform.set(value[0]!=0 ? true:false, value[1]!=0 ? true:false);
	        fr+=2;
	        iteratorAdvanced = true;
            }
            break;
        }
        case(osg::Uniform::BOOL_VEC3):
        {
            int value[3];
            if (fr[0].getInt(value[0]) && fr[1].getInt(value[1]) && fr[2].getInt(value[2]))
            {
                uniform.set(value[0]!=0 ? true:false, value[1]!=0 ? true:false, value[2]!=0 ? true:false);
	        fr+=3;
	        iteratorAdvanced = true;
            }
            break;
        }
        case(osg::Uniform::BOOL_VEC4):
        {
            int value[4];
            if (fr[0].getInt(value[0]) && fr[1].getInt(value[1]) && fr[2].getInt(value[2]) && fr[3].getInt(value[3]))
            {
                uniform.set(value[0]!=0 ? true:false, value[1]!=0 ? true:false, value[2]!=0 ? true:false, value[3]!=0 ? true:false);
	        fr+=4;
	        iteratorAdvanced = true;
            }
            break;
        }
        case(osg::Uniform::FLOAT_MAT2):
        {
            osg::notify(osg::WARN)<<"Warning : type not supported for reading."<<std::endl;
            break;
        }
        case(osg::Uniform::FLOAT_MAT3):
        {
            osg::notify(osg::WARN)<<"Warning : type not supported for reading."<<std::endl;
            break;
        }
        case(osg::Uniform::FLOAT_MAT4):
        {
            osg::notify(osg::WARN)<<"Warning : type not supported for reading."<<std::endl;
            break;
        }
        case(osg::Uniform::SAMPLER_1D):
        {
            osg::notify(osg::WARN)<<"Warning : type not supported for reading."<<std::endl;
            break;
        }
        case(osg::Uniform::SAMPLER_2D):
        {
            osg::notify(osg::WARN)<<"Warning : type not supported for reading."<<std::endl;
            break;
        }
        case(osg::Uniform::SAMPLER_3D):
        {
            osg::notify(osg::WARN)<<"Warning : type not supported for reading."<<std::endl;
            break;
        }
        case(osg::Uniform::SAMPLER_CUBE):
        {
             osg::notify(osg::WARN)<<"Warning : type not supported for reading."<<std::endl;
           break;
        }
        case(osg::Uniform::SAMPLER_1D_SHADOW):
        {
            osg::notify(osg::WARN)<<"Warning : type not supported for reading."<<std::endl;
            break;
        }
        case(osg::Uniform::SAMPLER_2D_SHADOW):
        {
            osg::notify(osg::WARN)<<"Warning : type not supported for reading."<<std::endl;
            break;
        }
    }

    return iteratorAdvanced;
}


bool Uniform_writeLocalData(const Object& obj,Output& fw)
{
    const Uniform& uniform = static_cast<const Uniform&>(obj);


    fw.indent() << "name "<< fw.wrapString(uniform.getName()) << std::endl;

    fw.indent() << "type " << Uniform::getTypename( uniform.getType() ) << " ";
    
    switch(uniform.getType())
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
        case(osg::Uniform::BOOL):
        {
            bool value = 0;
            uniform.get(value);
            fw << value;            
            break;
        }
        case(osg::Uniform::BOOL_VEC2):
        {
            bool value[2];
            uniform.get(value[0],value[1]);
            fw << value[0]<<" "<<value[1];
            break;
        }
        case(osg::Uniform::BOOL_VEC3):
        {
            bool value[3];
            uniform.get(value[0],value[1],value[2]);
            fw << value[0]<<" "<<value[1]<<" "<<value[2];
            break;
        }
        case(osg::Uniform::BOOL_VEC4):
        {
            bool value[4];
            uniform.get(value[0],value[1],value[2],value[3]);
            fw << value[0]<<" "<<value[1]<<" "<<value[2]<<" "<<value[3];
            break;
        }
        case(osg::Uniform::FLOAT_MAT2):
        {
            osg::notify(osg::WARN)<<"Warning : type not supported for writing."<<std::endl;
            break;
        }
        case(osg::Uniform::FLOAT_MAT3):
        {
            osg::notify(osg::WARN)<<"Warning : type not supported for writing."<<std::endl;
            break;
        }
        case(osg::Uniform::FLOAT_MAT4):
        {
            osg::notify(osg::WARN)<<"Warning : type not supported for writing."<<std::endl;
            break;
        }
        case(osg::Uniform::SAMPLER_1D):
        {
            osg::notify(osg::WARN)<<"Warning : type not supported for writing."<<std::endl;
            break;
        }
        case(osg::Uniform::SAMPLER_2D):
        {
            osg::notify(osg::WARN)<<"Warning : type not supported for writing."<<std::endl;
            break;
        }
        case(osg::Uniform::SAMPLER_3D):
        {
            osg::notify(osg::WARN)<<"Warning : type not supported for writing."<<std::endl;
            break;
        }
        case(osg::Uniform::SAMPLER_CUBE):
        {
            osg::notify(osg::WARN)<<"Warning : type not supported for writing."<<std::endl;
            break;
        }
        case(osg::Uniform::SAMPLER_1D_SHADOW):
        {
            osg::notify(osg::WARN)<<"Warning : type not supported for writing."<<std::endl;
            break;
        }
        case(osg::Uniform::SAMPLER_2D_SHADOW):
        {
            osg::notify(osg::WARN)<<"Warning : type not supported for writing."<<std::endl;
            break;
        }
    }
    
    fw << std::endl;
    
    return true;
}
