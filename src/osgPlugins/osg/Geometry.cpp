#include <osg/Geometry>
#include <osg/Notify>
#include <osg/io_utils>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/ParameterOutput>

#include <string.h>

using namespace osg;
using namespace osgDB;

// forward declare functions to use later.
bool Geometry_readLocalData(Object& obj, Input& fr);
bool Geometry_writeLocalData(const Object& obj, Output& fw);

bool Geometry_matchBindingTypeStr(const char* str,Geometry::AttributeBinding& mode);
const char* Geometry_getBindingTypeStr(Geometry::AttributeBinding mode);

bool Geometry_matchPrimitiveModeStr(const char* str,GLenum& mode);
const char* Geometry_getPrimitiveModeStr(GLenum mode);

Array* Array_readLocalData(Input& fr);

bool Primitive_readLocalData(Input& fr,osg::Geometry& geom);

//register the read and write functions with the osgDB::Registry.
REGISTER_DOTOSGWRAPPER(Geometry)
(
    new osg::Geometry,
    "Geometry",
    "Object Drawable Geometry",
    &Geometry_readLocalData,
    &Geometry_writeLocalData,
    DotOsgWrapper::READ_AND_WRITE
);

bool Geometry_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    Geometry& geom = static_cast<Geometry&>(obj);

    if (fr.matchSequence("Primitives %i {") || fr.matchSequence("PrimitiveSets %i {") )
    {
        int entry = fr[1].getNoNestedBrackets();

        int capacity;
        fr[1].getInt(capacity);
        
        Geometry::PrimitiveSetList& primitives = geom.getPrimitiveSetList();
        if (capacity>0) primitives.reserve(capacity);
        

        fr += 3;
        

        while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
        {
            if (!Primitive_readLocalData(fr,geom)) fr.advanceOverCurrentFieldOrBlock();
        }

        ++fr;
        
        iteratorAdvanced = true;

    }

    if (fr[0].matchWord("VertexArray"))
    {
        if (fr.matchSequence("VertexArray %i {"))
        {

            int entry = fr[0].getNoNestedBrackets();

            int capacity;
            fr[1].getInt(capacity);

            Vec3Array* vertices = new Vec3Array;
            vertices->reserve(capacity);

            fr += 3;

            while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
            {
                Vec3 v;
                if (fr[0].getFloat(v.x()) && fr[1].getFloat(v.y()) && fr[2].getFloat(v.z()))
                {
                    fr += 3;
                    vertices->push_back(v);
                }
                else
                {
                    ++fr;
                }
            }

            geom.setVertexArray(vertices);

            iteratorAdvanced = true;
            ++fr;

        }
        else
        {
            // post 0.9.3 releases.
            ++fr;
            Array* vertices = Array_readLocalData(fr);
            if (vertices)
            {
                geom.setVertexArray(vertices);
            }
            iteratorAdvanced = true;
        }
    }
    
    if (fr[0].matchWord("VertexIndices"))
    {
        ++fr;

        IndexArray* indices = dynamic_cast<IndexArray*>(Array_readLocalData(fr));
        if (indices)
        {
            geom.setVertexIndices(indices);
        }

        iteratorAdvanced = true;
    }


    Geometry::AttributeBinding normalBinding=Geometry::BIND_OFF;
    if (fr[0].matchWord("NormalBinding") && Geometry_matchBindingTypeStr(fr[1].getStr(),normalBinding))
    {
        geom.setNormalBinding(normalBinding);
        fr+=2;
        iteratorAdvanced = true;
    }

    if (fr[0].matchWord("NormalArray"))
    {
        if (fr.matchSequence("NormalArray %i {"))
        {
            // pre 0.9.3 releases..
            int entry = fr[0].getNoNestedBrackets();

            int capacity;
            fr[1].getInt(capacity);

            Vec3Array* normals = new Vec3Array;
            normals->reserve(capacity);

            fr += 3;

            while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
            {
                Vec3 v;
                if (fr[0].getFloat(v.x()) && fr[1].getFloat(v.y()) && fr[2].getFloat(v.z()))
                {
                    fr += 3;
                    normals->push_back(v);
                }
                else
                {
                    ++fr;
                }
            }

            geom.setNormalArray(normals);

            iteratorAdvanced = true;
            ++fr;
        }
        else
        {
            // post 0.9.3 releases.
            ++fr;
            Array* normals = Array_readLocalData(fr);
            if (normals)
            {
                geom.setNormalArray(normals);
            }
            iteratorAdvanced = true;
        }
    }
    if (fr[0].matchWord("NormalIndices"))
    {
        ++fr;
        IndexArray* indices = dynamic_cast<IndexArray*>(Array_readLocalData(fr));
        if (indices)
        {
            geom.setNormalIndices(indices);
        }
        iteratorAdvanced = true;
    }

    Geometry::AttributeBinding colorBinding=Geometry::BIND_OFF;
    if (fr[0].matchWord("ColorBinding") && Geometry_matchBindingTypeStr(fr[1].getStr(),colorBinding))
    {
        geom.setColorBinding(colorBinding);
        fr+=2;
        iteratorAdvanced = true;
    }

    if (fr[0].matchWord("ColorArray"))
    {
        ++fr;
        Array* colors = Array_readLocalData(fr);
        if (colors)
        {
            geom.setColorArray(colors);
        }
        iteratorAdvanced = true;
    }

    if (fr[0].matchWord("ColorIndices"))
    {
        ++fr;
        IndexArray* indices = dynamic_cast<IndexArray*>(Array_readLocalData(fr));
        if (indices)
        {
            geom.setColorIndices(indices);
        }
        iteratorAdvanced = true;
    }


    Geometry::AttributeBinding secondaryColorBinding=Geometry::BIND_OFF;
    if (fr[0].matchWord("SecondaryColorBinding") && Geometry_matchBindingTypeStr(fr[1].getStr(),secondaryColorBinding))
    {
        geom.setSecondaryColorBinding(secondaryColorBinding);
        fr+=2;
        iteratorAdvanced = true;
    }

    if (fr[0].matchWord("SecondaryColorArray"))
    {
        ++fr;
        Array* colors = Array_readLocalData(fr);
        if (colors)
        {
            geom.setSecondaryColorArray(colors);
        }
        iteratorAdvanced = true;
    }

    if (fr[0].matchWord("SecondaryColorIndices"))
    {
        ++fr;
        IndexArray* indices = dynamic_cast<IndexArray*>(Array_readLocalData(fr));
        if (indices)
        {
            geom.setSecondaryColorIndices(indices);
        }
        iteratorAdvanced = true;
    }


    Geometry::AttributeBinding fogCoordBinding=Geometry::BIND_OFF;
    if (fr[0].matchWord("FogCoordBinding") && Geometry_matchBindingTypeStr(fr[1].getStr(),fogCoordBinding))
    {
        geom.setFogCoordBinding(fogCoordBinding);
        fr+=2;
        iteratorAdvanced = true;
    }

    if (fr[0].matchWord("FogCoordArray"))
    {
        ++fr;
        Array* fogcoords = Array_readLocalData(fr);
        if (fogcoords)
        {
            geom.setFogCoordArray(fogcoords);
        }
        iteratorAdvanced = true;
    }

    if (fr[0].matchWord("FogCoordIndices"))
    {
        ++fr;
        IndexArray* indices = dynamic_cast<IndexArray*>(Array_readLocalData(fr));
        if (indices)
        {
            geom.setFogCoordIndices(indices);
        }
        iteratorAdvanced = true;
    }


    if (fr.matchSequence("TexCoordArray %i"))
    {
        int unit=0;
        fr[1].getInt(unit);
    
        fr+=2;
        Array* texcoords = Array_readLocalData(fr);
        if (texcoords)
        {
            geom.setTexCoordArray(unit,texcoords);
        }
        iteratorAdvanced = true;
        
    }

    if (fr.matchSequence("TexCoordIndices %i"))
    {
        int unit=0;
        fr[1].getInt(unit);
    
        fr+=2;
        IndexArray* indices = dynamic_cast<IndexArray*>(Array_readLocalData(fr));
        if (indices)
        {
            geom.setTexCoordIndices(unit,indices);
        }
        iteratorAdvanced = true;
    }

    Geometry::AttributeBinding vertexAttribBinding=Geometry::BIND_OFF;
    if (fr.matchSequence("VertexAttribBinding %i %w") && Geometry_matchBindingTypeStr(fr[2].getStr(),vertexAttribBinding))
    {
        int unit=0;
        fr[1].getInt(unit);
        geom.setVertexAttribBinding(unit,vertexAttribBinding);
        fr+=3;
        iteratorAdvanced = true;
    }

    if (fr.matchSequence("VertexAttribNormalize %i %w"))
    {
        int unit=0;
        fr[1].getInt(unit);
        
        if (fr[2].matchString("TRUE"))        
            geom.setVertexAttribNormalize(unit,GL_TRUE);
        else
            geom.setVertexAttribNormalize(unit,GL_FALSE);
        
        fr+=3;
        iteratorAdvanced = true;
    }

    if (fr.matchSequence("VertexAttribArray %i"))
    {
        int unit=0;
        fr[1].getInt(unit);
    
        fr+=2;
        Array* vertexattrib = Array_readLocalData(fr);
        if (vertexattrib)
        {
            geom.setVertexAttribArray(unit,vertexattrib);
        }
        iteratorAdvanced = true;
        
    }

    if (fr.matchSequence("VertexAttribIndices %i"))
    {
        int unit=0;
        fr[1].getInt(unit);
    
        fr+=2;
        IndexArray* indices = dynamic_cast<IndexArray*>(Array_readLocalData(fr));
        if (indices)
        {
            geom.setVertexAttribIndices(unit,indices);
        }
        iteratorAdvanced = true;
    }

    return iteratorAdvanced;
}


