#if defined(_MSC_VER)
	#pragma warning( disable : 4786 )
#endif

#include "GeoSet.h"
#include "osg/Notify"

#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/ParameterOutput"

using namespace osg;
using namespace osgDB;

// forward declare functions to use later.
bool GeoSet_readLocalData(Object& obj, Input& fr);
bool GeoSet_writeLocalData(const Object& obj, Output& fw);

bool GeoSet_readIndexData(Input& fr, const char* IndexName, GeoSet::IndexPointer& ip, bool& useCIndex);
bool GeoSet_writeIndexData(Output& fw, const char* IndexName,const GeoSet::IndexPointer& ip);
bool GeoSet_matchBindingTypeStr(const char* str,GeoSet::BindingType& mode);
const char* GeoSet_getBindingTypeStr(GeoSet::BindingType mode);
const char* GeoSet_getInterleavedRowComposition(GeoSet::InterleaveArrayType at);
int GeoSet_getInterleavedRowLength(GeoSet::InterleaveArrayType at);

// register the read and write functions with the osgDB::Registry.
RegisterDotOsgWrapperProxy g_GeoSetFuncProxy
(
    new osg::GeoSet,
    "GeoSet",
    "Object Drawable GeoSet",
    &GeoSet_readLocalData,
    &GeoSet_writeLocalData,
    DotOsgWrapper::READ_AND_WRITE
);

// register the old style 'Geoset' keyword read and write functions with the osgDB::Registry.
RegisterDotOsgWrapperProxy g_GeosetFuncProxy
(
    new osg::GeoSet,
    "Geoset",
    "Object Drawable Geoset",
    &GeoSet_readLocalData,
    NULL,
    DotOsgWrapper::READ_ONLY
);

