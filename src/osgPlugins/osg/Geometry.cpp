#include <osg/Geometry>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

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

bool Primitve_readLocalData(Input& fr,osg::Geometry& geom);

// register the read and write functions with the osgDB::Registry.
RegisterDotOsgWrapperProxy g_GeometryFuncProxy
(
    osgNew osg::Geometry,
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

    if (fr.matchSequence("Primitives %i {"))
    {
        int entry = fr[1].getNoNestedBrackets();

        int capacity;
        fr[1].getInt(capacity);
        
        Geometry::PrimitiveList& primitives = geom.getPrimitiveList();
        if (capacity>0) primitives.reserve(capacity);
        

        fr += 3;
        

        while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
        {
            if (!Primitve_readLocalData(fr,geom)) ++fr;
        }

        ++fr;
        
        iteratorAdvanced = true;

    }

    if (fr.matchSequence("VertexArray %i {"))
    {
    
        int entry = fr[0].getNoNestedBrackets();

        int capacity;
        fr[1].getInt(capacity);
        
        Vec3Array* vertices = osgNew Vec3Array;
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

    Geometry::AttributeBinding normalBinding=Geometry::BIND_OFF;
    if (fr[0].matchWord("NormalBinding") && Geometry_matchBindingTypeStr(fr[1].getStr(),normalBinding))
    {
        geom.setNormalBinding(normalBinding);
        fr+=2;
        iteratorAdvanced = true;
    }

    if (fr.matchSequence("NormalArray %i {"))
    {
        int entry = fr[0].getNoNestedBrackets();

        int capacity;
        fr[1].getInt(capacity);
        
        Vec3Array* normals = osgNew Vec3Array;
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

    Geometry::AttributeBinding colorBinding=Geometry::BIND_OFF;
    if (fr[0].matchWord("ColorBinding") && Geometry_matchBindingTypeStr(fr[1].getStr(),colorBinding))
    {
        geom.setColorBinding(colorBinding);
        fr+=2;
        iteratorAdvanced = true;
    }

    if (fr.matchSequence("ColorArray %w %i {"))
    {
        ++fr;
        Array* colors = Array_readLocalData(fr);
        if (colors)
        {
            geom.setColorArray(colors);
            iteratorAdvanced = true;
        }
    }

    Geometry::AttributeBinding secondaryColorBinding=Geometry::BIND_OFF;
    if (fr[0].matchWord("SecondaryColorBinding") && Geometry_matchBindingTypeStr(fr[1].getStr(),secondaryColorBinding))
    {
        geom.setSecondaryColorBinding(secondaryColorBinding);
        fr+=2;
        iteratorAdvanced = true;
    }

    if (fr.matchSequence("SecondaryColorArray %w %i {"))
    {
        ++fr;
        Array* colors = Array_readLocalData(fr);
        if (colors)
        {
            geom.setSecondaryColorArray(colors);
            iteratorAdvanced = true;
        }
    }



    Geometry::AttributeBinding fogCoordBinding=Geometry::BIND_OFF;
    if (fr[0].matchWord("FogCoordBinding") && Geometry_matchBindingTypeStr(fr[1].getStr(),fogCoordBinding))
    {
        geom.setFogCoordBinding(fogCoordBinding);
        fr+=2;
        iteratorAdvanced = true;
    }

    if (fr.matchSequence("FogCoordArray %i {"))
    {
        int entry = fr[0].getNoNestedBrackets();

        int capacity;
        fr[1].getInt(capacity);
        
        FloatArray* fogcoords = osgNew FloatArray;
        fogcoords->reserve(capacity);

        fr += 3;

        while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
        {
            float fc;
            if (fr[0].getFloat(fc))
            {
                ++fr;
                fogcoords->push_back(fc);
            }
            else
            {
                ++fr;
            }
        }
        
        geom.setFogCoordArray(fogcoords);
        
        iteratorAdvanced = true;
        ++fr;
    }

    if (fr.matchSequence("TexCoordArray %i %w %i {"))
    {
        int unit=0;
        fr[1].getInt(unit);
    
        fr+=2;
        Array* texcoords = Array_readLocalData(fr);
        if (texcoords)
        {
            geom.setTexCoordArray(unit,texcoords);
            iteratorAdvanced = true;
        }
        
    }

    return iteratorAdvanced;
}


Array* Array_readLocalData(Input& fr)
{
    int entry = fr[0].getNoNestedBrackets();

    const char* arrayName = fr[0].getStr();

    unsigned int capacity = 0;
    fr[1].getUInt(capacity);
    ++fr;

    fr += 2;

    if (strcmp(arrayName,"ByteArray")==0)
    {
        ByteArray* array = osgNew ByteArray;
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
        return array;
    }
    else if (strcmp(arrayName,"ShortArray")==0)
    {
        ShortArray* array = osgNew ShortArray;
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
        return array;
    }
    else if (strcmp(arrayName,"IntArray")==0)
    {
        IntArray* array = osgNew IntArray;
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
        return array;
    }
    else if (strcmp(arrayName,"UByteArray")==0)
    {
        UByteArray* array = osgNew UByteArray;
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
        return array;
    }
    else if (strcmp(arrayName,"UShortArray")==0)
    {
        UShortArray* array = osgNew UShortArray;
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
        return array;
    }
    else if (strcmp(arrayName,"UIntArray")==0)
    {
        UIntArray* array = osgNew UIntArray;
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
        return array;
    }
    else if (strcmp(arrayName,"UByte4Array")==0)
    {
        UByte4Array* array = osgNew UByte4Array;
        array->reserve(capacity);
        while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
        {
            unsigned int r,g,b,a;
            if (fr[0].getUInt(r) &&
                fr[1].getUInt(g) &&
                fr[2].getUInt(b) &&
                fr[3].getUInt(a))
            {
                ++fr;
                array->push_back(osg::UByte4(r,g,b,a));
            }
            else ++fr;
        }
        ++fr;
        return array;
    }
    else if (strcmp(arrayName,"FloatArray")==0)
    {
        FloatArray* array = osgNew FloatArray;
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
        return array;
    }
    else if (strcmp(arrayName,"Vec2Array")==0)
    {
        Vec2Array* array = osgNew Vec2Array;
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
        return array;
    }
    else if (strcmp(arrayName,"Vec3Array")==0)
    {
        Vec3Array* array = osgNew Vec3Array;
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
        return array;
    }
    else if (strcmp(arrayName,"Vec4Array")==0)
    {
        Vec4Array* array = osgNew Vec4Array;
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
        return array;
    }
    
    return 0;
}


template<class Iterator>
void Array_writeLocalData(Output& fw, Iterator first, Iterator last)
{
    fw.indent() << "{"<<std::endl;
    fw.moveIn();
    for(Iterator itr=first;
        itr!=last;
        ++itr)
    {
        fw.indent() << *itr << std::endl;
    }
    fw.moveOut();
    fw.indent()<<"}"<<std::endl;
    
}

bool Array_writeLocalData(const Array& array,Output& fw)
{
    switch(array.getType())
    {
        case(Array::ByteArrayType):
            {
                const ByteArray& carray = static_cast<const ByteArray&>(array);
                fw<<array.className()<<" "<<carray.size()<<std::endl;
                Array_writeLocalData(fw,carray.begin(),carray.end());
                return true;
            }
            break;
        case(Array::ShortArrayType):
            {
                const ShortArray& carray = static_cast<const ShortArray&>(array);
                fw<<array.className()<<" "<<carray.size()<<std::endl;
                Array_writeLocalData(fw,carray.begin(),carray.end());
                return true;
            }
            break;
        case(Array::IntArrayType):
            {
                const IntArray& carray = static_cast<const IntArray&>(array);
                fw<<array.className()<<" "<<carray.size()<<std::endl;
                Array_writeLocalData(fw,carray.begin(),carray.end());
                return true;
            }
            break;
        case(Array::UByteArrayType):
            {
                const UByteArray& carray = static_cast<const UByteArray&>(array);
                fw<<array.className()<<" "<<carray.size()<<std::endl;
                Array_writeLocalData(fw,carray.begin(),carray.end());
                return true;
            }
            break;
        case(Array::UShortArrayType):
            {
                const UShortArray& carray = static_cast<const UShortArray&>(array);
                fw<<array.className()<<" "<<carray.size()<<std::endl;
                Array_writeLocalData(fw,carray.begin(),carray.end());
                return true;
            }
            break;
        case(Array::UIntArrayType):
            {
                const UIntArray& carray = static_cast<const UIntArray&>(array);
                fw<<array.className()<<" "<<carray.size()<<" ";
                Array_writeLocalData(fw,carray.begin(),carray.end());
                return true;
            }
            break;
        case(Array::UByte4ArrayType):
            {
                const UByte4Array& carray = static_cast<const UByte4Array&>(array);
                fw<<array.className()<<" "<<carray.size()<<" ";
                Array_writeLocalData(fw,carray.begin(),carray.end());
                return true;
            }
            break;
        case(Array::FloatArrayType):
            {
                const FloatArray& carray = static_cast<const FloatArray&>(array);
                fw<<array.className()<<" "<<carray.size()<<std::endl;
                Array_writeLocalData(fw,carray.begin(),carray.end());
                return true;
            }
            break;
        case(Array::Vec2ArrayType):
            {
                const Vec2Array& carray = static_cast<const Vec2Array&>(array);
                fw<<array.className()<<" "<<carray.size()<<std::endl;
                Array_writeLocalData(fw,carray.begin(),carray.end());
                return true;
            }
            break;
        case(Array::Vec3ArrayType):
            {
                const Vec3Array& carray = static_cast<const Vec3Array&>(array);
                fw<<array.className()<<" "<<carray.size()<<std::endl;
                Array_writeLocalData(fw,carray.begin(),carray.end());
                return true;
            }
            break;
        case(Array::Vec4ArrayType):
            {
                const Vec4Array& carray = static_cast<const Vec4Array&>(array);
                fw<<array.className()<<" "<<carray.size()<<std::endl;
                Array_writeLocalData(fw,carray.begin(),carray.end());
                return true;
            }
            break;
        case(Array::ArrayType):
        default:
            return false;
    }
}


bool Primitve_readLocalData(Input& fr,osg::Geometry& geom)
{
    bool iteratorAdvanced = false;
    if (fr.matchSequence("DrawArrays %w %i %i"))
    {
        
        GLenum mode;
        Geometry_matchPrimitiveModeStr(fr[1].getStr(),mode);

        int first;
        fr[2].getInt(first);
        
        int count;
        fr[3].getInt(count);

        geom.addPrimitive(osgNew DrawArrays(mode,first,count));

        fr += 4;
        
        iteratorAdvanced = true;
        
    }
    else if (fr.matchSequence("DrawArrayLengths %w %i %i {"))
    {
        int entry = fr[1].getNoNestedBrackets();

        GLenum mode;
        Geometry_matchPrimitiveModeStr(fr[1].getStr(),mode);

        int first;
        fr[2].getInt(first);

        int capacity;
        fr[3].getInt(capacity);
        
        fr += 5;
        
        DrawArrayLengths* prim = osgNew DrawArrayLengths;
        prim->setMode(mode);
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
         
         geom.addPrimitive(prim);
   
        iteratorAdvanced = true;
    }
    else if (fr.matchSequence("DrawElementsUByte %w %i {"))
    {
        int entry = fr[1].getNoNestedBrackets();

        GLenum mode;
        Geometry_matchPrimitiveModeStr(fr[1].getStr(),mode);

        int capacity;
        fr[2].getInt(capacity);
        
        fr += 4;
        
        DrawElementsUByte* prim = osgNew DrawElementsUByte;
        prim->setMode(mode);
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
         
         geom.addPrimitive(prim);
   
        iteratorAdvanced = true;
    }
    else if (fr.matchSequence("DrawElementsUShort %w %i {"))
    {
        int entry = fr[1].getNoNestedBrackets();

        GLenum mode;
        Geometry_matchPrimitiveModeStr(fr[1].getStr(),mode);

        int capacity;
        fr[2].getInt(capacity);
        
        fr += 4;
        
        DrawElementsUShort* prim = osgNew DrawElementsUShort;
        prim->setMode(mode);
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
         
         geom.addPrimitive(prim);
   
        iteratorAdvanced = true;
    }
    else if (fr.matchSequence("DrawElementsUInt %w %i {"))
    {
        int entry = fr[1].getNoNestedBrackets();

        GLenum mode;
        Geometry_matchPrimitiveModeStr(fr[1].getStr(),mode);

        int capacity;
        fr[2].getInt(capacity);
        
        fr += 4;
        
        DrawElementsUInt* prim = osgNew DrawElementsUInt;
        prim->setMode(mode);
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
         
         geom.addPrimitive(prim);
   
        iteratorAdvanced = true;
    }

    return iteratorAdvanced;
}

bool Primitve_writeLocalData(const Primitive& prim,Output& fw)
{

    switch(prim.getType())
    {
        case(Primitive::DrawArraysPrimitiveType):
            {
                const DrawArrays& cprim = static_cast<const DrawArrays&>(prim);
                fw<<cprim.className()<<" "<<Geometry_getPrimitiveModeStr(cprim.getMode())<<" "<<cprim.getFirst()<<" "<<cprim.getCount()<<std::endl;
                return true;
            }
            break;
        case(Primitive::DrawArrayLengthsPrimitiveType):
            {
                const DrawArrayLengths& cprim = static_cast<const DrawArrayLengths&>(prim);
                fw<<cprim.className()<<" "<<Geometry_getPrimitiveModeStr(cprim.getMode())<<" "<<cprim.getFirst()<<" "<<cprim.size()<<std::endl;
                Array_writeLocalData(fw,cprim.begin(),cprim.end());
                return true;
            }
            break;
        case(Primitive::DrawElementsUBytePrimitiveType):
            {
                const DrawElementsUByte& cprim = static_cast<const DrawElementsUByte&>(prim);
                fw<<cprim.className()<<" "<<Geometry_getPrimitiveModeStr(cprim.getMode())<<" "<<cprim.size()<<std::endl;
                Array_writeLocalData(fw,cprim.begin(),cprim.end());
                return true;
            }
            break;
        case(Primitive::DrawElementsUShortPrimitiveType):
            {
                const DrawElementsUShort& cprim = static_cast<const DrawElementsUShort&>(prim);
                fw<<cprim.className()<<" "<<Geometry_getPrimitiveModeStr(cprim.getMode())<<" "<<cprim.size()<<std::endl;
                Array_writeLocalData(fw,cprim.begin(),cprim.end());
                return true;
            }
            break;
        case(Primitive::DrawElementsUIntPrimitiveType):
            {
                const DrawElementsUInt& cprim = static_cast<const DrawElementsUInt&>(prim);
                fw<<cprim.className()<<" "<<Geometry_getPrimitiveModeStr(cprim.getMode())<<" "<<cprim.size()<<std::endl;
                Array_writeLocalData(fw,cprim.begin(),cprim.end());
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

    const Geometry::PrimitiveList& primitives = geom.getPrimitiveList();
    if (!primitives.empty())
    {
        fw.indent() << "Primitives "<<primitives.size()<<std::endl;
        fw.indent() << "{"<<std::endl;
        fw.moveIn();
        for(Geometry::PrimitiveList::const_iterator itr=primitives.begin();
            itr!=primitives.end();
            ++itr)
        {
            fw.indent();
            Primitve_writeLocalData(**itr,fw);
        }
        fw.moveOut();
        fw.indent() << "}"<<std::endl;
    }

    if (geom.getVertexArray())
    {
        const Vec3Array& vertices = *geom.getVertexArray();
        fw.indent()<<"VertexArray "<<vertices.size()<<std::endl;
        
        Array_writeLocalData(fw,vertices.begin(),vertices.end());
        
    }

    if (geom.getNormalArray())
    {
        
        fw.indent()<<"NormalBinding "<<Geometry_getBindingTypeStr(geom.getNormalBinding())<<std::endl;
        
        const Vec3Array& normals = *geom.getNormalArray();
        fw.indent()<<"NormalArray "<<normals.size()<<std::endl;
        
        Array_writeLocalData(fw,normals.begin(),normals.end());
        
    }

    if (geom.getColorArray())
    {
        fw.indent()<<"ColorBinding "<<Geometry_getBindingTypeStr(geom.getColorBinding())<<std::endl;
        fw.indent()<<"ColorArray ";
        Array_writeLocalData(*geom.getColorArray(),fw);
    }

    if (geom.getSecondaryColorArray())
    {
        fw.indent()<<"SecondaryColorBinding "<<Geometry_getBindingTypeStr(geom.getSecondaryColorBinding())<<std::endl;
        fw.indent()<<"SecondaryColorArray ";
        Array_writeLocalData(*geom.getSecondaryColorArray(),fw);
    }

    if (geom.getFogCoordArray())
    {
        fw.indent()<<"FogCoordBinding "<<Geometry_getBindingTypeStr(geom.getFogCoordBinding())<<std::endl;

        const FloatArray& fogcoords = *geom.getFogCoordArray();
        fw.indent()<<"FogCoordArray "<<fogcoords.size()<<std::endl;
        
        Array_writeLocalData(fw,fogcoords.begin(),fogcoords.end());
    }

    const Geometry::TexCoordArrayList& tcal=geom.getTexCoordArrayList();
    for(unsigned int i=0;i<tcal.size();++i)
    {
        if (tcal[i].valid())
        {
            fw.indent()<<"TexCoordArray "<<i<<" ";
            Array_writeLocalData(*(tcal[i]),fw);
        }
    }
    

    return true;
}

bool Geometry_matchBindingTypeStr(const char* str,Geometry::AttributeBinding& mode)
{
    if (strcmp(str,"OFF")==0) mode = Geometry::BIND_OFF;
    else if (strcmp(str,"OVERALL")==0) mode = Geometry::BIND_OVERALL;
    else if (strcmp(str,"PER_PRIMITIVE")==0) mode = Geometry::BIND_PER_PRIMITIVE;
    else if (strcmp(str,"PER_VERTEX")==0) mode = Geometry::BIND_PER_VERTEX;
    else return false;
    return true;
}


const char* Geometry_getBindingTypeStr(Geometry::AttributeBinding mode)
{
    switch(mode)
    {
        case (Geometry::BIND_OVERALL)       : return "OVERALL";
        case (Geometry::BIND_PER_PRIMITIVE) : return "PER_PRIMITIVE";
        case (Geometry::BIND_PER_VERTEX)    : return "PER_VERTEX";
        case (Geometry::BIND_OFF)           :
        default                             : return "OFF";
    }
}

bool Geometry_matchPrimitiveModeStr(const char* str,GLenum& mode)
{
    if      (strcmp(str,"POINTS")==0)           mode = Primitive::POINTS;
    else if (strcmp(str,"LINES")==0)            mode = Primitive::LINES;
    else if (strcmp(str,"LINE_STRIP")==0)       mode = Primitive::LINE_STRIP;
    else if (strcmp(str,"LINE_LOOP")==0)        mode = Primitive::LINE_LOOP;
    else if (strcmp(str,"TRIANGLES")==0)        mode = Primitive::TRIANGLES;
    else if (strcmp(str,"TRIANGLE_STRIP")==0)   mode = Primitive::TRIANGLE_STRIP;
    else if (strcmp(str,"TRIANGLE_FAN")==0)     mode = Primitive::TRIANGLE_FAN;
    else if (strcmp(str,"QUADS")==0)            mode = Primitive::QUADS;
    else if (strcmp(str,"QUAD_STRIP")==0)       mode = Primitive::QUAD_STRIP;
    else if (strcmp(str,"POLYGON")==0)          mode = Primitive::POLYGON;
    else return false;
    return true;
}


const char* Geometry_getPrimitiveModeStr(GLenum mode)
{
    switch(mode)
    {
        case (Primitive::POINTS)            : return "POINTS";
        case (Primitive::LINES)             : return "LINES";
        case (Primitive::LINE_STRIP)        : return "LINE_STRIP";
        case (Primitive::LINE_LOOP)         : return "LINE_LOOP";
        case (Primitive::TRIANGLES)         : return "TRIANGLES";
        case (Primitive::TRIANGLE_STRIP)    : return "TRIANGLE_STRIP";
        case (Primitive::TRIANGLE_FAN)      : return "TRIANGLE_FAN";
        case (Primitive::QUADS)             : return "QUADS";
        case (Primitive::QUAD_STRIP)        : return "QUAD_STRIP";
        case (Primitive::POLYGON)           : return "POLYGON";
        default                             : return "UnknownPrimitveType";
    }
}