Array* Array_readLocalData(Input& fr)
{
    if (fr[0].matchWord("Use"))
    {
        if (fr[1].isString())
        {
            Object* obj = fr.getObjectForUniqueID(fr[1].getStr());
            if (obj)
            {
                fr+=2;
                return dynamic_cast<Array*>(obj);
            }
        }

        osg::notify(osg::WARN)<<"Warning: invalid uniqueID found in file."<<std::endl;
        return NULL;
    }

    std::string uniqueID;
    if (fr[0].matchWord("UniqueID") && fr[1].isString())
    {
        uniqueID = fr[1].getStr();
        fr += 2;
    }


    int entry = fr[0].getNoNestedBrackets();

    const char* arrayName = fr[0].getStr();

    unsigned int capacity = 0;
    fr[1].getUInt(capacity);
    ++fr;

    fr += 2;


    Array* return_array = 0;

    if (strcmp(arrayName,"ByteArray")==0)
    {
        ByteArray* array = new ByteArray;
        array->reserve(capacity);
        while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
        {
            int int_value;
            if (fr[0].getInt(int_value))
            {
                ++fr;
                array->push_back(int_value);
            }
            else ++fr;
        }
        ++fr;

        return_array = array;
    }
    else if (strcmp(arrayName,"ShortArray")==0)
    {
        ShortArray* array = new ShortArray;
        array->reserve(capacity);
        while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
        {
            int int_value;
            if (fr[0].getInt(int_value))
            {
                ++fr;
                array->push_back(int_value);
            }
            else ++fr;
        }
        ++fr;
        return_array = array;
    }
    else if (strcmp(arrayName,"IntArray")==0)
    {
        IntArray* array = new IntArray;
        array->reserve(capacity);
        while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
        {
            int int_value;
            if (fr[0].getInt(int_value))
            {
                ++fr;
                array->push_back(int_value);
            }
            else ++fr;
        }
        ++fr;
        return_array = array;
    }
    else if (strcmp(arrayName,"UByteArray")==0)
    {
        UByteArray* array = new UByteArray;
        array->reserve(capacity);
        while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
        {
            unsigned int uint_value;
            if (fr[0].getUInt(uint_value))
            {
                ++fr;
                array->push_back(uint_value);
            }
            else ++fr;
        }
        ++fr;
        return_array = array;
    }
    else if (strcmp(arrayName,"UShortArray")==0)
    {
        UShortArray* array = new UShortArray;
        array->reserve(capacity);
        while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
        {
            unsigned int uint_value;
            if (fr[0].getUInt(uint_value))
            {
                ++fr;
                array->push_back(uint_value);
            }
            else ++fr;
        }
        ++fr;
        return_array = array;
    }
    else if (strcmp(arrayName,"UIntArray")==0)
    {
        UIntArray* array = new UIntArray;
        array->reserve(capacity);
        while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
        {
            unsigned int uint_value;
            if (fr[0].getUInt(uint_value))
            {
                ++fr;
                array->push_back(uint_value);
            }
            else ++fr;
        }
        ++fr;
        return_array = array;
    }
    else if (strcmp(arrayName,"UVec4bArray")==0 || strcmp(arrayName,"Vec4ubArray")==0)
    {
        Vec4ubArray* array = new Vec4ubArray;
        array->reserve(capacity);
        while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
        {
            unsigned int r,g,b,a;
            if (fr[0].getUInt(r) &&
                fr[1].getUInt(g) &&
                fr[2].getUInt(b) &&
                fr[3].getUInt(a))
            {
                fr+=4;
                array->push_back(osg::Vec4ub(r,g,b,a));
            }
            else ++fr;
        }
        ++fr;
        return_array = array;
    }
    else if (strcmp(arrayName,"FloatArray")==0)
    {
        FloatArray* array = new FloatArray;
        array->reserve(capacity);
        while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
        {
            float float_value;
            if (fr[0].getFloat(float_value))
            {
                ++fr;
                array->push_back(float_value);
            }
            else ++fr;
        }
        ++fr;
        return_array = array;
    }
    else if (strcmp(arrayName,"DoubleArray")==0)
    {
        DoubleArray* array = new DoubleArray;
        array->reserve(capacity);
        while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
        {
            double double_value;
            if (fr[0].getFloat(double_value))
            {
                ++fr;
                array->push_back(double_value);
            }
            else ++fr;
        }
        ++fr;
        return_array = array;
    }
    else if (strcmp(arrayName,"Vec2Array")==0)
    {
        Vec2Array* array = new Vec2Array;
        array->reserve(capacity);
        while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
        {
            Vec2 v;
            if (fr[0].getFloat(v.x()) && fr[1].getFloat(v.y()))
            {
                fr += 2;
                array->push_back(v);
            }
            else ++fr;
        }
        ++fr;
        return_array = array;
    }
    else if (strcmp(arrayName,"Vec2dArray")==0)
    {
        Vec2dArray* array = new Vec2dArray;
        array->reserve(capacity);
        while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
        {
            Vec2d v;
            if (fr[0].getFloat(v.x()) && fr[1].getFloat(v.y()))
            {
                fr += 2;
                array->push_back(v);
            }
            else ++fr;
        }
        ++fr;
        return_array = array;
    }
    else if (strcmp(arrayName,"Vec3Array")==0)
    {
        Vec3Array* array = new Vec3Array;
        array->reserve(capacity);
        while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
        {
            Vec3 v;
            if (fr[0].getFloat(v.x()) && fr[1].getFloat(v.y()) && fr[2].getFloat(v.z()))
            {
                fr += 3;
                array->push_back(v);
            }
            else ++fr;
        }
        ++fr;
        return_array = array;
    }
    else if (strcmp(arrayName,"Vec3dArray")==0)
    {
        Vec3dArray* array = new Vec3dArray;
        array->reserve(capacity);
        while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
        {
            Vec3d v;
            if (fr[0].getFloat(v.x()) && fr[1].getFloat(v.y()) && fr[2].getFloat(v.z()))
            {
                fr += 3;
                array->push_back(v);
            }
            else ++fr;
        }
        ++fr;
        return_array = array;
    }
    else if (strcmp(arrayName,"Vec4Array")==0)
    {
        Vec4Array* array = new Vec4Array;
        array->reserve(capacity);
        while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
        {
            Vec4 v;
            if (fr[0].getFloat(v.x()) && fr[1].getFloat(v.y()) && fr[2].getFloat(v.z()) && fr[3].getFloat(v.w()))
            {
                fr += 4;
                array->push_back(v);
            }
            else ++fr;
        }
        ++fr;
        return_array = array;
    }
    else if (strcmp(arrayName,"Vec4dArray")==0)
    {
        Vec4dArray* array = new Vec4dArray;
        array->reserve(capacity);
        while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
        {
            Vec4d v;
            if (fr[0].getFloat(v.x()) && fr[1].getFloat(v.y()) && fr[2].getFloat(v.z()) && fr[3].getFloat(v.w()))
            {
                fr += 4;
                array->push_back(v);
            }
            else ++fr;
        }
        ++fr;
        return_array = array;
    }
    else if (strcmp(arrayName,"Vec2bArray")==0)
    {
        Vec2bArray* array = new Vec2bArray;
        array->reserve(capacity);
        while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
        {
            unsigned int r,g;
            if (fr[0].getUInt(r) &&
                fr[1].getUInt(g))
            {
                fr+=2;
                array->push_back(osg::Vec2b(r,g));
            }
            else ++fr;
        }
        ++fr;
        return_array = array;
    }
    else if (strcmp(arrayName,"Vec3bArray")==0)
    {
        Vec3bArray* array = new Vec3bArray;
        array->reserve(capacity);
        while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
        {
            unsigned int r,g,b;
            if (fr[0].getUInt(r) &&
                fr[1].getUInt(g) &&
                fr[2].getUInt(b))
            {
                fr+=3;
                array->push_back(osg::Vec3b(r,g,b));
            }
            else ++fr;
        }
        ++fr;
        return_array = array;
    }
    else if (strcmp(arrayName,"Vec4bArray")==0)
    {
        Vec4bArray* array = new Vec4bArray;
        array->reserve(capacity);
        while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
        {
            unsigned int r,g,b,a;
            if (fr[0].getUInt(r) &&
                fr[1].getUInt(g) &&
                fr[2].getUInt(b) &&
                fr[3].getUInt(a))
            {
                fr+=4;
                array->push_back(osg::Vec4b(r,g,b,a));
            }
            else ++fr;
        }
        ++fr;
        return_array = array;
    }
    else if (strcmp(arrayName,"Vec2sArray")==0)
    {
        Vec2sArray* array = new Vec2sArray;
        array->reserve(capacity);
        while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
        {
            unsigned int r,g;
            if (fr[0].getUInt(r) &&
                fr[1].getUInt(g))
            {
                fr+=2;
                array->push_back(osg::Vec2s(r,g));
            }
            else ++fr;
        }
        ++fr;
        return_array = array;
    }
    else if (strcmp(arrayName,"Vec3sArray")==0)
    {
        Vec3sArray* array = new Vec3sArray;
        array->reserve(capacity);
        while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
        {
            unsigned int r,g,b;
            if (fr[0].getUInt(r) &&
                fr[1].getUInt(g) &&
                fr[2].getUInt(b))
            {
                fr+=3;
                array->push_back(osg::Vec3s(r,g,b));
            }
            else ++fr;
        }
        ++fr;
        return_array = array;
    }
    else if (strcmp(arrayName,"Vec4sArray")==0)
    {
        Vec4sArray* array = new Vec4sArray;
        array->reserve(capacity);
        while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
        {
            unsigned int r,g,b,a;
            if (fr[0].getUInt(r) &&
                fr[1].getUInt(g) &&
                fr[2].getUInt(b) &&
                fr[3].getUInt(a))
            {
                fr+=4;
                array->push_back(osg::Vec4s(r,g,b,a));
            }
            else ++fr;
        }
        ++fr;
        return_array = array;
    }
    
    if (return_array) 
    {
        if (!uniqueID.empty()) fr.registerUniqueIDForObject(uniqueID.c_str(),return_array);
    }
    
    return return_array;
}


