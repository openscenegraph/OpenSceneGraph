#include <stdio.h>
#include <math.h>
#include <float.h>

#include "osg/GeoSet"
#include "osg/Input"
#include "osg/Output"
#include "osg/Notify"

using namespace osg;

GeoSet::GeoSet()
{
    _coords     = (Vec3 *)0;
    _cindex     = (ushort *)0;

    _normals = (Vec3 *)0;
    _nindex     = (ushort *)0;

    _colors     = (Vec4 *)0;
    _colindex = (ushort *)0;

    _tcoords = (Vec2 *)0;
    _tindex     = (ushort *)0;

    _iarray = (float *)0L;
    _iaindex = (ushort *)0;
    _iaformat = IA_OFF;
    _ogliaformat = 0;

    _bbox_computed = 0;

    _numprims = 0;
    _primtype = NO_TYPE;
    _oglprimtype = 0xFFFF;
    _needprimlen = 0; 
    _primLengths = (int *)0;

    _useDisplayList = true;
    _globj    = 0;

    _numcoords = 0;

    _normal_binding = BIND_OFF;
    _color_binding  = BIND_OFF;
    _texture_binding  = BIND_OFF;

    _fast_path = 1;
}


GeoSet::~GeoSet()
{
    if( _globj != 0 )
    {
        glDeleteLists( _globj, 1 );
    }
    // note all coordinates, colors, texture coordinates and normals
    // are not currently deleted, ahh!!!!  This issue needs to be
    // addressed.  However, since the data is simple passed in it
    // is unclear whether the data is shared, allocated with malloc,
    // or new [] so its difficult right now to now how to delete it
    // appropriatly.  Should data be copied rather than simply copying
    // pointers??  Using vector<> could simplify this issue. But then
    // would you want to share coords etc?
    // Robert Osfield, Decemeber 2000.
}

GeoSet* GeoSet::instance()
{
    static ref_ptr<GeoSet> s_geoset(new GeoSet);
    return s_geoset.get();
}

