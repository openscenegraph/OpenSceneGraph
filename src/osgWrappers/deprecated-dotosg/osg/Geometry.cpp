#include <osg/Config>
#ifndef OSG_USE_DEPRECATED_GEOMETRY_METHODS
#define OSG_USE_DEPRECATED_GEOMETRY_METHODS 1
#endif

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

bool Geometry_matchBindingTypeStr(const char* str,deprecated_osg::Geometry::AttributeBinding& mode);
const char* Geometry_getBindingTypeStr(deprecated_osg::Geometry::AttributeBinding mode);

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

    deprecated_osg::Geometry& geom = static_cast<deprecated_osg::Geometry&>(obj);

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


    deprecated_osg::Geometry::AttributeBinding normalBinding = deprecated_osg::Geometry::BIND_OFF;
    if (fr[0].matchWord("NormalBinding") && Geometry_matchBindingTypeStr(fr[1].getStr(),normalBinding))
    {
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

        geom.setNormalBinding(normalBinding);
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

    deprecated_osg::Geometry::AttributeBinding colorBinding = deprecated_osg::Geometry::BIND_OFF;
    if (fr[0].matchWord("ColorBinding") && Geometry_matchBindingTypeStr(fr[1].getStr(),colorBinding))
    {
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
            geom.setColorBinding(colorBinding);
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


    deprecated_osg::Geometry::AttributeBinding secondaryColorBinding = deprecated_osg::Geometry::BIND_OFF;
    if (fr[0].matchWord("SecondaryColorBinding") && Geometry_matchBindingTypeStr(fr[1].getStr(),secondaryColorBinding))
    {
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
            geom.setSecondaryColorBinding(secondaryColorBinding);
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


    deprecated_osg::Geometry::AttributeBinding fogCoordBinding = deprecated_osg::Geometry::BIND_OFF;
    if (fr[0].matchWord("FogCoordBinding") && Geometry_matchBindingTypeStr(fr[1].getStr(),fogCoordBinding))
    {
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
            geom.setFogCoordBinding(fogCoordBinding);
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

    deprecated_osg::Geometry::AttributeBinding vertexAttribBinding = deprecated_osg::Geometry::BIND_OFF;
    if (fr.matchSequence("VertexAttribBinding %i %w") && Geometry_matchBindingTypeStr(fr[2].getStr(),vertexAttribBinding))
    {
        int unit=0;
        fr[1].getInt(unit);
        fr+=3;
        iteratorAdvanced = true;
    }

    bool vertexAttribNormalize = false;
    if (fr.matchSequence("VertexAttribNormalize %i %w"))
    {
        int unit=0;
        fr[1].getInt(unit);

        vertexAttribNormalize = fr[2].matchString("TRUE");

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
            geom.setVertexAttribBinding(unit,vertexAttribBinding);
            geom.setVertexAttribNormalize(unit,vertexAttribNormalize);
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
                fw<<array.className()<<" "<<array.getNumElements()<<std::endl;
                const ByteArray::ElementDataType* base = static_cast<const ByteArray::ElementDataType*>(array.getDataPointer());
                writeArrayAsInts(fw,&base[0], &base[array.getNumElements()]);
                return true;
            }
            break;
        case(Array::ShortArrayType):
            {
                fw<<array.className()<<" "<<array.getNumElements()<<std::endl;
                const ShortArray::ElementDataType* base = static_cast<const ShortArray::ElementDataType*>(array.getDataPointer());
                writeArray(fw,&base[0], &base[array.getNumElements()]);
                return true;
            }
            break;
        case(Array::IntArrayType):
            {
                fw<<array.className()<<" "<<array.getNumElements()<<std::endl;
                const IntArray::ElementDataType* base = static_cast<const IntArray::ElementDataType*>(array.getDataPointer());
                writeArray(fw,&base[0], &base[array.getNumElements()]);
                return true;
            }
            break;
        case(Array::UByteArrayType):
            {
                fw<<array.className()<<" "<<array.getNumElements()<<std::endl;
                const UByteArray::ElementDataType* base = static_cast<const UByteArray::ElementDataType*>(array.getDataPointer());
                writeArrayAsInts(fw,&base[0], &base[array.getNumElements()]);
                return true;
            }
            break;
        case(Array::UShortArrayType):
            {
                fw<<array.className()<<" "<<array.getNumElements()<<std::endl;
                const UShortArray::ElementDataType* base = static_cast<const UShortArray::ElementDataType*>(array.getDataPointer());
                writeArray(fw,&base[0], &base[array.getNumElements()]);
                return true;
            }
            break;
        case(Array::UIntArrayType):
            {
                fw<<array.className()<<" "<<array.getNumElements()<<std::endl;
                const UIntArray::ElementDataType* base = static_cast<const UIntArray::ElementDataType*>(array.getDataPointer());
                writeArray(fw,&base[0], &base[array.getNumElements()]);
                return true;
            }
            break;
        case(Array::Vec4ubArrayType):
            {
                fw<<array.className()<<" "<<array.getNumElements()<<std::endl;
                const Vec4ubArray::ElementDataType* base = static_cast<const Vec4ubArray::ElementDataType*>(array.getDataPointer());
                writeArray(fw,&base[0], &base[array.getNumElements()],1);
                return true;
            }
            break;
        case(Array::FloatArrayType):
            {
                fw<<array.className()<<" "<<array.getNumElements()<<std::endl;
                const FloatArray::ElementDataType* base = static_cast<const FloatArray::ElementDataType*>(array.getDataPointer());
                writeArray(fw,&base[0], &base[array.getNumElements()]);
                return true;
            }
            break;
        case(Array::Vec2ArrayType):
            {
                fw<<array.className()<<" "<<array.getNumElements()<<std::endl;
                const Vec2Array::ElementDataType* base = static_cast<const Vec2Array::ElementDataType*>(array.getDataPointer());
                writeArray(fw,&base[0], &base[array.getNumElements()],1);
                return true;
            }
            break;
        case(Array::Vec3ArrayType):
            {
                fw<<array.className()<<" "<<array.getNumElements()<<std::endl;
                const Vec3Array::ElementDataType* base = static_cast<const Vec3Array::ElementDataType*>(array.getDataPointer());
                writeArray(fw,&base[0], &base[array.getNumElements()],1);
                return true;
            }
            break;
        case(Array::Vec4ArrayType):
            {
                fw<<array.className()<<" "<<array.getNumElements()<<std::endl;
                const Vec4Array::ElementDataType* base = static_cast<const Vec4Array::ElementDataType*>(array.getDataPointer());
                writeArray(fw,&base[0], &base[array.getNumElements()],1);
                return true;
            }
            break;
        case(Array::DoubleArrayType):
            {
                int prec = fw.precision(15);
                fw<<array.className()<<" "<<array.getNumElements()<<std::endl;
                const DoubleArray::ElementDataType* base = static_cast<const DoubleArray::ElementDataType*>(array.getDataPointer());
                writeArray(fw,&base[0], &base[array.getNumElements()]);
                fw.precision(prec);
                return true;
            }
            break;
        case(Array::Vec2dArrayType):
            {
                int prec = fw.precision(15);
                fw<<array.className()<<" "<<array.getNumElements()<<std::endl;
                const Vec2dArray::ElementDataType* base = static_cast<const Vec2dArray::ElementDataType*>(array.getDataPointer());
                writeArray(fw,&base[0], &base[array.getNumElements()],1);
                fw.precision(prec);
                return true;
            }
            break;
        case(Array::Vec3dArrayType):
            {
                int prec = fw.precision(15);
                fw<<array.className()<<" "<<array.getNumElements()<<std::endl;
                const Vec3dArray::ElementDataType* base = static_cast<const Vec3dArray::ElementDataType*>(array.getDataPointer());
                writeArray(fw,&base[0], &base[array.getNumElements()],1);
                fw.precision(prec);
                return true;
            }
            break;
        case(Array::Vec4dArrayType):
            {
                int prec = fw.precision(15);
                fw<<array.className()<<" "<<array.getNumElements()<<std::endl;
                const Vec4dArray::ElementDataType* base = static_cast<const Vec4dArray::ElementDataType*>(array.getDataPointer());
                writeArray(fw,&base[0], &base[array.getNumElements()],1);
                fw.precision(prec);
                return true;
            }
            break;
        case(Array::Vec2sArrayType):
            {
                fw<<array.className()<<" "<<array.getNumElements()<<std::endl;
                const Vec2sArray::ElementDataType* base = static_cast<const Vec2sArray::ElementDataType*>(array.getDataPointer());
                writeArray(fw,&base[0], &base[array.getNumElements()], 3);
                return true;
            }
            break;
        case(Array::Vec3sArrayType):
            {
                fw<<array.className()<<" "<<array.getNumElements()<<std::endl;
                const Vec3sArray::ElementDataType* base = static_cast<const Vec3sArray::ElementDataType*>(array.getDataPointer());
                writeArray(fw,&base[0], &base[array.getNumElements()], 2);
                return true;
            }
            break;
        case(Array::Vec4sArrayType):
            {
                fw<<array.className()<<" "<<array.getNumElements()<<std::endl;
                const Vec4sArray::ElementDataType* base = static_cast<const Vec4sArray::ElementDataType*>(array.getDataPointer());
                writeArray(fw,&base[0], &base[array.getNumElements()], 1);
                return true;
            }
            break;
        case(Array::Vec2bArrayType):
            {
                fw<<array.className()<<" "<<array.getNumElements()<<std::endl;
                const Vec2bArray::ElementDataType* base = static_cast<const Vec2bArray::ElementDataType*>(array.getDataPointer());
                writeArray(fw,&base[0], &base[array.getNumElements()],1);
                return true;
            }
            break;
        case(Array::Vec3bArrayType):
            {
                fw<<array.className()<<" "<<array.getNumElements()<<std::endl;
                const Vec3bArray::ElementDataType* base = static_cast<const Vec3bArray::ElementDataType*>(array.getDataPointer());
                writeArray(fw,&base[0], &base[array.getNumElements()],1);
                return true;
            }
            break;
        case(Array::Vec4bArrayType):
            {
                fw<<array.className()<<" "<<array.getNumElements()<<std::endl;
                const Vec4bArray::ElementDataType* base = static_cast<const Vec4bArray::ElementDataType*>(array.getDataPointer());
                writeArray(fw,&base[0], &base[array.getNumElements()],1);
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
    const deprecated_osg::Geometry& geom = static_cast<const deprecated_osg::Geometry&>(obj);

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

    const Geometry::ArrayList& tcal=geom.getTexCoordArrayList();
    unsigned int i;
    for(i=0;i<tcal.size();++i)
    {
        const osg::Array* array = tcal[i].get();
        if (array)
        {
            fw.indent()<<"TexCoordArray "<<i<<" ";
            Array_writeLocalData(*array,fw);
        }

        const osg::IndexArray* indices = (array!=0) ? dynamic_cast<const osg::IndexArray*>(array->getUserData()) : 0;
        if (indices)
        {
            fw.indent()<<"TexCoordIndices "<<i<<" ";
            Array_writeLocalData(*indices,fw);
        }
    }

    const Geometry::ArrayList& vaal=geom.getVertexAttribArrayList();
    for(i=0;i<vaal.size();++i)
    {
        const osg::Array* array = vaal[i].get();

        if (array)
        {
            fw.indent()<<"VertexAttribBinding "<<i<<" "<<Geometry_getBindingTypeStr(static_cast<deprecated_osg::Geometry::AttributeBinding>(array->getBinding()))<<std::endl;

            if (array->getNormalize())
                fw.indent()<<"VertexAttribNormalize "<<i<<" TRUE"<<std::endl;
            else
                fw.indent()<<"VertexAttribNormalize "<<i<<" FALSE"<<std::endl;

            fw.indent()<<"VertexAttribArray "<<i<<" ";
            Array_writeLocalData(*array,fw);
        }

        const osg::IndexArray* indices = (array!=0) ? dynamic_cast<const osg::IndexArray*>(array->getUserData()) : 0;
        if (indices)
        {
            fw.indent()<<"VertexAttribIndices "<<i<<" ";
            Array_writeLocalData(*indices,fw);
        }
    }

    return true;
}

bool Geometry_matchBindingTypeStr(const char* str,deprecated_osg::Geometry::AttributeBinding& mode)
{
    if (strcmp(str,"OFF")==0) mode = deprecated_osg::Geometry::BIND_OFF;
    else if (strcmp(str,"OVERALL")==0) mode = deprecated_osg::Geometry::BIND_OVERALL;
    else if (strcmp(str,"PER_PRIMITIVE")==0) mode = deprecated_osg::Geometry::BIND_PER_PRIMITIVE;
    else if (strcmp(str,"PER_PRIMITIVE_SET")==0) mode = deprecated_osg::Geometry::BIND_PER_PRIMITIVE_SET;
    else if (strcmp(str,"PER_VERTEX")==0) mode = deprecated_osg::Geometry::BIND_PER_VERTEX;
    else return false;
    return true;
}


const char* Geometry_getBindingTypeStr(deprecated_osg::Geometry::AttributeBinding mode)
{
    switch(mode)
    {
        case (deprecated_osg::Geometry::BIND_OVERALL)           : return "OVERALL";
        case (deprecated_osg::Geometry::BIND_PER_PRIMITIVE)     : return "PER_PRIMITIVE";
        case (deprecated_osg::Geometry::BIND_PER_PRIMITIVE_SET) : return "PER_PRIMITIVE_SET";
        case (deprecated_osg::Geometry::BIND_PER_VERTEX)        : return "PER_VERTEX";
        case (deprecated_osg::Geometry::BIND_OFF)               :
        default                                 : return "OFF";
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
    else if (strcmp(str,"LINES_ADJACENCY")==0)          mode = PrimitiveSet::LINES_ADJACENCY;
    else if (strcmp(str,"LINE_STRIP_ADJACENCY")==0)     mode = PrimitiveSet::LINE_STRIP_ADJACENCY;
    else if (strcmp(str,"TRIANGLES_ADJACENCY")==0)      mode = PrimitiveSet::TRIANGLES_ADJACENCY;
    else if (strcmp(str,"TRIANGLE_STRIP_ADJECENCY")==0) mode = PrimitiveSet::TRIANGLE_STRIP_ADJACENCY;
    else if (strcmp(str,"TRIANGLE_STRIP_ADJACENCY")==0) mode = PrimitiveSet::TRIANGLE_STRIP_ADJACENCY;
    else if (strcmp(str,"PATCHES")==0)                  mode = PrimitiveSet::PATCHES;
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
        case (PrimitiveSet::LINES_ADJACENCY)            : return "LINES_ADJACENCY";
        case (PrimitiveSet::LINE_STRIP_ADJACENCY)       : return "LINE_STRIP_ADJACENCY";
        case (PrimitiveSet::TRIANGLES_ADJACENCY)        : return "TRIANGLES_ADJACENCY";
        case (PrimitiveSet::TRIANGLE_STRIP_ADJACENCY)   : return "TRIANGLE_STRIP_ADJACENCY";
        case (PrimitiveSet::PATCHES)                    : return "PATCHES";
        default                                         : return "UnknownPrimitveType";
    }
}