bool Array_writeLocalData(const Array& array,Output& fw)
{
    if (array.referenceCount()>1)
    {
        std::string uniqueID;
        if (fw.getUniqueIDForObject(&array,uniqueID))
        {
            fw << "Use " << uniqueID << std::endl;
            return true;
        }
        else
        {
            std::string uniqueID;
            fw.createUniqueIDForObject(&array,uniqueID);
            fw.registerUniqueIDForObject(&array,uniqueID);
            fw << "UniqueID " << uniqueID << " ";
        }
    }


    switch(array.getType())
    {
        case(Array::ByteArrayType):
            {
                const ByteArray& carray = static_cast<const ByteArray&>(array);
                fw<<array.className()<<" "<<carray.size()<<std::endl;
                writeArrayAsInts(fw,carray.begin(),carray.end());
                return true;
            }
            break;
        case(Array::ShortArrayType):
            {
                const ShortArray& carray = static_cast<const ShortArray&>(array);
                fw<<array.className()<<" "<<carray.size()<<std::endl;
                writeArray(fw,carray.begin(),carray.end());
                return true;
            }
            break;
        case(Array::IntArrayType):
            {
                const IntArray& carray = static_cast<const IntArray&>(array);
                fw<<array.className()<<" "<<carray.size()<<std::endl;
                writeArray(fw,carray.begin(),carray.end());
                return true;
            }
            break;
        case(Array::UByteArrayType):
            {
                const UByteArray& carray = static_cast<const UByteArray&>(array);
                fw<<array.className()<<" "<<carray.size()<<std::endl;
                writeArrayAsInts(fw,carray.begin(),carray.end());
                return true;
            }
            break;
        case(Array::UShortArrayType):
            {
                const UShortArray& carray = static_cast<const UShortArray&>(array);
                fw<<array.className()<<" "<<carray.size()<<std::endl;
                writeArray(fw,carray.begin(),carray.end());
                return true;
            }
            break;
        case(Array::UIntArrayType):
            {
                const UIntArray& carray = static_cast<const UIntArray&>(array);
                fw<<array.className()<<" "<<carray.size()<<" ";
                writeArray(fw,carray.begin(),carray.end());
                return true;
            }
            break;
        case(Array::Vec4ubArrayType):
            {
                const Vec4ubArray& carray = static_cast<const Vec4ubArray&>(array);
                fw<<array.className()<<" "<<carray.size()<<" ";
                writeArray(fw,carray.begin(),carray.end(),1);
                return true;
            }
            break;
        case(Array::FloatArrayType):
            {
                const FloatArray& carray = static_cast<const FloatArray&>(array);
                fw<<array.className()<<" "<<carray.size()<<std::endl;
                writeArray(fw,carray.begin(),carray.end());
                return true;
            }
            break;
        case(Array::Vec2ArrayType):
            {
                const Vec2Array& carray = static_cast<const Vec2Array&>(array);
                fw<<array.className()<<" "<<carray.size()<<std::endl;
                writeArray(fw,carray.begin(),carray.end(),1);
                return true;
            }
            break;
        case(Array::Vec3ArrayType):
            {
                const Vec3Array& carray = static_cast<const Vec3Array&>(array);
                fw<<array.className()<<" "<<carray.size()<<std::endl;
                writeArray(fw,carray.begin(),carray.end(),1);
                return true;
            }
            break;
        case(Array::Vec4ArrayType):
            {
                const Vec4Array& carray = static_cast<const Vec4Array&>(array);
                fw<<array.className()<<" "<<carray.size()<<std::endl;
                writeArray(fw,carray.begin(),carray.end(),1);
                return true;
            }
            break;
        case(Array::DoubleArrayType):
            {
                int prec = fw.precision(15);
                const DoubleArray& carray = static_cast<const DoubleArray&>(array);
                fw<<array.className()<<" "<<carray.size()<<std::endl;
                writeArray(fw,carray.begin(),carray.end());
                fw.precision(prec);
                return true;
            }
            break;
        case(Array::Vec2dArrayType):
            {
                int prec = fw.precision(15);
                const Vec2dArray& carray = static_cast<const Vec2dArray&>(array);
                fw<<array.className()<<" "<<carray.size()<<std::endl;
                writeArray(fw,carray.begin(),carray.end(),1);
                fw.precision(prec);
                return true;
            }
            break;
        case(Array::Vec3dArrayType):
            {
                int prec = fw.precision(15);
                const Vec3dArray& carray = static_cast<const Vec3dArray&>(array);
                fw<<array.className()<<" "<<carray.size()<<std::endl;
                writeArray(fw,carray.begin(),carray.end(),1);
                fw.precision(prec);
                return true;
            }
            break;
        case(Array::Vec4dArrayType):
            {
                int prec = fw.precision(15);
                const Vec4dArray& carray = static_cast<const Vec4dArray&>(array);
                fw<<array.className()<<" "<<carray.size()<<std::endl;
                writeArray(fw,carray.begin(),carray.end(),1);
                fw.precision(prec);
                return true;
            }
            break;
        case(Array::Vec2sArrayType):
            {
                const Vec2sArray& carray = static_cast<const Vec2sArray&>(array);
                fw<<array.className()<<" "<<carray.size()<<std::endl;
                writeArray(fw,carray.begin(),carray.end(), 3);
                return true;
            }
            break;
        case(Array::Vec3sArrayType):
            {
                const Vec3sArray& carray = static_cast<const Vec3sArray&>(array);
                fw<<array.className()<<" "<<carray.size()<<std::endl;
                writeArray(fw,carray.begin(),carray.end(), 2);
                return true;
            }
            break;
        case(Array::Vec4sArrayType):
            {
                const Vec4sArray& carray = static_cast<const Vec4sArray&>(array);
                fw<<array.className()<<" "<<carray.size()<<std::endl;
                writeArray(fw,carray.begin(),carray.end(), 1);
                return true;
            }
            break;
        case(Array::Vec2bArrayType):
            {
                const Vec2bArray& carray = static_cast<const Vec2bArray&>(array);
                fw<<array.className()<<" "<<carray.size()<<" ";
                writeArray(fw,carray.begin(),carray.end(),1);
                return true;
            }
            break;
        case(Array::Vec3bArrayType):
            {
                const Vec3bArray& carray = static_cast<const Vec3bArray&>(array);
                fw<<array.className()<<" "<<carray.size()<<" ";
                writeArray(fw,carray.begin(),carray.end(),1);
                return true;
            }
            break;
        case(Array::Vec4bArrayType):
            {
                const Vec4bArray& carray = static_cast<const Vec4bArray&>(array);
                fw<<array.className()<<" "<<carray.size()<<" ";
                writeArray(fw,carray.begin(),carray.end(),1);
                return true;
            }
            break;
        case(Array::ArrayType):
        default:
            return false;
    }
}