bool GeoSet::readLocalData(Input& fr)
{
    bool iteratorAdvanced = false;

    if (fr[0].matchWord("Use"))
    {
        if (fr[1].isString()) {
            GeoState* geostate = dynamic_cast<GeoState*>(fr.getObjectForUniqueID(fr[1].getStr()));
            if (geostate) {
                fr+=2;
                setGeoState(geostate);
                iteratorAdvanced = true;
            }
        }
    }

    if (GeoState* readState = static_cast<GeoState*>(GeoState::instance()->readClone(fr)))
    {
        setGeoState(readState);
        iteratorAdvanced = true;
    }

    Vec3* coordList = NULL;
    ushort* coordIndexList = NULL;
    Vec3* normalList = NULL;
    ushort* normalIndexList = NULL;
    Vec4* colorList = NULL;
    ushort* colorIndexList = NULL;
    Vec2* textureList = NULL;
    ushort* textureIndexList = NULL;
    float* interleavedArray = NULL;
    ushort* interleavedIndexArray = NULL;

    BindingType bind=BIND_OFF;
    BindingType normal_bind=BIND_OFF;
    BindingType color_bind=BIND_OFF;
    BindingType texture_bind=BIND_OFF;
    InterleaveArrayType iaType = IA_OFF;

    int start_indent = fr[0].getNoNestedBrackets();
    while (!fr.eof() && fr[0].getNoNestedBrackets()>=start_indent)
    {

        bool fieldAdvanced = false;

        bool readPrimitiveLengths = false;
        if (fr.matchSequence("tstrips %i {"))
        {
            readPrimitiveLengths = true;
            setPrimType(TRIANGLE_STRIP);
        }
        else if (fr.matchSequence("flat_tstrips %i {"))
        {
            readPrimitiveLengths = true;
            setPrimType(FLAT_TRIANGLE_STRIP);
        }
        else if (fr.matchSequence("polys %i {"))
        {
            readPrimitiveLengths = true;
            setPrimType(POLYGON);
        }
        else if (fr.matchSequence("quadstrip %i {"))
        {
            readPrimitiveLengths = true;
            setPrimType(QUAD_STRIP);
        }
        else if (fr.matchSequence("lineloops %i {"))
        {
            readPrimitiveLengths = true;
            setPrimType(LINE_LOOP);
        }
        else if (fr.matchSequence("linestrip %i {"))
        {
            readPrimitiveLengths = true;
            setPrimType(LINE_STRIP);
        }
        else if (fr.matchSequence("flat_linestrip %i {"))
        {
            readPrimitiveLengths = true;
            setPrimType(LINE_STRIP);
        }
        else if (fr.matchSequence("tfans %i {"))
        {
            readPrimitiveLengths = true;
            setPrimType(TRIANGLE_FAN);
        }
        else if (fr.matchSequence("flat_tfans %i {"))
        {
            readPrimitiveLengths = true;
            setPrimType(FLAT_TRIANGLE_FAN);
        }

        if (readPrimitiveLengths)
        {

            int entry = fr[1].getNoNestedBrackets();
            fr += 3;

            int capacity;
            if (!fr[1].getInt(capacity)) capacity=100;
            int size = 0;
            int* list = new int [capacity];

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

            setNumPrims(size);
            setPrimLengths(list);
        }

        if (fr.matchSequence("lines %i"))
        {
            setPrimType(LINES);
            int noLines;
            if (fr[1].getInt(noLines)) setNumPrims(noLines);
            fieldAdvanced = true;
            fr+=2;
        }

        if (fr.matchSequence("triangles %i"))
        {
            setPrimType(TRIANGLES);
            int noTriangles;
            if (fr[1].getInt(noTriangles)) setNumPrims(noTriangles);
            fieldAdvanced = true;
            fr+=2;
        }

        if (fr.matchSequence("quads %i"))
        {
            setPrimType(QUADS);
            int noQuads;
            if (fr[1].getInt(noQuads)) setNumPrims(noQuads);
            fieldAdvanced = true;
            fr+=2;
        }

        if (fr.matchSequence("points %i"))
        {
            setPrimType(POINTS);
            int noPoints;
            if (fr[1].getInt(noPoints)) setNumPrims(noPoints);
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

        if ((matchFirst=fr.matchSequence("CoordIndex {")) || fr.matchSequence("CoordIndex %i {"))
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

            coordIndexList = new ushort[capacity];

            while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
            {
                int index;
                if (fr[0].getInt(index))
                {

                    if (size>=capacity)
                    {
                        int oldCapacity = capacity;
                        while(capacity<=size) capacity *= 2;
                        ushort* oldList = coordIndexList;
                        coordIndexList = new ushort[capacity];
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

        }

        if (fr[0].matchWord("Normal_Binding") && matchBindingTypeStr(fr[1].getStr(),bind))
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

        if ((matchFirst=fr.matchSequence("NIndex {")) || fr.matchSequence("NIndex %i {"))
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

            normalIndexList = new ushort[capacity];

            while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
            {
                int index;
                if (fr[0].getInt(index))
                {

                    if (size>=capacity)
                    {
                        int oldCapacity = capacity;
                        while(capacity<=size) capacity *= 2;
                        ushort* oldList = normalIndexList;
                        normalIndexList = new ushort[capacity];
                        for(int i=0;i<oldCapacity;++i)
                        {
                            normalIndexList[i] = oldList[i];
                        }
                        delete [] oldList;
                    }
                    normalIndexList[size] = index;
                    ++size;

                }
                ++fr;
            }

            fieldAdvanced = true;
            ++fr;

        }

        if (fr[0].matchWord("Color_Binding") && matchBindingTypeStr(fr[1].getStr(),bind))
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

        if ((matchFirst=fr.matchSequence("ColIndex {")) || fr.matchSequence("ColIndex %i {"))
        {

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

            colorIndexList = new ushort[capacity];

            while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
            {
                int index;
                if (fr[0].getInt(index))
                {

                    if (size>=capacity)
                    {
                        int oldCapacity = capacity;
                        while(capacity<=size) capacity *= 2;
                        ushort* oldList = colorIndexList;
                        colorIndexList = new ushort[capacity];
                        for(int i=0;i<oldCapacity;++i)
                        {
                            colorIndexList[i] = oldList[i];
                        }
                        delete [] oldList;
                    }
                    colorIndexList[size] = index;
                    ++size;

                }
                ++fr;
            }

            fieldAdvanced = true;
            ++fr;

        }

        if (fr[0].matchWord("Texture_Binding") && matchBindingTypeStr(fr[1].getStr(),bind))
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

        if ((matchFirst=fr.matchSequence("TIndex {")) || fr.matchSequence("TIndex %i {"))
        {

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

            textureIndexList = new ushort[capacity];

            while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
            {
                int index;
                if (fr[0].getInt(index))
                {

                    if (size>=capacity)
                    {
                        int oldCapacity = capacity;
                        while(capacity<=size) capacity *= 2;
                        ushort* oldList = textureIndexList;
                        textureIndexList = new ushort[capacity];
                        for(int i=0;i<oldCapacity;++i)
                        {
                            textureIndexList[i] = oldList[i];
                        }
                        delete [] oldList;
                    }
                    textureIndexList[size] = index;
                    ++size;

                }
                ++fr;
            }

            fieldAdvanced = true;
            ++fr;

        }

        if (matchFirst=fr.matchSequence("InterleavedArray %s {") || 
            fr.matchSequence("InterleavedArray %s %i {"))
        {

// set up coordinates.
            int entry = fr[0].getNoNestedBrackets();
            
            const char* type = fr[1].getStr();
            if (strcmp(type,"IA_OFF")==0) iaType = IA_OFF;
            else if (strcmp(type,"IA_V2F")==0) iaType =IA_V2F;
            else if (strcmp(type,"IA_V3F")==0) iaType =IA_V3F;
            else if (strcmp(type,"IA_C4UB_V2F")==0) iaType =IA_C4UB_V2F;
            else if (strcmp(type,"IA_C4UB_V3F")==0) iaType =IA_C4UB_V3F;
            else if (strcmp(type,"IA_C3F_V3F")==0) iaType =IA_C3F_V3F;
            else if (strcmp(type,"IA_N3F_V3F")==0) iaType =IA_N3F_V3F;
            else if (strcmp(type,"IA_C4F_N3F_V3F")==0) iaType =IA_C4F_N3F_V3F;
            else if (strcmp(type,"IA_T2F_V3F")==0) iaType =IA_T2F_V3F;
            else if (strcmp(type,"IA_T4F_V4F")==0) iaType =IA_T4F_V4F;
            else if (strcmp(type,"IA_T2F_C4UB_V3F")==0) iaType =IA_T2F_C4UB_V3F;
            else if (strcmp(type,"IA_T2F_C3F_V3F")==0) iaType =IA_T2F_C3F_V3F;
            else if (strcmp(type,"IA_T2F_N3F_V3F")==0) iaType =IA_T2F_N3F_V3F;
            else if (strcmp(type,"IA_T2F_C4F_N3F_V3F")==0) iaType =IA_T2F_C4F_N3F_V3F;
            else if (strcmp(type,"IA_T4F_C4F_N3F_V4F")==0) iaType =IA_T4F_C4F_N3F_V4F;
            else 
            {  
                iaType = IA_OFF;
            }


            int capacity = 100;
            if (matchFirst)
            {
                fr += 3;
            }
            else
            {
                fr[3].getInt(capacity);
                fr += 4;
            }

            if (iaType == IA_OFF)
            {
                // no data should be read - read over {}.
                interleavedArray = NULL;
                while (!fr.eof() && fr[0].getNoNestedBrackets()>entry) ++fr;
            }
            else
            {
                // now read the data rows between the {}.
                const char* rowComp = getInterleavedRowComposition(iaType);
                int rowLength = getInterleavedRowLength(iaType);

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
                    while (*itrRowComp!=0)
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
                        ++fr;
                    }
                }

                interleavedArray = (float*)dataList;            
            }
            
            fieldAdvanced = true;
            ++fr;

        }

        if ((matchFirst=fr.matchSequence("InterleavedArrayIndex {")) || fr.matchSequence("InterleavedArrayIndex %i {"))
        {

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

            interleavedIndexArray = new ushort[capacity];

            while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
            {
                int index;
                if (fr[0].getInt(index))
                {

                    if (size>=capacity)
                    {
                        int oldCapacity = capacity;
                        while(capacity<=size) capacity *= 2;
                        ushort* oldList = interleavedIndexArray;
                        interleavedIndexArray = new ushort[capacity];
                        for(int i=0;i<oldCapacity;++i)
                        {
                            interleavedIndexArray[i] = oldList[i];
                        }
                        delete [] oldList;
                    }
                    interleavedIndexArray[size] = index;
                    ++size;

                }
                ++fr;
            }

            fieldAdvanced = true;
            ++fr;

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
        if (coordIndexList)
        {
            setCoords(coordList,coordIndexList);
        }
        else
        {
            setCoords(coordList);
        }
    }

// set up the normal lists.
    if (normalList)
    {
        setNormalBinding(normal_bind);
        if (normalIndexList)
        {
            setNormals(normalList,normalIndexList);
        }
        else
        {
            setNormals(normalList);
        }
    } else setNormalBinding(BIND_OFF);

// set up the color lists.
    if (colorList)
    {
        setColorBinding(color_bind);
        if (colorIndexList)
        {
            setColors(colorList,colorIndexList);
        }
        else
        {
            setColors(colorList);
        }
    } else setColorBinding(BIND_OFF);

    if (textureList)
    {
        setTextureBinding(texture_bind);
        if (textureIndexList)
        {
            setTextureCoords(textureList,textureIndexList);
        }
        else
        {
            setTextureCoords(textureList);
        }
    } else setTextureBinding(BIND_OFF);

    if (interleavedArray)
    {
        if (interleavedIndexArray)
        {
            setInterleavedArray(iaType,interleavedArray,interleavedIndexArray);
        }
        else
        {
            setInterleavedArray(iaType,interleavedArray);
        }
    } else _iaformat = IA_OFF;

    return iteratorAdvanced;
}


bool GeoSet::writeLocalData(Output& fw)
{
    int i;

    if (_state.valid())
    {
        _state->write(fw);
    }

    // write out primitives.
    bool writeOutPrimitiveLengths = false;
    switch(getPrimType())
    {
        case (TRIANGLE_STRIP):
            fw.indent()<<"tstrips "<< getNumPrims() << endl;
            writeOutPrimitiveLengths = true;
            break;
        case (FLAT_TRIANGLE_STRIP):
            fw.indent()<<"flat_tstrips "<< getNumPrims() << endl;
            writeOutPrimitiveLengths = true;
            break;
        case (POLYGON):
            fw.indent()<<"polys "<< getNumPrims() << endl;
            writeOutPrimitiveLengths = true;
            break;
        case (QUAD_STRIP):
            fw.indent()<<"quadstrip "<< getNumPrims() << endl;
            writeOutPrimitiveLengths = true;
            break;
        case (LINE_LOOP):
            fw.indent()<<"lineloops "<< getNumPrims() << endl;
            writeOutPrimitiveLengths = true;
            break;
        case (LINE_STRIP):
            fw.indent()<<"linestrip "<< getNumPrims() << endl;
            writeOutPrimitiveLengths = false;
            break;
        case (FLAT_LINE_STRIP):
            fw.indent()<<"flat_linestrip "<< getNumPrims() << endl;
            writeOutPrimitiveLengths = false;
            break;
        case (TRIANGLE_FAN):
            fw.indent()<<"tfans "<< getNumPrims() << endl;
            writeOutPrimitiveLengths = true;
        case (FLAT_TRIANGLE_FAN):
            fw.indent()<<"flat_tfans "<< getNumPrims() << endl;
            writeOutPrimitiveLengths = true;
            break;
        case (LINES):
            fw.indent()<<"lines "<< getNumPrims() << endl;
            writeOutPrimitiveLengths = false;
            break;
        case (TRIANGLES):
            fw.indent()<<"triangles "<< getNumPrims() << endl;
            writeOutPrimitiveLengths = false;
            break;
        case (QUADS):
            fw.indent()<<"quads "<< getNumPrims() << endl;
            writeOutPrimitiveLengths = false;
            break;
        case (POINTS) :
            fw.indent()<<"points "<< getNumPrims() << endl;
            break;
        default:
            notify(WARN) << "GeoSet::writeLocalData() - unhandled primitive type = "<<(int)getPrimType()<<endl;
    }
    if (writeOutPrimitiveLengths)
    {
        writeArrayBlock(fw,getPrimLengths(),getPrimLengths()+getNumPrims());
    }

    computeNumVerts();

    if (_coords)
    {
        // write out _coords.
        fw.indent() << "Coords " << _numcoords<<endl;
        fw.indent() << "{"<<endl;
        fw.moveIn();
        for(i=0;i<_numcoords;++i)
        {
            fw.indent() << _coords[i][0] << ' ' << _coords[i][1] << ' ' << _coords[i][2] << endl;
        }
        fw.moveOut();
        fw.indent()<<"}"<<endl;
    }
    if (_cindex)
    {
        // write our CoordIndex
        fw.indent() << "CoordIndex " << _numindices<<endl;
        writeArrayBlock(fw,_cindex,_cindex+_numindices);
    }

    if (_normals)
    {
        // write out _normals.
        fw.indent() << "Normal_Binding "<<getBindingTypeStr(_normal_binding)<<endl;

        fw.indent() << "Normals " << _numnormals<<endl;
        fw.indent() << "{"<<endl;
        fw.moveIn();
        for(i=0;i<_numnormals;++i)
        {
            fw.indent() << _normals[i][0] << ' ' << _normals[i][1] << ' ' << _normals[i][2] << endl;
        }
        fw.moveOut();
        fw.indent()<<"}"<<endl;
    }
    if (_nindex)
    {
        // write our _nindex
        fw.indent() << "NIndex " << _numnindices<<endl;
        writeArrayBlock(fw,_nindex,_nindex+_numnindices);
    }

    if (_colors)
    {
        // write out _colors.
        fw.indent() << "Color_Binding "<<getBindingTypeStr(_color_binding)<<endl;

        fw.indent() << "Colors " << _numcolors<<endl;
        fw.indent() << "{"<<endl;
        fw.moveIn();
        for(i=0;i<_numcolors;++i)
        {
            fw.indent() << _colors[i][0] << ' ' << _colors[i][1] << ' ' << _colors[i][2] << ' ' << _colors[i][3] << endl;
        }
        fw.moveOut();
        fw.indent()<<"}"<<endl;
    }
    if (_colindex)
    {
    // write our _colindex
        fw.indent() << "ColIndex " << _numcindices<<endl;
        writeArrayBlock(fw,_colindex,_colindex+_numcindices);
    }


    if (_tcoords)
    {
        // write out _tcoords.
        fw.indent() << "Texture_Binding "<<getBindingTypeStr(_texture_binding)<<endl;

        fw.indent() << "TCoords " << _numtcoords<<endl;
        fw.indent() << "{"<<endl;
        fw.moveIn();
        for(i=0;i<_numtcoords;++i)
        {
            fw.indent() << _tcoords[i][0] << ' ' << _tcoords[i][1] << endl;
        }
        fw.moveOut();
        fw.indent()<<"}"<<endl;
    }
    if (_tindex)
    {
        // write our _tindex
        fw.indent() << "TIndex " << _numtindices<<endl;
        writeArrayBlock(fw,_tindex,_tindex+_numtindices);
    }

    if (_iarray)
    {
        // write out the interleaved arrays.
        const char* rowComp = getInterleavedRowComposition(_iaformat);

        fw.indent() << "InterleavedArray ";
        switch(_iaformat)
        {
        case(IA_OFF): fw << "IA_OFF"; break;
        case(IA_V2F): fw << "IA_V2F"; break;
        case(IA_V3F): fw << "IA_V3F"; break;
        case(IA_C4UB_V2F): fw << "IA_C4UB_V2F"; break;
        case(IA_C4UB_V3F): fw << "IA_C4UB_V3F"; break;
        case(IA_C3F_V3F): fw << "IA_C3F_V3F"; break;
        case(IA_N3F_V3F): fw << "IA_N3F_V3F"; break;
        case(IA_C4F_N3F_V3F): fw << "IA_C4F_N3F_V3F"; break;
        case(IA_T2F_V3F): fw << "IA_T2F_V3F"; break;
        case(IA_T4F_V4F): fw << "IA_T4F_V4F"; break;
        case(IA_T2F_C4UB_V3F): fw << "IA_T2F_C4UB_V3F"; break;
        case(IA_T2F_C3F_V3F): fw << "IA_T2F_C3F_V3F"; break;
        case(IA_T2F_N3F_V3F): fw << "IA_T2F_N3F_V3F"; break;
        case(IA_T2F_C4F_N3F_V3F): fw << "IA_T2F_C4F_N3F_V3F"; break;
        case(IA_T4F_C4F_N3F_V4F): fw << "IA_T4F_C4F_N3F_V4F"; break;
        default: fw << "IA_OFF"; break;
        }

        fw << " " <<  _numcoords<<endl;
        fw.indent() << "{"<<endl;
        fw.moveIn();

        unsigned char* itrRowData = (unsigned char*)_iarray;
        for(i=0;i<_numcoords;++i)
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
            }
            fw << endl;

        }
        fw.moveOut();
        fw.indent()<<"}"<<endl;



    }

    return true;
}