bool GeoSet_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    GeoSet& geoset = static_cast<GeoSet&>(obj);

    Vec3* coordList = NULL;
    bool coordIndexUseCIndex = false;
    GeoSet::IndexPointer coordIndex;
    Vec3* normalList = NULL;
    bool normIndexUseCIndex = false;
    GeoSet::IndexPointer normIndex;
    Vec4* colorList = NULL;
    bool colIndexUseCIndex = false;
    GeoSet::IndexPointer colIndex;
    Vec2* textureList = NULL;
    bool tIndexUseCIndex = false;
    GeoSet::IndexPointer tIndex;
    float* interleavedArray = NULL;
    bool iaIndexUseCIndex = false;
    GeoSet::IndexPointer iaIndex;

    GeoSet::BindingType bind=GeoSet::BIND_OFF;
    GeoSet::BindingType normal_bind=GeoSet::BIND_OFF;
    GeoSet::BindingType color_bind=GeoSet::BIND_OFF;
    GeoSet::BindingType texture_bind=GeoSet::BIND_OFF;
    GeoSet::InterleaveArrayType iaType = GeoSet::IA_OFF;

    int start_indent = fr[0].getNoNestedBrackets();
    while (!fr.eof() && fr[0].getNoNestedBrackets()>=start_indent)
    {

        bool fieldAdvanced = false;

        bool readPrimitiveLengths = false;
        if (fr.matchSequence("tstrips %i {"))
        {
            readPrimitiveLengths = true;
            geoset.setPrimType(GeoSet::TRIANGLE_STRIP);
        }
        else if (fr.matchSequence("flat_tstrips %i {"))
        {
            readPrimitiveLengths = true;
            geoset.setPrimType(GeoSet::FLAT_TRIANGLE_STRIP);
        }
        else if (fr.matchSequence("polys %i {"))
        {
            readPrimitiveLengths = true;
            geoset.setPrimType(GeoSet::POLYGON);
        }
        else if (fr.matchSequence("quadstrip %i {"))
        {
            readPrimitiveLengths = true;
            geoset.setPrimType(GeoSet::QUAD_STRIP);
        }
        else if (fr.matchSequence("lineloops %i {"))
        {
            readPrimitiveLengths = true;
            geoset.setPrimType(GeoSet::LINE_LOOP);
        }
        else if (fr.matchSequence("linestrip %i {"))
        {
            readPrimitiveLengths = true;
            geoset.setPrimType(GeoSet::LINE_STRIP);
        }
        else if (fr.matchSequence("flat_linestrip %i {"))
        {
            readPrimitiveLengths = true;
            geoset.setPrimType(GeoSet::FLAT_LINE_STRIP);
        }
        else if (fr.matchSequence("tfans %i {"))
        {
            readPrimitiveLengths = true;
            geoset.setPrimType(GeoSet::TRIANGLE_FAN);
        }
        else if (fr.matchSequence("flat_tfans %i {"))
        {
            readPrimitiveLengths = true;
            geoset.setPrimType(GeoSet::FLAT_TRIANGLE_FAN);
        }

        if (readPrimitiveLengths)
        {

            int entry = fr[1].getNoNestedBrackets();
            fr += 3;

            int capacity;
            if (!fr[1].getInt(capacity)) capacity=100;
            int size = 0;
            int* list = new int [capacity];
            memset(list,0,capacity*sizeof(int));
            while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
            {
                int primLength;

                if (fr[0].getInt(primLength))
                {

                    if (size>=capacity)
                    {
                        int oldCapacity = capacity;
                        while(capacity<=size) capacity *= 2;
                        int* oldList = list;
                        list = new int[capacity];
                        memset(list,0,capacity*sizeof(int));
                        for(int i=0;i<oldCapacity;++i)
                        {
                            list[i] = oldList[i];
                        }
                        delete [] oldList;
                    }
                    list[size] = primLength;
                    ++size;

                }
                ++fr;
            }

            fieldAdvanced = true;
            ++fr;

            geoset.setNumPrims(size);
            geoset.setPrimLengths(list);
        }

        if (fr.matchSequence("lines %i"))
        {
            geoset.setPrimType(GeoSet::LINES);
            int noLines;
            if (fr[1].getInt(noLines)) geoset.setNumPrims(noLines);
            fieldAdvanced = true;
            fr+=2;
        }

        if (fr.matchSequence("triangles %i"))
        {
            geoset.setPrimType(GeoSet::TRIANGLES);
            int noTriangles;
            if (fr[1].getInt(noTriangles)) geoset.setNumPrims(noTriangles);
            fieldAdvanced = true;
            fr+=2;
        }

        if (fr.matchSequence("quads %i"))
        {
            geoset.setPrimType(GeoSet::QUADS);
            int noQuads;
            if (fr[1].getInt(noQuads)) geoset.setNumPrims(noQuads);
            fieldAdvanced = true;
            fr+=2;
        }

        if (fr.matchSequence("points %i"))
        {
            geoset.setPrimType(GeoSet::POINTS);
            int noPoints;
            if (fr[1].getInt(noPoints)) geoset.setNumPrims(noPoints);
            fieldAdvanced = true;
            fr+=2;
        }

        bool matchFirst = false;
        if ((matchFirst=fr.matchSequence("Coords {")) || fr.matchSequence("Coords %i {"))
        {

            // set up coordinates.
            int entry = fr[0].getNoNestedBrackets();

            int capacity = 100;
            if (matchFirst)
            {
                fr += 2;
            }
            else
            {
                fr[1].getInt(capacity);
                fr += 3;
            }

            int size = 0;
            coordList = new Vec3[capacity];

            while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
            {
                float x,y,z;
                if (fr[0].getFloat(x) && fr[1].getFloat(y) && fr[2].getFloat(z))
                {
                    fr += 3;

                    if (size>=capacity)
                    {
                        int oldCapacity = capacity;
                        while(capacity<=size) capacity *= 2;
                        Vec3* oldList = coordList;
                        coordList = new Vec3[capacity];
                        for(int i=0;i<oldCapacity;++i)
                        {
                            coordList[i][0] = oldList[i][0];
                            coordList[i][1] = oldList[i][1];
                            coordList[i][2] = oldList[i][2];
                        }
                        delete [] oldList;
                    }
                    coordList[size][0] = x;
                    coordList[size][1] = y;
                    coordList[size][2] = z;
                    ++size;

                }
                else
                {
                    ++fr;
                }
            }

            fieldAdvanced = true;
            ++fr;

        }

        if (GeoSet_readIndexData(fr, "CoordIndex" ,coordIndex, coordIndexUseCIndex))
        {
            fieldAdvanced = true;
        }

        if (fr[0].matchWord("Normal_Binding") && GeoSet_matchBindingTypeStr(fr[1].getStr(),bind))
        {
            normal_bind = bind;
            fr+=2;
            iteratorAdvanced = true;
        }

        if ((matchFirst=fr.matchSequence("Normals {")) || fr.matchSequence("Normals %i {"))
        {

            // set up normalinates.
            int entry = fr[0].getNoNestedBrackets();

            int capacity = 100;
            if (matchFirst)
            {
                fr += 2;
            }
            else
            {
                fr[1].getInt(capacity);
                fr += 3;
            }

            int size = 0;
            normalList = new Vec3[capacity];

            while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
            {
                float x,y,z;
                if (fr[0].getFloat(x) && fr[1].getFloat(y) && fr[2].getFloat(z))
                {
                    fr += 3;

                    if (size>=capacity)
                    {
                        int oldCapacity = capacity;
                        while(capacity<=size) capacity *= 2;
                        Vec3* oldList = normalList;
                        normalList = new Vec3[capacity];
                        for(int i=0;i<oldCapacity;++i)
                        {
                            normalList[i][0] = oldList[i][0];
                            normalList[i][1] = oldList[i][1];
                            normalList[i][2] = oldList[i][2];
                        }
                        delete [] oldList;
                    }
                    normalList[size][0] = x;
                    normalList[size][1] = y;
                    normalList[size][2] = z;
                    ++size;

                }
                else
                {
                    ++fr;
                }
            }

            fieldAdvanced = true;
            ++fr;
        }

        if (GeoSet_readIndexData(fr, "NIndex" ,normIndex, normIndexUseCIndex))
        {
            fieldAdvanced = true;
        }

        if (fr[0].matchWord("Color_Binding") && GeoSet_matchBindingTypeStr(fr[1].getStr(),bind))
        {
            color_bind = bind;
            fr+=2;
            iteratorAdvanced = true;
        }

        if ((matchFirst=fr.matchSequence("Colors {")) || fr.matchSequence("Colors %i {"))
        {

            // set up coordinates.
            int entry = fr[0].getNoNestedBrackets();

            int capacity = 100;
            if (matchFirst)
            {
                fr += 2;
            }
            else
            {
                fr[1].getInt(capacity);
                fr += 3;
            }

            int size = 0;
            colorList = new Vec4[capacity];

            while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
            {
                float r,g,b,a;
                if (fr[0].getFloat(r) && fr[1].getFloat(g) && fr[2].getFloat(b) && fr[3].getFloat(a))
                {

                    fr += 4;

                    if (size>=capacity)
                    {
                        int oldCapacity = capacity;
                        while(capacity<=size) capacity *= 2;
                        Vec4* oldList = colorList;
                        colorList = new Vec4[capacity];
                        for(int i=0;i<oldCapacity;++i)
                        {
                            colorList[i][0] = oldList[i][0];
                            colorList[i][1] = oldList[i][1];
                            colorList[i][2] = oldList[i][2];
                            colorList[i][3] = oldList[i][3];
                        }
                        delete [] oldList;
                    }

                    colorList[size][0] = r;
                    colorList[size][1] = g;
                    colorList[size][2] = b;
                    colorList[size][3] = a;
                    ++size;

                }
                else
                {
                    ++fr;
                }
            }

            fieldAdvanced = true;
            ++fr;

        }

        if (GeoSet_readIndexData(fr, "ColIndex" ,colIndex, colIndexUseCIndex))
        {
            fieldAdvanced = true;
        }

        if (fr[0].matchWord("Texture_Binding") && GeoSet_matchBindingTypeStr(fr[1].getStr(),bind))
        {
            texture_bind = bind;
            fr+=2;
            iteratorAdvanced = true;
        }

        if ((matchFirst=fr.matchSequence("TCoords {")) || fr.matchSequence("TCoords %i {"))
        {

            // set up coordinates.
            int entry = fr[0].getNoNestedBrackets();

            int capacity = 100;
            if (matchFirst)
            {
                fr += 2;
            }
            else
            {
                fr[1].getInt(capacity);
                fr += 3;
            }

            int size = 0;
            textureList = new Vec2[capacity];

            while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
            {
                float r,s;
                if (fr[0].getFloat(r) && fr[1].getFloat(s))
                {
                    fr += 2;
                    if (size>=capacity)
                    {
                        int oldCapacity = capacity;
                        while(capacity<=size) capacity *= 2;
                        Vec2* oldList = textureList;
                        textureList = new Vec2[capacity];
                        for(int i=0;i<oldCapacity;++i)
                        {
                            textureList[i][0] = oldList[i][0];
                            textureList[i][1] = oldList[i][1];
                        }
                        delete [] oldList;
                    }
                    textureList[size][0] = r;
                    textureList[size][1] = s;
                    ++size;

                }
                else
                {
                    ++fr;
                }
            }

            fieldAdvanced = true;
            ++fr;

        }

        if (GeoSet_readIndexData(fr, "TIndex" ,tIndex, tIndexUseCIndex))
        {
            fieldAdvanced = true;
        }
        
        if ((matchFirst=fr.matchSequence("InterleavedArray %w {")) ||
            fr.matchSequence("InterleavedArray %w %i {"))
        {

            // set up coordinates.
            int entry = fr[0].getNoNestedBrackets();

            const char* type = fr[1].getStr();
            if (strcmp(type,"IA_OFF")==0) iaType = GeoSet::IA_OFF;
            else if (strcmp(type,"IA_V2F")==0) iaType =GeoSet::IA_V2F;
            else if (strcmp(type,"IA_V3F")==0) iaType =GeoSet::IA_V3F;
            else if (strcmp(type,"IA_C4UB_V2F")==0) iaType =GeoSet::IA_C4UB_V2F;
            else if (strcmp(type,"IA_C4UB_V3F")==0) iaType =GeoSet::IA_C4UB_V3F;
            else if (strcmp(type,"IA_C3F_V3F")==0) iaType =GeoSet::IA_C3F_V3F;
            else if (strcmp(type,"IA_N3F_V3F")==0) iaType =GeoSet::IA_N3F_V3F;
            else if (strcmp(type,"IA_C4F_N3F_V3F")==0) iaType =GeoSet::IA_C4F_N3F_V3F;
            else if (strcmp(type,"IA_T2F_V3F")==0) iaType =GeoSet::IA_T2F_V3F;
            else if (strcmp(type,"IA_T4F_V4F")==0) iaType =GeoSet::IA_T4F_V4F;
            else if (strcmp(type,"IA_T2F_C4UB_V3F")==0) iaType =GeoSet::IA_T2F_C4UB_V3F;
            else if (strcmp(type,"IA_T2F_C3F_V3F")==0) iaType =GeoSet::IA_T2F_C3F_V3F;
            else if (strcmp(type,"IA_T2F_N3F_V3F")==0) iaType =GeoSet::IA_T2F_N3F_V3F;
            else if (strcmp(type,"IA_T2F_C4F_N3F_V3F")==0) iaType =GeoSet::IA_T2F_C4F_N3F_V3F;
            else if (strcmp(type,"IA_T4F_C4F_N3F_V4F")==0) iaType =GeoSet::IA_T4F_C4F_N3F_V4F;
            else
            {
                iaType = GeoSet::IA_OFF;
            }

            int capacity = 100;
            if (matchFirst)
            {
                fr += 3;
            }
            else
            {
                fr[2].getInt(capacity);
                fr += 4;
            }

            if (iaType == GeoSet::IA_OFF)
            {
                // no data should be read - read over {}.
                interleavedArray = NULL;
                while (!fr.eof() && fr[0].getNoNestedBrackets()>entry) ++fr;
            }
            else
            {
                // now read the data rows between the {}.
                const char* rowComp = GeoSet_getInterleavedRowComposition(iaType);
                int rowLength = GeoSet_getInterleavedRowLength(iaType);
                
                int size = 0;
                unsigned char* dataList = new unsigned char[capacity*rowLength];

                unsigned char* rowData = new unsigned char [rowLength];

                float floatData;
                int intData;
                while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
                {

                    unsigned char* itrRowData = rowData;
                    const char* itrRowComp = rowComp;
                    int rn = 0;
                    while (*itrRowComp!=0 && !fr.eof() && fr[0].getNoNestedBrackets()>entry)
                    {
                        if (*itrRowComp=='f')
                        {
                            if (!fr[rn].getFloat(floatData)) break;
                            *(float*)itrRowData = floatData;
                            itrRowData += 4;
                        }
                        else
                        {
                            if (!fr[rn].getInt(intData)) break;
                            *itrRowData = (unsigned char)intData;
                            itrRowData += 1;
                        }
                        ++itrRowComp;
                        ++rn;
                    }
                    if (*itrRowComp==0)
                    {
                        fr += rn;
                        if (size>=capacity)
                        {
                            int oldCapacity = capacity;
                            while(capacity<=size) capacity *= 2;
                            unsigned char* oldList = dataList;
                            dataList = new unsigned char[capacity*rowLength];
                            memcpy(dataList,oldList,oldCapacity*rowLength);
                            delete [] oldList;
                        }
                        memcpy(dataList+size*rowLength,rowData,rowLength);
                        ++size;

                    }
                    else
                    {
                        if (!fr.eof() && fr[0].getNoNestedBrackets()>entry) ++fr;
                    }
                }

                delete [] rowData;

                interleavedArray = (float*)dataList;
            }

            fieldAdvanced = true;
            ++fr;

        }

        if (GeoSet_readIndexData(fr, "InterleavedArrayIndex" ,iaIndex, iaIndexUseCIndex))
        {
            fieldAdvanced = true;
        }

        if (!fieldAdvanced)
        {
            if (fr[0].getNoNestedBrackets()>start_indent) fr.advanceToEndOfBlock(start_indent+1);
            else ++fr;
        }
        iteratorAdvanced = true;

    }

    // set up the coord lists.
    if (coordList)
    {
        geoset.setCoords(coordList,coordIndex);
    }

    // set up the normal lists.
    if (normalList)
    {
        geoset.setNormalBinding(normal_bind);
        if (normIndexUseCIndex) geoset.setNormals(normalList,coordIndex);
        else geoset.setNormals(normalList,normIndex);

    } else geoset.setNormalBinding(GeoSet::BIND_OFF);

    // set up the color lists.
    if (colorList)
    {
        geoset.setColorBinding(color_bind);
        if (colIndexUseCIndex) geoset.setColors(colorList,coordIndex);
        else geoset.setColors(colorList,colIndex);

    } else geoset.setColorBinding(GeoSet::BIND_OFF);

    if (textureList)
    {
        geoset.setTextureBinding(texture_bind);
        if (tIndexUseCIndex) geoset.setTextureCoords(textureList,coordIndex);
        else geoset.setTextureCoords(textureList,tIndex);
        
    } else geoset.setTextureBinding(GeoSet::BIND_OFF);

    if (interleavedArray)
    {
        if (iaIndexUseCIndex) geoset.setInterleavedArray(iaType,interleavedArray,coordIndex);
        else geoset.setInterleavedArray(iaType,interleavedArray,iaIndex);

    };

    return iteratorAdvanced;
}