bool Primitive_readLocalData(Input& fr,osg::Geometry& geom)
{
    bool iteratorAdvanced = false;
    bool firstMatched = false;
    if ((firstMatched = fr.matchSequence("DrawArrays %w %i %i %i")) || 
         fr.matchSequence("DrawArrays %w %i %i") )
    {
        
        GLenum mode;
        Geometry_matchPrimitiveModeStr(fr[1].getStr(),mode);

        int first;
        fr[2].getInt(first);
        
        int count;
        fr[3].getInt(count);

        int numInstances = 0;
        if (firstMatched)
        {
            fr[4].getInt(numInstances);
            fr += 5;
        }
        else
        {
            fr += 4;
        }

        geom.addPrimitiveSet(new DrawArrays(mode, first, count, numInstances));

        
        iteratorAdvanced = true;
        
    }
    else if ((firstMatched = fr.matchSequence("DrawArrayLengths %w %i %i %i {")) || 
         fr.matchSequence("DrawArrayLengths %w %i %i {") )
    {
        int entry = fr[1].getNoNestedBrackets();

        GLenum mode;
        Geometry_matchPrimitiveModeStr(fr[1].getStr(),mode);

        int first;
        fr[2].getInt(first);

        int capacity;
        fr[3].getInt(capacity);
        
        int numInstances = 0;
        if (firstMatched)
        {
            fr[4].getInt(numInstances);
            fr += 6;
        }
        else
        {
            fr += 5;
        }

        DrawArrayLengths* prim = new DrawArrayLengths;
        prim->setMode(mode);
        prim->setNumInstances(numInstances);
        prim->setFirst(first);
        prim->reserve(capacity);

        while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
        {
            unsigned int i;
            if (fr[0].getUInt(i))
            {
                prim->push_back(i);
                ++fr;
            }
        }
         ++fr;
         
         geom.addPrimitiveSet(prim);
   
        iteratorAdvanced = true;
    }
    else if ((firstMatched = fr.matchSequence("DrawElementsUByte %w %i %i {")) || 
         fr.matchSequence("DrawElementsUByte %w %i {"))
    {
        int entry = fr[1].getNoNestedBrackets();

        GLenum mode;
        Geometry_matchPrimitiveModeStr(fr[1].getStr(),mode);

        int capacity;
        fr[2].getInt(capacity);
        
        int numInstances = 0;
        if (firstMatched)
        {
            fr[3].getInt(numInstances);
            fr += 5;
        }
        else
        {
            fr += 4;
        }

        DrawElementsUByte* prim = new DrawElementsUByte;
        prim->setMode(mode);
        prim->setNumInstances(numInstances);
        prim->reserve(capacity);

        while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
        {
            unsigned int i;
            if (fr[0].getUInt(i))
            {
                prim->push_back(i);
                ++fr;
            }
        }
         ++fr;
         
         geom.addPrimitiveSet(prim);
   
        iteratorAdvanced = true;
    }
    else if ((firstMatched = fr.matchSequence("DrawElementsUShort %w %i %i {")) || 
         fr.matchSequence("DrawElementsUShort %w %i {"))
    {
        int entry = fr[1].getNoNestedBrackets();

        GLenum mode;
        Geometry_matchPrimitiveModeStr(fr[1].getStr(),mode);

        int capacity;
        fr[2].getInt(capacity);
        
        int numInstances = 0;
        if (firstMatched)
        {
            fr[3].getInt(numInstances);
            fr += 5;
        }
        else
        {
            fr += 4;
        }
        
        DrawElementsUShort* prim = new DrawElementsUShort;
        prim->setMode(mode);
        prim->setNumInstances(numInstances);
        prim->reserve(capacity);

        while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
        {
            unsigned int i;
            if (fr[0].getUInt(i))
            {
                prim->push_back(i);
                ++fr;
            }
        }
         ++fr;
         
         geom.addPrimitiveSet(prim);
   
        iteratorAdvanced = true;
    }
    else if ((firstMatched = fr.matchSequence("DrawElementsUInt %w %i %i {")) || 
              fr.matchSequence("DrawElementsUInt %w %i {"))
    {
        int entry = fr[1].getNoNestedBrackets();

        GLenum mode;
        Geometry_matchPrimitiveModeStr(fr[1].getStr(),mode);

        int capacity;
        fr[2].getInt(capacity);
        
        int numInstances = 0;
        if (firstMatched)
        {
            fr[3].getInt(numInstances);
            fr += 5;
        }
        else
        {
            fr += 4;
        }
        
        DrawElementsUInt* prim = new DrawElementsUInt;
        prim->setMode(mode);
        prim->setNumInstances(numInstances);
        prim->reserve(capacity);

        while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
        {
            unsigned int i;
            if (fr[0].getUInt(i))
            {
                prim->push_back(i);
                ++fr;
            }
        }
         ++fr;
         
         geom.addPrimitiveSet(prim);
   
        iteratorAdvanced = true;
    }

    return iteratorAdvanced;
}