const char* GeoSet::getInterleavedRowComposition(InterleaveArrayType at) const
{
    switch(at)
    {
    case(IA_OFF): return "";
    case(IA_V2F): return "ff";
    case(IA_V3F): return "fff";
    case(IA_C4UB_V2F): return "bbbbff";
    case(IA_C4UB_V3F): return "bbbbfff";
    case(IA_C3F_V3F): return "ffffff";
    case(IA_N3F_V3F): return "ffffff";
    case(IA_C4F_N3F_V3F): return "ffffffffff";
    case(IA_T2F_V3F): return "fffff";
    case(IA_T4F_V4F): return "ffffffff";
    case(IA_T2F_C4UB_V3F): return "ffbbbbfff";
    case(IA_T2F_C3F_V3F): return "ffffffff";
    case(IA_T2F_N3F_V3F): return "ffffffff";
    case(IA_T2F_C4F_N3F_V3F): return "ffffffffffff";
    case(IA_T4F_C4F_N3F_V4F): return "fffffffffffffff";
    }
    return "";
}

int GeoSet::getInterleavedRowLength(InterleaveArrayType at) const
{
    switch(at)
    {
    case(IA_OFF): return 0;
    case(IA_V2F): return 8;
    case(IA_V3F): return 6;
    case(IA_C4UB_V2F): return 12;
    case(IA_C4UB_V3F): return 16;
    case(IA_C3F_V3F): return 24;
    case(IA_N3F_V3F): return 24;
    case(IA_C4F_N3F_V3F): return 40;
    case(IA_T2F_V3F): return 20;
    case(IA_T4F_V4F): return 32;
    case(IA_T2F_C4UB_V3F): return 24;
    case(IA_T2F_C3F_V3F): return 32;
    case(IA_T2F_N3F_V3F): return 32;
    case(IA_T2F_C4F_N3F_V3F): return 48;
    case(IA_T4F_C4F_N3F_V4F): return 60;
    }
    return 0;
}