bool GeoSet_writeLocalData(const Object& obj, Output& fw)
{
    int i;

    const GeoSet& geoset = static_cast<const GeoSet&>(obj);

    // write out primitives.
    bool writeOutPrimitiveLengths = false;
    switch(geoset.getPrimType())
    {
        case (GeoSet::TRIANGLE_STRIP):
            fw.indent()<<"tstrips "<< geoset.getNumPrims() << std::endl;
            writeOutPrimitiveLengths = true;
            break;
        case (GeoSet::FLAT_TRIANGLE_STRIP):
            fw.indent()<<"flat_tstrips "<< geoset.getNumPrims() << std::endl;
            writeOutPrimitiveLengths = true;
            break;
        case (GeoSet::POLYGON):
            fw.indent()<<"polys "<< geoset.getNumPrims() << std::endl;
            writeOutPrimitiveLengths = true;
            break;
        case (GeoSet::QUAD_STRIP):
            fw.indent()<<"quadstrip "<< geoset.getNumPrims() << std::endl;
            writeOutPrimitiveLengths = true;
            break;
        case (GeoSet::LINE_LOOP):
            fw.indent()<<"lineloops "<< geoset.getNumPrims() << std::endl;
            writeOutPrimitiveLengths = true;
            break;
        case (GeoSet::LINE_STRIP):
            fw.indent()<<"linestrip "<< geoset.getNumPrims() << std::endl;
            writeOutPrimitiveLengths = true;
            break;
        case (GeoSet::FLAT_LINE_STRIP):
            fw.indent()<<"flat_linestrip "<< geoset.getNumPrims() << std::endl;
            writeOutPrimitiveLengths = true;
            break;
        case (GeoSet::TRIANGLE_FAN):
            fw.indent()<<"tfans "<< geoset.getNumPrims() << std::endl;
            writeOutPrimitiveLengths = true;
        case (GeoSet::FLAT_TRIANGLE_FAN):
            fw.indent()<<"flat_tfans "<< geoset.getNumPrims() << std::endl;
            writeOutPrimitiveLengths = true;
            break;
        case (GeoSet::LINES):
            fw.indent()<<"lines "<< geoset.getNumPrims() << std::endl;
            writeOutPrimitiveLengths = false;
            break;
        case (GeoSet::TRIANGLES):
            fw.indent()<<"triangles "<< geoset.getNumPrims() << std::endl;
            writeOutPrimitiveLengths = false;
            break;
        case (GeoSet::QUADS):
            fw.indent()<<"quads "<< geoset.getNumPrims() << std::endl;
            writeOutPrimitiveLengths = false;
            break;
        case (GeoSet::POINTS) :
            fw.indent()<<"points "<< geoset.getNumPrims() << std::endl;
            break;
        default:
            notify(WARN) << "GeoSet::writeLocalData() - unhandled primitive type = "<<(int)geoset.getPrimType()<< std::endl;
    }
    if (writeOutPrimitiveLengths)
    {
        writeArray(fw,geoset.getPrimLengths(),geoset.getPrimLengths()+geoset.getNumPrims());
    }

    GeoSet& non_const_geoset = const_cast<GeoSet&>(geoset);
    non_const_geoset.computeNumVerts();

    if (geoset.getCoords())
    {
        // write out _coords.
        fw.indent() << "Coords " << geoset.getNumCoords()<< std::endl;
        fw.indent() << "{"<< std::endl;
        fw.moveIn();
        const Vec3* coords = geoset.getCoords();
        for(i=0;i<geoset.getNumCoords();++i)
        {
            fw.indent() << coords[i][0] << ' ' << coords[i][1] << ' ' << coords[i][2] << std::endl;
        }
        fw.moveOut();
        fw.indent()<<"}"<< std::endl;
    }

    if (geoset.getCoordIndices()._size)
    {
        GeoSet_writeIndexData(fw,"CoordIndex",geoset.getCoordIndices());
    }

    if (geoset.getNormals())
    {
        // write out _normals.
        fw.indent() << "Normal_Binding "<<GeoSet_getBindingTypeStr(geoset.getNormalBinding())<< std::endl;

        fw.indent() << "Normals " << geoset.getNumNormals()<< std::endl;
        fw.indent() << "{"<< std::endl;
        fw.moveIn();
        const Vec3* norms = geoset.getNormals();
        for(i=0;i<geoset.getNumNormals();++i)
        {
            fw.indent() << norms[i][0] << ' ' << norms[i][1] << ' ' << norms[i][2] << std::endl;
        }

        fw.moveOut();
        fw.indent()<<"}"<< std::endl;
    }
    if (geoset.getNormalIndices()._size)
    {
        if (geoset.getNormalIndices()==geoset.getCoordIndices())
        {
            fw.indent() << "NIndex Use_CIndex"<< std::endl;
        }
        else
        {
            GeoSet_writeIndexData(fw,"NIndex",geoset.getNormalIndices());
        }

    }

    if (geoset.getColors())
    {
        // write out _colors.
        fw.indent() << "Color_Binding "<<GeoSet_getBindingTypeStr(geoset.getColorBinding())<< std::endl;

        fw.indent() << "Colors " << geoset.getNumColors()<< std::endl;
        fw.indent() << "{"<< std::endl;
        fw.moveIn();
        const Vec4* colors = geoset.getColors();
        for(i=0;i<geoset.getNumColors();++i)
        {
            fw.indent() << colors[i][0] << ' ' << colors[i][1] << ' ' << colors[i][2] << ' ' << colors[i][3] << std::endl;
        }
        fw.moveOut();
        fw.indent()<<"}"<< std::endl;
    }
    if (geoset.getColorIndices()._size)
    {
        if (geoset.getColorIndices()==geoset.getCoordIndices())
        {
            fw.indent() << "ColIndex Use_CIndex"<< std::endl;
        }
        else
        {
            GeoSet_writeIndexData(fw,"ColIndex",geoset.getColorIndices());
        }
    }

    if (geoset.getTextureCoords())
    {
        // write out _tcoords.
        fw.indent() << "Texture_Binding "<<GeoSet_getBindingTypeStr(geoset.getTextureBinding())<< std::endl;

        fw.indent() << "TCoords " << geoset.getNumTextureCoords()<< std::endl;
        fw.indent() << "{"<< std::endl;
        fw.moveIn();
        const Vec2* tcoords = geoset.getTextureCoords();
        for(i=0;i<geoset.getNumTextureCoords();++i)
        {
            fw.indent() << tcoords[i][0] << ' ' << tcoords[i][1] << std::endl;
        }
        fw.moveOut();
        fw.indent()<<"}"<< std::endl;
    }
    if (geoset.getTextureIndices()._size)
    {
        if (geoset.getTextureIndices()==geoset.getCoordIndices())
        {
            fw.indent() << "TIndex Use_CIndex"<< std::endl;
        }
        else
        {
            GeoSet_writeIndexData(fw,"TIndex",geoset.getTextureIndices());
        }

    }

    if (geoset.getInterleavedArray())
    {
        // write out the interleaved arrays.
        const char* rowComp = GeoSet_getInterleavedRowComposition(geoset.getInterleavedFormat());

        fw.indent() << "InterleavedArray ";
        switch(geoset.getInterleavedFormat())
        {
            case(GeoSet::IA_OFF): fw << "IA_OFF"; break;
            case(GeoSet::IA_V2F): fw << "IA_V2F"; break;
            case(GeoSet::IA_V3F): fw << "IA_V3F"; break;
            case(GeoSet::IA_C4UB_V2F): fw << "IA_C4UB_V2F"; break;
            case(GeoSet::IA_C4UB_V3F): fw << "IA_C4UB_V3F"; break;
            case(GeoSet::IA_C3F_V3F): fw << "IA_C3F_V3F"; break;
            case(GeoSet::IA_N3F_V3F): fw << "IA_N3F_V3F"; break;
            case(GeoSet::IA_C4F_N3F_V3F): fw << "IA_C4F_N3F_V3F"; break;
            case(GeoSet::IA_T2F_V3F): fw << "IA_T2F_V3F"; break;
            case(GeoSet::IA_T4F_V4F): fw << "IA_T4F_V4F"; break;
            case(GeoSet::IA_T2F_C4UB_V3F): fw << "IA_T2F_C4UB_V3F"; break;
            case(GeoSet::IA_T2F_C3F_V3F): fw << "IA_T2F_C3F_V3F"; break;
            case(GeoSet::IA_T2F_N3F_V3F): fw << "IA_T2F_N3F_V3F"; break;
            case(GeoSet::IA_T2F_C4F_N3F_V3F): fw << "IA_T2F_C4F_N3F_V3F"; break;
            case(GeoSet::IA_T4F_C4F_N3F_V4F): fw << "IA_T4F_C4F_N3F_V4F"; break;
            default: fw << "IA_OFF"; break;
        }

        fw << " " <<  geoset.getNumInterleavedCoords()<< std::endl;
        fw.indent() << "{"<< std::endl;
        fw.moveIn();

        const unsigned char* itrRowData = (const unsigned char*)geoset.getInterleavedArray();
        for(i=0;i<geoset.getNumInterleavedCoords();++i)
        {
            fw.indent();

            const char* itrRowComp = rowComp;
            while (*itrRowComp!=0)
            {
                if (*itrRowComp=='f')
                {
                    fw << *(float*)itrRowData<<" ";
                    itrRowData += 4;
                }
                else
                {
                    fw << (int)*itrRowData<<" ";
                    itrRowData += 1;
                }
                itrRowComp++;
            }
            fw << std::endl			;

        }
        fw.moveOut();
        fw.indent()<<"}"<< std::endl;

    }

    return true;
}