bool Primitive_writeLocalData(const PrimitiveSet& prim,Output& fw)
{

    switch(prim.getType())
    {
        case(PrimitiveSet::DrawArraysPrimitiveType):
            {
                const DrawArrays& cprim = static_cast<const DrawArrays&>(prim);
                fw<<cprim.className()<<" "<<Geometry_getPrimitiveModeStr(cprim.getMode())<<" "<<cprim.getFirst()<<" "<<cprim.getCount();
                if (prim.getNumInstances()>0) fw<<" "<<prim.getNumInstances();
                fw<<std::endl;
                return true;
            }
            break;
        case(PrimitiveSet::DrawArrayLengthsPrimitiveType):
            {
                const DrawArrayLengths& cprim = static_cast<const DrawArrayLengths&>(prim);
                fw<<cprim.className()<<" "<<Geometry_getPrimitiveModeStr(cprim.getMode())<<" "<<cprim.getFirst()<<" "<<cprim.size();
                if (prim.getNumInstances()>0) fw<<" "<<prim.getNumInstances();
                fw<<std::endl;
                writeArray(fw,cprim.begin(),cprim.end());
                return true;
            }
            break;
        case(PrimitiveSet::DrawElementsUBytePrimitiveType):
            {
                const DrawElementsUByte& cprim = static_cast<const DrawElementsUByte&>(prim);
                fw<<cprim.className()<<" "<<Geometry_getPrimitiveModeStr(cprim.getMode())<<" "<<cprim.size();
                if (prim.getNumInstances()>0) fw<<" "<<prim.getNumInstances();
                fw<<std::endl;
                writeArrayAsInts(fw,cprim.begin(),cprim.end());
                return true;
            }
            break;
        case(PrimitiveSet::DrawElementsUShortPrimitiveType):
            {
                const DrawElementsUShort& cprim = static_cast<const DrawElementsUShort&>(prim);
                fw<<cprim.className()<<" "<<Geometry_getPrimitiveModeStr(cprim.getMode())<<" "<<cprim.size();
                if (prim.getNumInstances()>0) fw<<" "<<prim.getNumInstances();
                fw<<std::endl;
                writeArray(fw,cprim.begin(),cprim.end());
                return true;
            }
            break;
        case(PrimitiveSet::DrawElementsUIntPrimitiveType):
            {
                const DrawElementsUInt& cprim = static_cast<const DrawElementsUInt&>(prim);
                fw<<cprim.className()<<" "<<Geometry_getPrimitiveModeStr(cprim.getMode())<<" "<<cprim.size();
                if (prim.getNumInstances()>0) fw<<" "<<prim.getNumInstances();
                fw<<std::endl;
                writeArray(fw,cprim.begin(),cprim.end());
                return true;
            }
            break;
        default:
            return false;
    }
}