bool GeoSet::matchBindingTypeStr(const char* str,BindingType& mode)
{
    if (strcmp(str,"OFF")==0) mode = BIND_OFF;
    else if (strcmp(str,"OVERALL")==0) mode = BIND_OVERALL;
    else if (strcmp(str,"PER_PRIMITIVE")==0) mode = BIND_PERPRIM;
    else if (strcmp(str,"PER_VERTEX")==0) mode = BIND_PERVERTEX;
    else if (strcmp(str,"DEFAULT")==0) mode = BIND_DEFAULT;
    else return false;
    return true;
}


const char* GeoSet::getBindingTypeStr(BindingType mode)
{
    switch(mode)
    {
        case (BIND_OFF)        : return "OFF";
        case (BIND_OVERALL)    : return "OVERALL";
        case (BIND_PERPRIM)    : return "PER_PRIMITIVE";
        case (BIND_PERVERTEX)  : return "PER_VERTEX";
        case (BIND_DEFAULT):
        default                     : return "DEFAULT";
    }
}

void GeoSet::setColorBinding( BindingType binding )
{
    if( binding != BIND_DEFAULT &&
        binding != BIND_OFF &&
        binding != BIND_OVERALL &&
        binding != BIND_PERPRIM  &&
        binding != BIND_PERVERTEX )
        _color_binding = BIND_OFF;
    else
        _color_binding = binding;

    if( _color_binding == BIND_DEFAULT )
        _color_binding = BIND_PERVERTEX;


    set_fast_path();
}