bool GeoSet_readIndexData(Input& fr, const char* IndexName, GeoSet::IndexPointer& ip, bool& useCIndex)
{
    if (!fr[0].matchWord(IndexName)) return false;
    
    bool fieldAdvanced = true;
    
    if (fr[1].matchWord("Use_CIndex"))
    {
        fr += 2;
        useCIndex = true;
        fieldAdvanced = true;
    }
    else
    {
        useCIndex = false;
    
        int entry = fr[0].getNoNestedBrackets();

        bool is_ushort = true;
        int capacity = 100;
        
        int i=1;
        if      (fr[i].matchWord("ushort")) { is_ushort = true; ++i; }
        else if (fr[i].matchWord("uint"))   { is_ushort = false; ++i; }
        
        if (fr[i].isInt()) { fr[i].getInt(capacity); ++i; }
        
        if (fr[i].isOpenBracket())
        {
            ++i;
            fr += i;

            int size = 0;

            if (is_ushort)
            {
                // read ushorts...
                GLushort* coordIndexList = new GLushort[capacity];

                while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
                {
                    int index;
                    if (fr[0].getInt(index))
                    {

                        if (size>=capacity)
                        {
                            int oldCapacity = capacity;
                            while(capacity<=size) capacity *= 2;
                            GLushort* oldList = coordIndexList;
                            coordIndexList = new GLushort[capacity];
                            for(int i=0;i<oldCapacity;++i)
                            {
                                coordIndexList[i] = oldList[i];
                            }
                            delete [] oldList;
                        }
                        coordIndexList[size] = index;
                        ++size;

                    }
                    ++fr;
                }

                fieldAdvanced = true;
                ++fr;
                
                ip.set(size,coordIndexList);
                
            }
            else
            {
                // read uints...
                GLuint* coordIndexList = new GLuint[capacity];

                while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
                {
                    int index;
                    if (fr[0].getInt(index))
                    {

                        if (size>=capacity)
                        {
                            int oldCapacity = capacity;
                            while(capacity<=size) capacity *= 2;
                            GLuint* oldList = coordIndexList;
                            coordIndexList = new GLuint[capacity];
                            for(int i=0;i<oldCapacity;++i)
                            {
                                coordIndexList[i] = oldList[i];
                            }
                            delete [] oldList;
                        }
                        coordIndexList[size] = index;
                        ++size;

                    }
                    ++fr;
                }

                fieldAdvanced = true;
                ++fr;
                
                ip.set(size,coordIndexList);
            }                
        }
                
    }
    return fieldAdvanced;
}