bool Geometry_writeLocalData(const Object& obj, Output& fw)
{
    const Geometry& geom = static_cast<const Geometry&>(obj);

    const Geometry::PrimitiveSetList& primitives = geom.getPrimitiveSetList();
    if (!primitives.empty())
    {
        fw.indent() << "PrimitiveSets "<<primitives.size()<<std::endl;
        fw.indent() << "{"<<std::endl;
        fw.moveIn();
        for(Geometry::PrimitiveSetList::const_iterator itr=primitives.begin();
            itr!=primitives.end();
            ++itr)
        {
            fw.indent();
            Primitive_writeLocalData(**itr,fw);
        }
        fw.moveOut();
        fw.indent() << "}"<<std::endl;
    }

    if (geom.getVertexArray())
    {
//         const Vec3Array& vertices = *geom.getVertexArray();
//         fw.indent()<<"VertexArray "<<vertices.size()<<std::endl;
//         Array_writeLocalData(fw,vertices.begin(),vertices.end(),1);

        fw.indent()<<"VertexArray ";
        Array_writeLocalData(*geom.getVertexArray(),fw);
        
    }
    if (geom.getVertexIndices())
    {
        fw.indent()<<"VertexIndices ";
        Array_writeLocalData(*geom.getVertexIndices(),fw);        
    }

    if (geom.getNormalArray())
    {
        
        fw.indent()<<"NormalBinding "<<Geometry_getBindingTypeStr(geom.getNormalBinding())<<std::endl;
        
//        const Vec3Array& normals = *geom.getNormalArray();
//        fw.indent()<<"NormalArray "<<normals.size()<<std::endl;
//        Array_writeLocalData(fw,normals.begin(),normals.end(),1);

        fw.indent()<<"NormalArray ";
        Array_writeLocalData(*geom.getNormalArray(),fw);        
        
    }
    if (geom.getNormalIndices())
    {
        fw.indent()<<"NormalIndices ";
        Array_writeLocalData(*geom.getNormalIndices(),fw);        
    }

    if (geom.getColorArray())
    {
        fw.indent()<<"ColorBinding "<<Geometry_getBindingTypeStr(geom.getColorBinding())<<std::endl;
        fw.indent()<<"ColorArray ";
        Array_writeLocalData(*geom.getColorArray(),fw);
    }
    if (geom.getColorIndices())
    {
        fw.indent()<<"ColorIndices ";
        Array_writeLocalData(*geom.getColorIndices(),fw);        
    }

    if (geom.getSecondaryColorArray())
    {
        fw.indent()<<"SecondaryColorBinding "<<Geometry_getBindingTypeStr(geom.getSecondaryColorBinding())<<std::endl;
        fw.indent()<<"SecondaryColorArray ";
        Array_writeLocalData(*geom.getSecondaryColorArray(),fw);
    }
    if (geom.getSecondaryColorIndices())
    {
        fw.indent()<<"SecondayColorIndices ";
        Array_writeLocalData(*geom.getSecondaryColorIndices(),fw);        
    }

    if (geom.getFogCoordArray())
    {
        fw.indent()<<"FogCoordBinding "<<Geometry_getBindingTypeStr(geom.getFogCoordBinding())<<std::endl;
        fw.indent()<<"FogCoordArray ";
        Array_writeLocalData(*geom.getFogCoordArray(),fw);
    }
    if (geom.getFogCoordIndices())
    {
        fw.indent()<<"FogCoordIndices ";
        Array_writeLocalData(*geom.getFogCoordIndices(),fw);        
    }

    const Geometry::ArrayDataList& tcal=geom.getTexCoordArrayList();
    unsigned int i;
    for(i=0;i<tcal.size();++i)
    {
        if (tcal[i].array.valid())
        {
            fw.indent()<<"TexCoordArray "<<i<<" ";
            Array_writeLocalData(*(tcal[i].array),fw);
        }
        if (tcal[i].indices.valid())
        {
            fw.indent()<<"TexCoordIndices "<<i<<" ";
            Array_writeLocalData(*(tcal[i].indices),fw);
        }
    }
    
    const Geometry::ArrayDataList& vaal=geom.getVertexAttribArrayList();
    for(i=0;i<vaal.size();++i)
    {
        const osg::Geometry::ArrayData& arrayData = vaal[i];
    
        if (arrayData.array.valid())
        {
            fw.indent()<<"VertexAttribBinding "<<i<<" "<<Geometry_getBindingTypeStr(arrayData.binding)<<std::endl;
            
            if (arrayData.normalize)
                fw.indent()<<"VertexAttribNormalize "<<i<<" TRUE"<<std::endl;
            else
                fw.indent()<<"VertexAttribNormalize "<<i<<" FALSE"<<std::endl;
                
            fw.indent()<<"VertexAttribArray "<<i<<" ";
            Array_writeLocalData(*(arrayData.array),fw);
        }
        if (arrayData.indices.valid())
        {
            fw.indent()<<"VertexAttribIndices "<<i<<" ";
            Array_writeLocalData(*(arrayData.indices),fw);
        }
    }

    return true;
}