void GeoSet::setNormalBinding( BindingType binding )
{
    if( binding != BIND_DEFAULT &&
        binding != BIND_OFF &&
        binding != BIND_OVERALL &&
        binding != BIND_PERPRIM  &&
        binding != BIND_PERVERTEX )
        _normal_binding = BIND_OFF;
    else
        _normal_binding = binding;

    if( _normal_binding == BIND_DEFAULT )
    	_normal_binding = BIND_PERVERTEX;

    set_fast_path();
}



void GeoSet::setTextureBinding( BindingType binding )
{
    if( binding != BIND_DEFAULT &&
        binding != BIND_OFF &&
        binding != BIND_PERVERTEX )
        _texture_binding = BIND_OFF;
    else
        _texture_binding = binding;

    if( _texture_binding == BIND_DEFAULT )
    	_texture_binding = BIND_PERVERTEX;

    set_fast_path();
}


void GeoSet::draw( void )
{
    if( _globj != 0 )
    {
        glCallList( _globj );
    }
    else if (_useDisplayList)
    {
        _globj = glGenLists( 1 );        
        glNewList( _globj, GL_COMPILE_AND_EXECUTE );
        drawImmediateMode();
        glEndList();
    } else
    {
        drawImmediateMode();
    }
}

void GeoSet::drawImmediateMode()
{
    if( _coords == (Vec3 *)0 && _iaformat == IA_OFF ) return;

    // need to do this to get a valid _numcoords, _numindices & _primlength
    if( _numcoords == 0 )
	computeNumVerts();

    if( _fast_path )
        draw_fast_path();
    else
    	draw_alternate_path();
}