bool GeoSet_writeIndexData(Output& fw, const char* IndexName,const GeoSet::IndexPointer& ip)
{
    if (ip._size)
    {
        if (ip._is_ushort)
        {
            // write our CoordIndex
            fw.indent() << IndexName << " ushort " << ip._size<< std::endl;
            writeArray(fw,ip._ptr._ushort,ip._ptr._ushort+ip._size);
        }
        else
        {
            // write our CoordIndex
            fw.indent() << IndexName << " uint " << ip._size<< std::endl;
            writeArray(fw,ip._ptr._uint,ip._ptr._uint+ip._size);
        }
        return true;
    }
    else
    {
        return false;
    }
}

bool GeoSet_matchBindingTypeStr(const char* str,GeoSet::BindingType& mode)
{
    if (strcmp(str,"OFF")==0) mode = GeoSet::BIND_OFF;
    else if (strcmp(str,"OVERALL")==0) mode = GeoSet::BIND_OVERALL;
    else if (strcmp(str,"PER_PRIMITIVE")==0) mode = GeoSet::BIND_PERPRIM;
    else if (strcmp(str,"PER_VERTEX")==0) mode = GeoSet::BIND_PERVERTEX;
    else if (strcmp(str,"DEFAULT")==0) mode = GeoSet::BIND_DEFAULT;
    else return false;
    return true;
}