bool Geometry_matchBindingTypeStr(const char* str,Geometry::AttributeBinding& mode)
{
    if (strcmp(str,"OFF")==0) mode = Geometry::BIND_OFF;
    else if (strcmp(str,"OVERALL")==0) mode = Geometry::BIND_OVERALL;
    else if (strcmp(str,"PER_PRIMITIVE")==0) mode = Geometry::BIND_PER_PRIMITIVE;
    else if (strcmp(str,"PER_PRIMITIVE_SET")==0) mode = Geometry::BIND_PER_PRIMITIVE_SET;
    else if (strcmp(str,"PER_VERTEX")==0) mode = Geometry::BIND_PER_VERTEX;
    else return false;
    return true;
}


const char* Geometry_getBindingTypeStr(Geometry::AttributeBinding mode)
{
    switch(mode)
    {
        case (Geometry::BIND_OVERALL)           : return "OVERALL";
        case (Geometry::BIND_PER_PRIMITIVE)     : return "PER_PRIMITIVE";
        case (Geometry::BIND_PER_PRIMITIVE_SET) : return "PER_PRIMITIVE_SET";
        case (Geometry::BIND_PER_VERTEX)        : return "PER_VERTEX";
        case (Geometry::BIND_OFF)               :
        default                                        : return "OFF";
    }
}