void GeoSet::compile( void )
{
    _useDisplayList = true;

    if( _globj != 0 )
    {
        glDeleteLists( _globj, 1 );
    }

    if (_state.valid()) _state->apply();

    _globj = glGenLists( 1 );
    glNewList( _globj, GL_COMPILE );
    drawImmediateMode();
    glEndList();

}

void GeoSet::computeNumVerts()
{
    int i;
    int numverts=0;

    int flat_shaded_offset=0;
    if      (_primtype == FLAT_LINE_STRIP)     flat_shaded_offset=_numprims;
    else if (_primtype == FLAT_TRIANGLE_STRIP) flat_shaded_offset=2*_numprims;
    else if (_primtype == FLAT_TRIANGLE_FAN)   flat_shaded_offset=2*_numprims;

    switch( _primtype )
    {
        case POINTS :
	    _primlength = 1;
            numverts = _numprims * _primlength;
            break;

        case LINES :
	    _primlength = 2;
            numverts = _numprims * _primlength;
            break;

        case TRIANGLES :
	    _primlength = 3;
            numverts =  _numprims * _primlength;
            break;

        case QUADS :
	    _primlength = 4;
            numverts =  _numprims * _primlength;
            break;

        case QUAD_STRIP :
        case FLAT_TRIANGLE_FAN :
        case TRIANGLE_FAN :
        case LINE_LOOP :
        case LINE_STRIP :
        case FLAT_LINE_STRIP :
        case TRIANGLE_STRIP :
        case FLAT_TRIANGLE_STRIP :
        case POLYGON :
	    _primlength = 0;
            numverts = 0;
            for( i = 0; i < _numprims; i++ )
                numverts += _primLengths[i];
            break;
        default:
            notify(WARN) << "Not supported primitive "<<(int)_primtype<<endl;
            break;
    }


    if( _cindex != (ushort *)0L )
    {
        int max = 0;

        _numindices = numverts;
        for( i = 0; i < _numindices; i++ )
            if( _cindex[i] > max ) max = _cindex[i];

        _numcoords = max + 1;
    }
    else
    {
        _numcoords = numverts;
        _numcindices = 0;
    }




    if (_normals)
    {

        int nn;
        switch(_normal_binding)
        {
        case (BIND_OFF)        : nn = 0; break;
        case (BIND_OVERALL)    : nn = 1; break;
        case (BIND_PERPRIM)    : nn = getNumPrims(); break;
        case (BIND_PERVERTEX)  : nn = numverts-flat_shaded_offset; break;
        default                     : nn = 0; break;
        }

        // calc the maximum num of vertex from the index list.
        int cc;
        if (_nindex)
        {
            _numnindices = nn;
            cc = 0;
            for( i = 0; i < nn; i++ )
                if( _nindex[i] > cc ) cc = _nindex[i];
            cc++;
        }
        else
        {
            cc = nn;
            _numnindices = 0;
        }

        _numnormals = cc;
    }
    else
    {
        _numnormals = 0;
        _numnindices = 0;
    }

    if (_colors)
    {

        int nn;
        switch(_color_binding)
        {
        case (BIND_OFF)        : nn = 0; break;
        case (BIND_OVERALL)    : nn = 1; break;
        case (BIND_PERPRIM)    : nn = getNumPrims(); break;
        case (BIND_PERVERTEX)  : nn = numverts-flat_shaded_offset; break;
        default                     : nn = 0; break;
        }

        // calc the maximum num of vertex from the index list.
        int cc;
        if (_colindex)
        {
            _numcindices = nn;
            cc = 0;
            for( i = 0; i < nn; i++ )
                if( _colindex[i] > cc ) cc = _colindex[i];
            cc++;
        }
        else
        {
            cc = nn;
            _numcindices = 0;
        }

        _numcolors = cc;
    }
    else
    {
        _numcolors = 0;
        _numcindices = 0;
    }


    if (_tcoords)
    {

        int nn;
        switch(_texture_binding)
        {
        case (BIND_OFF)        : nn = 0; break;
        case (BIND_OVERALL)    : nn = 1; break;
        case (BIND_PERPRIM)    : nn = getNumPrims(); break;
        case (BIND_PERVERTEX)  : nn = numverts; break;
        default                     : nn = 0; break;
        }

        // calc the maximum num of vertex from the index list.
        int cc;
        if (_tindex)
        {
            _numtindices = nn;
            cc = 0;
            for( i = 0; i < nn; i++ )
                if( _tindex[i] > cc ) cc = _tindex[i];
            cc++;
        }
        else
        {
            cc = nn;
            _numtindices = 0;
        }

        _numtcoords = cc;
    }
    else
    {
        _numtcoords = 0;
        _numtindices = 0;
    }

}