const char* GeoSet_getBindingTypeStr(GeoSet::BindingType mode)
{
    switch(mode)
    {
        case (GeoSet::BIND_OFF)        : return "OFF";
        case (GeoSet::BIND_OVERALL)    : return "OVERALL";
        case (GeoSet::BIND_PERPRIM)    : return "PER_PRIMITIVE";
        case (GeoSet::BIND_PERVERTEX)  : return "PER_VERTEX";
        case (GeoSet::BIND_DEFAULT):
        default                     : return "DEFAULT";
    }
}
const char* GeoSet_getInterleavedRowComposition(GeoSet::InterleaveArrayType at)
{
    switch(at)
    {
        case(GeoSet::IA_OFF): return "";
        case(GeoSet::IA_V2F): return "ff";
        case(GeoSet::IA_V3F): return "fff";
        case(GeoSet::IA_C4UB_V2F): return "bbbbff";
        case(GeoSet::IA_C4UB_V3F): return "bbbbfff";
        case(GeoSet::IA_C3F_V3F): return "ffffff";
        case(GeoSet::IA_N3F_V3F): return "ffffff";
        case(GeoSet::IA_C4F_N3F_V3F): return "ffffffffff";
        case(GeoSet::IA_T2F_V3F): return "fffff";
        case(GeoSet::IA_T4F_V4F): return "ffffffff";
        case(GeoSet::IA_T2F_C4UB_V3F): return "ffbbbbfff";
        case(GeoSet::IA_T2F_C3F_V3F): return "ffffffff";
        case(GeoSet::IA_T2F_N3F_V3F): return "ffffffff";
        case(GeoSet::IA_T2F_C4F_N3F_V3F): return "ffffffffffff";
        case(GeoSet::IA_T4F_C4F_N3F_V4F): return "fffffffffffffff";
    }
    return "";
}


int GeoSet_getInterleavedRowLength(GeoSet::InterleaveArrayType at)
{
    switch(at)
    {
        case(GeoSet::IA_OFF): return 0;
        case(GeoSet::IA_V2F): return 8;
        case(GeoSet::IA_V3F): return 6;
        case(GeoSet::IA_C4UB_V2F): return 12;
        case(GeoSet::IA_C4UB_V3F): return 16;
        case(GeoSet::IA_C3F_V3F): return 24;
        case(GeoSet::IA_N3F_V3F): return 24;
        case(GeoSet::IA_C4F_N3F_V3F): return 40;
        case(GeoSet::IA_T2F_V3F): return 20;
        case(GeoSet::IA_T4F_V4F): return 32;
        case(GeoSet::IA_T2F_C4UB_V3F): return 24;
        case(GeoSet::IA_T2F_C3F_V3F): return 32;
        case(GeoSet::IA_T2F_N3F_V3F): return 32;
        case(GeoSet::IA_T2F_C4F_N3F_V3F): return 48;
        case(GeoSet::IA_T4F_C4F_N3F_V4F): return 60;
    }
    return 0;
}