bool Geometry_matchPrimitiveModeStr(const char* str,GLenum& mode)
{
    if      (strcmp(str,"POINTS")==0)           mode = PrimitiveSet::POINTS;
    else if (strcmp(str,"LINES")==0)            mode = PrimitiveSet::LINES;
    else if (strcmp(str,"LINE_STRIP")==0)       mode = PrimitiveSet::LINE_STRIP;
    else if (strcmp(str,"LINE_LOOP")==0)        mode = PrimitiveSet::LINE_LOOP;
    else if (strcmp(str,"TRIANGLES")==0)        mode = PrimitiveSet::TRIANGLES;
    else if (strcmp(str,"TRIANGLE_STRIP")==0)   mode = PrimitiveSet::TRIANGLE_STRIP;
    else if (strcmp(str,"TRIANGLE_FAN")==0)     mode = PrimitiveSet::TRIANGLE_FAN;
    else if (strcmp(str,"QUADS")==0)            mode = PrimitiveSet::QUADS;
    else if (strcmp(str,"QUAD_STRIP")==0)       mode = PrimitiveSet::QUAD_STRIP;
    else if (strcmp(str,"POLYGON")==0)          mode = PrimitiveSet::POLYGON;
    else return false;
    return true;
}


const char* Geometry_getPrimitiveModeStr(GLenum mode)
{
    switch(mode)
    {
        case (PrimitiveSet::POINTS)            : return "POINTS";
        case (PrimitiveSet::LINES)             : return "LINES";
        case (PrimitiveSet::LINE_STRIP)        : return "LINE_STRIP";
        case (PrimitiveSet::LINE_LOOP)         : return "LINE_LOOP";
        case (PrimitiveSet::TRIANGLES)         : return "TRIANGLES";
        case (PrimitiveSet::TRIANGLE_STRIP)    : return "TRIANGLE_STRIP";
        case (PrimitiveSet::TRIANGLE_FAN)      : return "TRIANGLE_FAN";
        case (PrimitiveSet::QUADS)             : return "QUADS";
        case (PrimitiveSet::QUAD_STRIP)        : return "QUAD_STRIP";
        case (PrimitiveSet::POLYGON)           : return "POLYGON";
        default                                : return "UnknownPrimitveType";
    }
}