void GeoSet::computeBound( void )
{
    int i;

    Vec3 center;

    if( _coords == (Vec3 *)0 ) return;

    if( _numcoords == 0 )
        computeNumVerts();

    if( _numcoords == 0 )
        return;

    center.set(0.0f,0.0f,0.0f);

    for( i = 0; i < _numcoords; i++ )
    {
        center += _coords[i];
    }

    center /= (float)_numcoords;

    _bbox.init();

    for( i = 0; i < _numcoords; i++ )
    {
        _bbox.expandBy(_coords[i]);
    }

    _bbox_computed++;
}

const BoundingBox& GeoSet::getBound()
{
    if( _bbox_computed == 0 )
        computeBound();

    return _bbox;
}

bool GeoSet::check()
{
    if( _coords == (Vec3 *)0 ) return false;

    if( _cindex   != (ushort *)0 ||
        _nindex   != (ushort *)0 ||
        _colindex != (ushort *)0 ||
        _tindex   != (ushort *)0  )
    {

        if( (_coords  && _cindex == (ushort *)0) ||
            (_normals && _nindex == (ushort *)0) ||
            (_colors  && _colindex == (ushort *)0) ||
            (_tcoords && _tindex == (ushort *)0)  )
        {

            notify(WARN) <<  "GeoState::check() : "
                "Cannot mix indexed and non-indexed attributes.\n";
            return false;
        }
    }
    return true;
}

void GeoSet::setPrimType( PrimitiveType type )
{
    switch( type )
    {
	case NO_TYPE: break;

	case POINTS:              _oglprimtype = GL_POINTS;         _needprimlen = 0; break;
	case LINES:               _oglprimtype = GL_LINES;          _needprimlen = 0; break;
	case FLAT_LINE_STRIP:     _oglprimtype = GL_LINE_STRIP;     _needprimlen=1; break;
	case LINE_STRIP:          _oglprimtype = GL_LINE_STRIP;     _needprimlen=1; break;
	case LINE_LOOP:           _oglprimtype = GL_LINE_LOOP;      _needprimlen=1; break;
	case TRIANGLES:           _oglprimtype = GL_TRIANGLES;      _needprimlen=0; break;
	case FLAT_TRIANGLE_STRIP: _oglprimtype = GL_TRIANGLE_STRIP; _needprimlen=1; break;
	case TRIANGLE_STRIP:      _oglprimtype = GL_TRIANGLE_STRIP; _needprimlen=1; break;
	case TRIANGLE_FAN:        _oglprimtype = GL_TRIANGLE_FAN;   _needprimlen=1; break;
	case FLAT_TRIANGLE_FAN:   _oglprimtype = GL_TRIANGLE_FAN;   _needprimlen=1; break;
	case QUADS:               _oglprimtype = GL_QUADS;          _needprimlen=0; break;
	case QUAD_STRIP:          _oglprimtype = GL_QUAD_STRIP;     _needprimlen=1; break;
	case POLYGON :            _oglprimtype = GL_POLYGON;        _needprimlen=1; break;
    }


    _primtype = type;

    if( _primtype == FLAT_LINE_STRIP ) _flat_shaded_skip = 1;
    else if( _primtype == FLAT_TRIANGLE_STRIP ) _flat_shaded_skip = 2;
    else if( _primtype == FLAT_TRIANGLE_FAN )   _flat_shaded_skip = 2;
    else _flat_shaded_skip = 0;
}


void GeoSet::setCoords( Vec3 *cp )                  
{ 
    _coords = cp; 
    _cindex = 0L;
    set_fast_path();
}
void GeoSet::setCoords( Vec3 *cp, ushort *ci )      
{ 
    _coords = cp; 
    _cindex = ci; 
    set_fast_path();
}

void GeoSet::setNormals( Vec3 *np )                 
{ 
    _normals = np; 
    _nindex = 0L;
    if( _normal_binding == BIND_OFF )
        setNormalBinding( BIND_DEFAULT );
    else
        set_fast_path();
}

void GeoSet::setNormals( Vec3 *np, ushort *ni )     
{ 
    _normals = np; 
    _nindex = ni; 
    if( _normal_binding == BIND_OFF )
        setNormalBinding( BIND_DEFAULT );
    else
        set_fast_path();
}


void GeoSet::setColors( Vec4 *lp )                  
{ 
    _colors = lp; 
    _colindex = 0L;
    if( _color_binding == BIND_OFF )
        setColorBinding( BIND_DEFAULT );
    else
        set_fast_path();
}

void GeoSet::setColors( Vec4 *lp, ushort *li )      
{ 
    _colors = lp; 
    _colindex = li; 
    if( _color_binding == BIND_OFF )
        setColorBinding( BIND_DEFAULT );
    else
        set_fast_path();
}

void GeoSet::setTextureCoords( Vec2 *tc )            
{ 
    _tcoords = tc; 
    _tindex = 0L;
    if( _texture_binding == BIND_OFF )
        setTextureBinding( BIND_DEFAULT );
    else
        set_fast_path();
}

void GeoSet::setTextureCoords( Vec2 *tc, ushort *ti ) 
{ 
    _tcoords = tc; 
    _tindex = ti; 
    if( _texture_binding == BIND_OFF )
        setTextureBinding( BIND_DEFAULT );
    else
        set_fast_path();

}

void GeoSet::setInterleavedArray( InterleaveArrayType format, float *pointer )
{
    _iaformat = format;

    _ogliaformat = 
    	(_iaformat == IA_OFF )            ? 0 :
	(_iaformat == IA_V2F )            ?  GL_V2F:
	(_iaformat == IA_V3F )            ?  GL_V3F:
	(_iaformat == IA_C4UB_V2F)        ?  GL_C4UB_V2F:
        (_iaformat == IA_C4UB_V3F)        ?  GL_C4UB_V3F:
        (_iaformat == IA_C3F_V3F)         ?  GL_C3F_V3F:
	(_iaformat == IA_N3F_V3F)         ?  GL_N3F_V3F:
	(_iaformat == IA_C4F_N3F_V3F)     ?  GL_C4F_N3F_V3F:
	(_iaformat == IA_T2F_V3F)         ?  GL_T2F_V3F:
	(_iaformat == IA_T4F_V4F)         ?  GL_T4F_V4F:
	(_iaformat == IA_T2F_C4UB_V3F)    ?  GL_T2F_C4UB_V3F:
	(_iaformat == IA_T2F_C3F_V3F)     ?  GL_T2F_C3F_V3F:
	(_iaformat == IA_T2F_N3F_V3F)     ?  GL_T2F_N3F_V3F:
	(_iaformat == IA_T2F_C4F_N3F_V3F) ?  GL_T2F_C4F_N3F_V3F: 
	(_iaformat == IA_T4F_C4F_N3F_V4F) ?  GL_T4F_C4F_N3F_V4F: 0;

    _iarray = pointer;

    set_fast_path();
}

void GeoSet::setInterleavedArray( InterleaveArrayType format, float *ia, ushort *iai )
{
    _iaformat = format;

    _ogliaformat = 
    	(_iaformat == IA_OFF )            ? 0 :
	(_iaformat == IA_V2F )            ?  GL_V2F:
	(_iaformat == IA_V3F )            ?  GL_V3F:
	(_iaformat == IA_C4UB_V2F)        ?  GL_C4UB_V2F:
        (_iaformat == IA_C4UB_V3F)        ?  GL_C4UB_V3F:
        (_iaformat == IA_C3F_V3F)         ?  GL_C3F_V3F:
	(_iaformat == IA_N3F_V3F)         ?  GL_N3F_V3F:
	(_iaformat == IA_C4F_N3F_V3F)     ?  GL_C4F_N3F_V3F:
	(_iaformat == IA_T2F_V3F)         ?  GL_T2F_V3F:
	(_iaformat == IA_T4F_V4F)         ?  GL_T4F_V4F:
	(_iaformat == IA_T2F_C4UB_V3F)    ?  GL_T2F_C4UB_V3F:
	(_iaformat == IA_T2F_C3F_V3F)     ?  GL_T2F_C3F_V3F:
	(_iaformat == IA_T2F_N3F_V3F)     ?  GL_T2F_N3F_V3F:
	(_iaformat == IA_T2F_C4F_N3F_V3F) ?  GL_T2F_C4F_N3F_V3F: 
	(_iaformat == IA_T4F_C4F_N3F_V4F) ?  GL_T4F_C4F_N3F_V4F: 0;

    _iarray = ia;
    _iaindex = iai;
    set_fast_path();
}

void GeoSet::setUseDisplayList(bool flag)
{
    // if value unchanged simply return.
    if (_useDisplayList==flag) return;

    // if was previously set to true, remove display list.
    if (_useDisplayList)
    {
        if( _globj != 0 )
        {
            glDeleteLists( _globj, 1 );
            _globj = 0;
        }
    }
    // set with new value.
    _useDisplayList = flag;
}

void GeoSet::dirtyDisplayList()
{
    if( _globj != 0 )
    {
        glDeleteLists( _globj, 1 );
        _globj = 0;
    }
}
