/*
    vertexData.cpp
    Copyright (c) 2007, Tobias Wolf <twolf@access.unizh.ch>
    All rights reserved.

    Implementation of the VertexData class.
*/

#include "typedefs.h"
#include "vertexData.h"

#include <cstdlib>
#include <algorithm>
#include <osg/Geometry>
#include <osg/Geode>
#include <osg/io_utils>
#include <osgUtil/SmoothingVisitor>
#include <osg/TexEnv>
#include <osgDB/ReaderWriter>
#include <osgDB/FileNameUtils>
#include <osgDB/ReadFile>
#include <osg/Texture2D>

using namespace std;
using namespace ply;

template<int PLYType> struct DrawElementCreator: public DrawElementFactory
{
    virtual osg::DrawElements * getDrawElement(){ OSG_FATAL<<"ply::VertexData: DrawElementCreator not implemented: "<<std::endl; return 0;}
    virtual void addElement(char * dataptr,osg::Array* arr){ OSG_FATAL<<"ply::VertexData: DrawElementCreator not implemented: "<<std::endl;}
};
template<> struct DrawElementCreator<PLY_UCHAR>: public DrawElementFactory
{
    virtual osg::DrawElements * getDrawElement(){ return new osg::DrawElementsUByte; }
    virtual void addElement(char * dataptr,osg::DrawElements* arr){ char *ptr=dataptr; static_cast<osg::DrawElementsUByte*>(arr)->push_back(ptr[0]); }
};
template<> struct DrawElementCreator<PLY_USHORT>: public DrawElementFactory
{
    virtual osg::DrawElements * getDrawElement(){ return new osg::DrawElementsUShort; }
    virtual void addElement(char * dataptr,osg::DrawElements* arr){ char *ptr=dataptr; static_cast<osg::DrawElementsUShort*>(arr)->push_back(ptr[0]); }
};
template<> struct DrawElementCreator<PLY_UINT>: public DrawElementFactory
{
    virtual osg::DrawElements * getDrawElement(){ return new osg::DrawElementsUInt; }
    virtual void addElement(char * dataptr,osg::DrawElements* arr){ char *ptr=dataptr; static_cast<osg::DrawElementsUInt*>(arr)->push_back(ptr[0]); }
};


template<int PLYType, int numcomp> struct ArrayCreator: public ArrayFactory
{
    virtual osg::Array * getArray(){ OSG_FATAL<<"ply::VertexData: ArrayCreator not implemented: "<<std::endl; return 0;}
    virtual void addElement(char * dataptr,osg::Array* arr){ OSG_FATAL<<"ply::VertexData: ArrayCreator not implemented: "<<std::endl;}
};
//
template<> struct ArrayCreator<PLY_CHAR,0>: public ArrayFactory
{
    virtual osg::Array * getArray(){ return new osg::ByteArray; }
    virtual void addElement(char * dataptr,osg::Array* arr){ char *ptr=dataptr; static_cast<osg::ByteArray*>(arr)->push_back(ptr[0]); }
};
template<> struct ArrayCreator<PLY_CHAR,1>: public ArrayFactory
{
    virtual osg::Array * getArray(){ return new osg::Vec2bArray; }
    virtual void addElement(char * dataptr, osg::Array* arr){ char *ptr=dataptr; static_cast<osg::Vec2bArray*>(arr)->push_back(osg::Vec2b(ptr[0],ptr[1])); }
};
template<> struct ArrayCreator<PLY_CHAR,2>: public ArrayFactory
{
    virtual osg::Array * getArray(){ return new osg::Vec3bArray; }
    virtual void addElement(char * dataptr, osg::Array* arr){ char *ptr=dataptr; static_cast<osg::Vec3bArray*>(arr)->push_back(osg::Vec3b(ptr[0],ptr[1],ptr[2])); }
};
template<> struct ArrayCreator<PLY_CHAR,3>: public ArrayFactory
{
    virtual osg::Array * getArray(){ return new osg::Vec4bArray; }
    virtual void addElement(char * dataptr, osg::Array* arr){ char *ptr=dataptr; static_cast<osg::Vec4bArray*>(arr)->push_back(osg::Vec4b(ptr[0],ptr[1],ptr[2],ptr[3])); }
};
//
template<> struct ArrayCreator<PLY_UCHAR,0>: public ArrayFactory
{
    virtual osg::Array * getArray(){ return new osg::UByteArray; }
    virtual void addElement(char * dataptr,osg::Array* arr){ unsigned char *ptr=(unsigned char*)dataptr; static_cast<osg::UByteArray*>(arr)->push_back(ptr[0]); }
};
template<> struct ArrayCreator<PLY_UCHAR,1>: public ArrayFactory
{
    virtual osg::Array * getArray(){ return new osg::Vec2ubArray; }
    virtual void addElement(char * dataptr, osg::Array* arr){ unsigned char *ptr=(unsigned char*)dataptr; static_cast<osg::Vec2ubArray*>(arr)->push_back(osg::Vec2ub(ptr[0],ptr[1])); }
};
template<> struct ArrayCreator<PLY_UCHAR,2>: public ArrayFactory
{
    virtual osg::Array * getArray(){ return new osg::Vec3ubArray; }
    virtual void addElement(char * dataptr, osg::Array* arr){ unsigned char *ptr=(unsigned char*)dataptr; static_cast<osg::Vec3ubArray*>(arr)->push_back(osg::Vec3ub(ptr[0],ptr[1],ptr[2])); }
};
template<> struct ArrayCreator<PLY_UCHAR,3>: public ArrayFactory
{
    virtual osg::Array * getArray(){ return new osg::Vec4ubArray; }
    virtual void addElement(char * dataptr, osg::Array* arr){ unsigned char *ptr=(unsigned char*)dataptr; static_cast<osg::Vec4ubArray*>(arr)->push_back(osg::Vec4ub(ptr[0],ptr[1],ptr[2],ptr[3])); }
};
//
template<> struct ArrayCreator<PLY_SHORT,0>: public ArrayFactory
{
    virtual osg::Array * getArray(){ return new osg::ShortArray; }
    virtual void addElement(char * dataptr,osg::Array* arr){ short *ptr=(short*)dataptr; static_cast<osg::ShortArray*>(arr)->push_back(ptr[0]); }
};
template<> struct ArrayCreator<PLY_SHORT,1>: public ArrayFactory
{
    virtual osg::Array * getArray(){ return new osg::Vec2sArray; }
    virtual void addElement(char * dataptr, osg::Array* arr){ short *ptr=(short*)dataptr; static_cast<osg::Vec2sArray*>(arr)->push_back(osg::Vec2s(ptr[0],ptr[1])); }
};
template<> struct ArrayCreator<PLY_SHORT,2>: public ArrayFactory
{
    virtual osg::Array * getArray(){ return new osg::Vec3sArray; }
    virtual void addElement(char * dataptr, osg::Array* arr){ short *ptr=(short*)dataptr; static_cast<osg::Vec3sArray*>(arr)->push_back(osg::Vec3s(ptr[0],ptr[1],ptr[2])); }
};
template<> struct ArrayCreator<PLY_SHORT,3>: public ArrayFactory
{
    virtual osg::Array * getArray(){ return new osg::Vec4sArray; }
    virtual void addElement(char * dataptr, osg::Array* arr){ short *ptr=(short*)dataptr; static_cast<osg::Vec4sArray*>(arr)->push_back(osg::Vec4s(ptr[0],ptr[1],ptr[2],ptr[3])); }
};
//
template<> struct ArrayCreator<PLY_USHORT,0>: public ArrayFactory
{
    virtual osg::Array * getArray(){ return new osg::UShortArray; }
    virtual void addElement(char * dataptr,osg::Array* arr){ unsigned short *ptr=(unsigned short*)dataptr; static_cast<osg::UShortArray*>(arr)->push_back(ptr[0]); }
};
template<> struct ArrayCreator<PLY_USHORT,1>: public ArrayFactory
{
    virtual osg::Array * getArray(){ return new osg::Vec2usArray; }
    virtual void addElement(char * dataptr, osg::Array* arr){ unsigned short *ptr=(unsigned short*)dataptr; static_cast<osg::Vec2usArray*>(arr)->push_back(osg::Vec2us(ptr[0],ptr[1])); }
};
template<> struct ArrayCreator<PLY_USHORT,2>: public ArrayFactory
{
    virtual osg::Array * getArray(){ return new osg::Vec3usArray; }
    virtual void addElement(char * dataptr, osg::Array* arr){ unsigned short *ptr=(unsigned short*)dataptr; static_cast<osg::Vec3usArray*>(arr)->push_back(osg::Vec3us(ptr[0],ptr[1],ptr[2])); }
};
template<> struct ArrayCreator<PLY_USHORT,3>: public ArrayFactory
{
    virtual osg::Array * getArray(){ return new osg::Vec4usArray; }
    virtual void addElement(char * dataptr, osg::Array* arr){ unsigned short *ptr=(unsigned short*)dataptr; static_cast<osg::Vec4usArray*>(arr)->push_back(osg::Vec4us(ptr[0],ptr[1],ptr[2],ptr[3])); }
};

//
template<> struct ArrayCreator<PLY_INT,0>: public ArrayFactory
{
    virtual osg::Array * getArray(){ return new osg::IntArray; }
    virtual void addElement(char * dataptr,osg::Array* arr){ int *ptr=(int*)dataptr; static_cast<osg::IntArray*>(arr)->push_back(ptr[0]); }
};
template<> struct ArrayCreator<PLY_INT,1>: public ArrayFactory
{
    virtual osg::Array * getArray(){ return new osg::Vec2iArray; }
    virtual void addElement(char * dataptr, osg::Array* arr){ int *ptr=(int*)dataptr; static_cast<osg::Vec2iArray*>(arr)->push_back(osg::Vec2i(ptr[0],ptr[1])); }
};
template<> struct ArrayCreator<PLY_INT,2>: public ArrayFactory
{
    virtual osg::Array * getArray(){ return new osg::Vec3iArray; }
    virtual void addElement(char * dataptr, osg::Array* arr){ int *ptr=(int*)dataptr; static_cast<osg::Vec3iArray*>(arr)->push_back(osg::Vec3i(ptr[0],ptr[1],ptr[2])); }
};
template<> struct ArrayCreator<PLY_INT,3>: public ArrayFactory
{
    virtual osg::Array * getArray(){ return new osg::Vec4iArray; }
    virtual void addElement(char * dataptr, osg::Array* arr){ int *ptr=(int*)dataptr; static_cast<osg::Vec4iArray*>(arr)->push_back(osg::Vec4i(ptr[0],ptr[1],ptr[2],ptr[3])); }
};
//
template<> struct ArrayCreator<PLY_UINT,0>: public ArrayFactory
{
    virtual osg::Array * getArray(){ return new osg::UIntArray; }
    virtual void addElement(char * dataptr,osg::Array* arr){ unsigned int *ptr=(unsigned int*)dataptr; static_cast<osg::UIntArray*>(arr)->push_back(ptr[0]); }
};
template<> struct ArrayCreator<PLY_UINT,1>: public ArrayFactory
{
    virtual osg::Array * getArray(){ return new osg::Vec2uiArray; }
    virtual void addElement(char * dataptr, osg::Array* arr){ unsigned int *ptr=(unsigned int*)dataptr; static_cast<osg::Vec2uiArray*>(arr)->push_back(osg::Vec2ui(ptr[0],ptr[1])); }
};
template<> struct ArrayCreator<PLY_UINT,2>: public ArrayFactory
{
    virtual osg::Array * getArray(){ return new osg::Vec3uiArray; }
    virtual void addElement(char * dataptr, osg::Array* arr){ unsigned int *ptr=(unsigned int*)dataptr; static_cast<osg::Vec3uiArray*>(arr)->push_back(osg::Vec3ui(ptr[0],ptr[1],ptr[2])); }
};
template<> struct ArrayCreator<PLY_UINT,3>: public ArrayFactory
{
    virtual osg::Array * getArray(){ return new osg::Vec4uiArray; }
    virtual void addElement(char * dataptr, osg::Array* arr){ unsigned int *ptr=(unsigned int*)dataptr; static_cast<osg::Vec4uiArray*>(arr)->push_back(osg::Vec4ui(ptr[0],ptr[1],ptr[2],ptr[3])); }
};

//
template<> struct ArrayCreator<PLY_FLOAT,0>: public ArrayFactory
{
    virtual osg::Array * getArray(){ return new osg::FloatArray; }
    virtual void addElement(char * dataptr,osg::Array* arr){ float *ptr=(float*)dataptr; static_cast<osg::FloatArray*>(arr)->push_back(ptr[0]); }
};
template<> struct ArrayCreator<PLY_FLOAT,1>: public ArrayFactory
{
    virtual osg::Array * getArray(){ return new osg::Vec2Array; }
    virtual void addElement(char * dataptr, osg::Array* arr){ float *ptr=(float*)dataptr; static_cast<osg::Vec2Array*>(arr)->push_back(osg::Vec2(ptr[0],ptr[1])); }
};
template<> struct ArrayCreator<PLY_FLOAT,2>: public ArrayFactory
{
    virtual osg::Array * getArray(){ return new osg::Vec3Array; }
    virtual void addElement(char * dataptr, osg::Array* arr){ float *ptr=(float*)dataptr; static_cast<osg::Vec3Array*>(arr)->push_back(osg::Vec3(ptr[0],ptr[1],ptr[2])); }
};
template<> struct ArrayCreator<PLY_FLOAT,3>: public ArrayFactory
{
    virtual osg::Array * getArray(){ return new osg::Vec4Array; }
    virtual void addElement(char * dataptr, osg::Array* arr){ float *ptr=(float*)dataptr; static_cast<osg::Vec4Array*>(arr)->push_back(osg::Vec4(ptr[0],ptr[1],ptr[2],ptr[3])); }
};

//
template<> struct ArrayCreator<PLY_DOUBLE,0>: public ArrayFactory
{
    virtual osg::Array * getArray(){ return new osg::DoubleArray; }
    virtual void addElement(char * dataptr,osg::Array* arr){ double *ptr=(double*)dataptr; static_cast<osg::DoubleArray*>(arr)->push_back(ptr[0]); }
};
template<> struct ArrayCreator<PLY_DOUBLE,1>: public ArrayFactory
{
    virtual osg::Array * getArray(){ return new osg::Vec2dArray; }
    virtual void addElement(char * dataptr, osg::Array* arr){ double *ptr=(double*)dataptr; static_cast<osg::Vec2dArray*>(arr)->push_back(osg::Vec2d(ptr[0],ptr[1])); }
};
template<> struct ArrayCreator<PLY_DOUBLE,2>: public ArrayFactory
{
    virtual osg::Array * getArray(){ return new osg::Vec3dArray; }
    virtual void addElement(char * dataptr, osg::Array* arr){ double *ptr=(double*)dataptr; static_cast<osg::Vec3dArray*>(arr)->push_back(osg::Vec3d(ptr[0],ptr[1],ptr[2])); }
};
template<> struct ArrayCreator<PLY_DOUBLE,3>: public ArrayFactory
{
    virtual osg::Array * getArray(){ return new osg::Vec4dArray; }
    virtual void addElement(char * dataptr, osg::Array* arr){ double *ptr=(double*)dataptr; static_cast<osg::Vec4dArray*>(arr)->push_back(osg::Vec4d(ptr[0],ptr[1],ptr[2],ptr[3])); }
};

/*  Constructor.  */
VertexData::VertexData(const VertexSemantics& s)
    : _semantics(s), _invertFaces( false )
{
    // Initialize array factories
    _prfactories[PLY_CHAR] = _prfactories[PLY_UCHAR] = _prfactories[PLY_UINT8] = new DrawElementCreator<PLY_UCHAR>;
    _prfactories[PLY_INT] = _prfactories[PLY_UINT] = new DrawElementCreator<PLY_UINT>;
    _prfactories[PLY_USHORT] = _prfactories[PLY_SHORT] = new DrawElementCreator<PLY_USHORT>;

    _arrayfactories[PLY_CHAR][0] = new ArrayCreator<PLY_CHAR,0>;
    _arrayfactories[PLY_CHAR][1] = new ArrayCreator<PLY_CHAR,1>;
    _arrayfactories[PLY_CHAR][2] = new ArrayCreator<PLY_CHAR,2>;
    _arrayfactories[PLY_CHAR][3] = new ArrayCreator<PLY_CHAR,3>;

    _arrayfactories[PLY_UCHAR][0] = _arrayfactories[PLY_UINT8][0] = new ArrayCreator<PLY_UCHAR,0>;
    _arrayfactories[PLY_UCHAR][1] = _arrayfactories[PLY_UINT8][1] = new ArrayCreator<PLY_UCHAR,1>;
    _arrayfactories[PLY_UCHAR][2] = _arrayfactories[PLY_UINT8][2] = new ArrayCreator<PLY_UCHAR,2>;
    _arrayfactories[PLY_UCHAR][3] = _arrayfactories[PLY_UINT8][3] = new ArrayCreator<PLY_UCHAR,3>;

    _arrayfactories[PLY_SHORT][0] = new ArrayCreator<PLY_SHORT,0>;
    _arrayfactories[PLY_SHORT][1] = new ArrayCreator<PLY_SHORT,1>;
    _arrayfactories[PLY_SHORT][2] = new ArrayCreator<PLY_SHORT,2>;
    _arrayfactories[PLY_SHORT][3] = new ArrayCreator<PLY_SHORT,3>;

    _arrayfactories[PLY_USHORT][0] = new ArrayCreator<PLY_USHORT,0>;
    _arrayfactories[PLY_USHORT][1] = new ArrayCreator<PLY_USHORT,1>;
    _arrayfactories[PLY_USHORT][2] = new ArrayCreator<PLY_USHORT,2>;
    _arrayfactories[PLY_USHORT][3] = new ArrayCreator<PLY_USHORT,3>;

    _arrayfactories[PLY_INT][0] = _arrayfactories[PLY_UINT][0] = new ArrayCreator<PLY_INT,0>;
    _arrayfactories[PLY_INT][1] = _arrayfactories[PLY_UINT][1] = new ArrayCreator<PLY_INT,1>;
    _arrayfactories[PLY_INT][2] = _arrayfactories[PLY_UINT][2] = new ArrayCreator<PLY_INT,2>;
    _arrayfactories[PLY_INT][3] = _arrayfactories[PLY_UINT][3] = new ArrayCreator<PLY_INT,3>;

    _arrayfactories[PLY_UINT][0] = new ArrayCreator<PLY_UINT,0>;
    _arrayfactories[PLY_UINT][1] = new ArrayCreator<PLY_UINT,1>;
    _arrayfactories[PLY_UINT][2] = new ArrayCreator<PLY_UINT,2>;
    _arrayfactories[PLY_UINT][3] = new ArrayCreator<PLY_UINT,3>;

    _arrayfactories[PLY_FLOAT][0] = _arrayfactories[PLY_FLOAT32][0] = new ArrayCreator<PLY_FLOAT,0>;
    _arrayfactories[PLY_FLOAT][1] = _arrayfactories[PLY_FLOAT32][1] = new ArrayCreator<PLY_FLOAT,1>;
    _arrayfactories[PLY_FLOAT][2] = _arrayfactories[PLY_FLOAT32][2] = new ArrayCreator<PLY_FLOAT,2>;
    _arrayfactories[PLY_FLOAT][3] = _arrayfactories[PLY_FLOAT32][3] = new ArrayCreator<PLY_FLOAT,3>;

    _arrayfactories[PLY_DOUBLE][0] = new ArrayCreator<PLY_DOUBLE,0>;
    _arrayfactories[PLY_DOUBLE][1] = new ArrayCreator<PLY_DOUBLE,1>;
    _arrayfactories[PLY_DOUBLE][2] = new ArrayCreator<PLY_DOUBLE,2>;
    _arrayfactories[PLY_DOUBLE][3] = new ArrayCreator<PLY_DOUBLE,3>;
}


/*  Read the index data from the open file.  */
void VertexData::readListProperty( PlyFile* file, const int nFaces, char*  elemName, PlyProperty* listprop )
{
    PFactAndDrawElements & out = _factoryarrayspair.back().second.second;
    //PFactAndDrawElement triangles(_prfactories[/*listprop->internal_type*/PLY_UINT], _prfactories[/*listprop->internal_type*/PLY_UINT]->getDrawElement());
    //PFactAndDrawElement quads(_prfactories[/*listprop->internal_type*/PLY_UINT], _prfactories[/*listprop->internal_type*/PLY_UINT]->getDrawElement());
    osg::ref_ptr<osg::DrawElementsUInt> triangles = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES);
    osg::ref_ptr<osg::DrawElementsUInt> quads = new osg::DrawElementsUInt(osg::PrimitiveSet::QUADS);
    // temporary face structure for ply loading
    struct _Face
    {
        unsigned int   nVertices;
        unsigned int*   vertices;
    } face;

    PlyProperty faceProps[] =
    {
        { listprop->name, listprop->external_type, PLY_UINT, offsetof( _Face, vertices ),
          1, listprop->count_external, PLY_UINT, offsetof( _Face, nVertices ) }
    };

    ply_get_property( file,elemName, &faceProps[0] );

    const char NUM_VERTICES_TRIANGLE(3);
    const char NUM_VERTICES_QUAD(4);

    // read the faces, reversing the reading direction if _invertFaces is true
    for( int i = 0 ; i < nFaces; i++ )
    {
        // initialize face values
        face.nVertices = 0;
        face.vertices = 0;

        ply_get_element( file, static_cast< void* >( &face ) );
        if (face.vertices)
        {
            if (face.nVertices == NUM_VERTICES_TRIANGLE ||  face.nVertices == NUM_VERTICES_QUAD)
            {
                unsigned short index;
                for(int j = 0 ; j < face.nVertices ; j++)
                {
                    index = ( _invertFaces ? face.nVertices - 1 - j : j );
                    if(face.nVertices == 4)
                        quads->push_back(face.vertices[index]);
                    else
                        triangles->push_back(face.vertices[index]);
                }
            }
            // free the memory that was allocated by ply_get_element
            free( face.vertices );
        }
    }
    if(!triangles->empty())
        out.push_back(PFactAndDrawElement(0, triangles));
    if(!quads->empty())
        out.push_back(PFactAndDrawElement(0, quads));

}

inline osg::Array * getArrayFromFactory(ArrayFactory*factarray, int curchannel)
{
        osg::Array* arr = factarray->getArray();
        arr->setUserData(new osg::IntValueObject(curchannel));
        return arr;
}

/*  Read the vertex and (if available/wanted) color data from the open file.  */
void VertexData::readVertices( PlyFile* file, const int nVertices,char*  elemName, PlyProperty** props, int numprops)
{
    // read in the vertices
    std::vector<int> propertyOffsets;
    unsigned int totalvertexsize = 0;

    std::pair<std::string, APFactAndArrays> newarrayvector(elemName, APFactAndArrays());
    _factoryarrayspair.push_back(newarrayvector);
    AFactAndArrays &factoryarrays = _factoryarrayspair.back().second.first;
    {
        int curchannel = -1, numcomp = 0, curcompsize = 0; const VertexSemantic* cursem = 0;
        for(int propcpt=0; propcpt < numprops; ++propcpt)
        {
        //search for prop in user semantics and goto next if not found (TODO add semantics through reader options)
            VertexSemantics::iterator semit;
            for( semit =_semantics.begin(); semit!=_semantics.end() &&strcmp(props[propcpt]->name,semit->name)!=0; ++semit);
            if(semit==_semantics.end()) continue;
            VertexSemantic &sem = *semit;
            if(props[propcpt]->is_list )
            {
                readListProperty(  file, nVertices, elemName, props[propcpt]);
                continue;
            }

            //setup user property
            props[propcpt]->offset = totalvertexsize+curcompsize;
            props[propcpt]->internal_type = sem.internal_type;
            ply_get_property( file, elemName, props[propcpt]);


            if(curchannel != sem.osgmapping)
            {
                if(numcomp != 0)
                {
                    if(numcomp>4) { OSG_FATAL<<"osg ply plugin doesn't support "<<numcomp<<" components arrays, trying 4 instead"<<std::endl; numcomp = 4; }
                    ArrayFactory * factarray = _arrayfactories[cursem->internal_type][numcomp-1];
                    if(factarray) factoryarrays.push_back(AFactAndArray(factarray, getArrayFromFactory(factarray, curchannel)));
                    propertyOffsets.push_back(totalvertexsize);
                    totalvertexsize +=  curcompsize;
                }
                numcomp = 0; curcompsize = 0; cursem = &sem;
                curchannel = sem.osgmapping;
            }
            numcomp++;
            switch (sem.internal_type)
            {
                case PLY_UINT8:    curcompsize+=sizeof(uint8_t);        break;
                case PLY_UCHAR:    curcompsize+=sizeof(unsigned char);  break;
                case PLY_CHAR:     curcompsize+=sizeof(char);           break;
                case PLY_USHORT:   curcompsize+=sizeof(unsigned short); break;
                case PLY_SHORT:    curcompsize+=sizeof(short);          break;
                case PLY_UINT:     curcompsize+=sizeof(unsigned int);   break;
                case PLY_INT:      curcompsize+=sizeof(int);            break;
                case PLY_FLOAT:    curcompsize+=sizeof(float);          break;
                case PLY_DOUBLE:   curcompsize+=sizeof(double);         break;
                case PLY_INT32:    curcompsize+=sizeof(int32_t);        break;
                case PLY_FLOAT32:  curcompsize+=4;                      break;
                default:
                    OSG_WARN<<"ply::VertexData: unknown internal_type" <<sem.internal_type<<" not implemented,"<<std::endl;
            }

        }
        if(numcomp != 0){
            if(numcomp>4) { OSG_FATAL<<"osg ply plugin doesn't support "<<numcomp<<" components arrays, trying 4 instead"<<std::endl; numcomp = 4; }
            ArrayFactory * factarray = _arrayfactories[cursem->internal_type][numcomp-1];
            if(factarray) factoryarrays.push_back(AFactAndArray(factarray, getArrayFromFactory(factarray, curchannel)));
            propertyOffsets.push_back(totalvertexsize);
            totalvertexsize +=  curcompsize;
        }
    }
    if(totalvertexsize>0)
    {
        char * rawvertex= new char[totalvertexsize];
        for( int i = 0; i < nVertices; ++i )
        {
            ply_get_element( file, static_cast< void* >( rawvertex ) );

            ///convert rawvertex to osg
            int curprop = 0;
            for(std::vector<AFactAndArray>::iterator arrit = factoryarrays.begin(); arrit != factoryarrays.end(); ++arrit)
            {
                arrit->first->addElement((char*)rawvertex+propertyOffsets[curprop++], arrit->second);
            }
        }
        delete rawvertex;
    }
}

/*  Open a PLY file and read vertex, color and index data. and returns the node  */
osg::Node* VertexData::readPlyFile( const char* filename, const bool ignoreColors )
{
    int     nPlyElems;
    char**  elemNames;
    int     fileType;
    float   version;
    bool    result = false;
    int     nComments;
    char**  comments;

    PlyFile* file = NULL;

    // Try to open ply file as for reading
    try
    {
            file  = ply_open_for_reading( const_cast< char* >( filename ),
                                          &nPlyElems, &elemNames,
                                          &fileType, &version );
    }
    // Catch the if any exception thrown
    catch( exception& e )
    {
        MESHERROR << "Unable to read PLY file, an exception occurred:  "
                    << e.what() << endl;
    }

    if( !file )
    {
        MESHERROR << "Unable to open PLY file " << filename
                  << " for reading." << endl;
        return NULL;
    }

    MESHASSERT( elemNames != 0 );


    nComments = file->num_comments;
    comments = file->comments;


    #ifndef NDEBUG
    MESHINFO << filename << ": " << nPlyElems << " elements, file type = "
             << fileType << ", version = " << version << endl;
    #endif

    std::string textureFile;
    for( int i = 0; i < nComments; i++ )
    {
        if( equal_strings( comments[i], "InvertFaces" ) )
        {
            _invertFaces = true;
        }
        if (strncmp(comments[i], "TextureFile", 11)==0)
        {
            textureFile = comments[i] + 12;
            if (!osgDB::isAbsolutePath(textureFile))
            {
                textureFile = osgDB::concatPaths(osgDB::getFilePath(filename), textureFile);
            }
        }
    }
    for( int i = 0; i < nPlyElems; ++i )
    {
        int nElems;
        int nProps;

        PlyProperty** props = NULL;
        try{
                props = ply_get_element_description( file, elemNames[i],
                                                     &nElems, &nProps );
        }
        catch( exception& e )
        {
            MESHERROR << "Unable to get PLY file description, an exception occurred:  "
                        << e.what() << endl;
        }
        MESHASSERT( props != 0 );

        #ifndef NDEBUG
        MESHINFO << "element " << i << ": name = " << elemNames[i] << ", "
                 << nProps << " properties, " << nElems << " elements" << endl;
        for( int j = 0; j < nProps; ++j )
        {
            MESHINFO << "element " << i << ", property " << j << ": "
                     << "name = " << props[j]->name << endl;
        }
        #endif

        {
            try {
                // Read vertices and store in a std::vector array
                readVertices( file, nElems, elemNames[i], props, nProps );
                result = true;
            }
            catch( exception& e )
            {
                MESHERROR << "Unable to read vertex in PLY file, an exception occurred:  "
                            << e.what() << endl;
                // stop for loop by setting the loop variable to break condition
                // this way resources still get released even on error cases
                i = nPlyElems;

            }
        }       

        // free the memory that was allocated by ply_get_element_description
        for( int j = 0; j < nProps; ++j )
            free( props[j] );
        free( props );
    }

    ply_close( file );

    // free the memory that was allocated by ply_open_for_reading
    for( int i = 0; i < nPlyElems; ++i )
        free( elemNames[i] );
    free( elemNames );

   // If the result is true means the ply file is successfully read
   if(result)
   {
        // Create geometry node
        osg::Geometry* geom  =  new osg::Geometry;

        // Add the primitive set
        bool hasTriOrQuads = false;
        if (_triangles.valid() && _triangles->size() > 0 )
        {
            geom->addPrimitiveSet(_triangles.get());
            hasTriOrQuads = true;
        }

        if (_quads.valid() && _quads->size() > 0 )
        {
            geom->addPrimitiveSet(_quads.get());
            hasTriOrQuads = true;
        }
        // Assuming First Element is vertices
        std::vector< std::pair<std::string, APFactAndArrays> >::iterator elementarraysit = _factoryarrayspair.begin();
        AFactAndArrays &factoryarrays = elementarraysit->second.first;
        int numvertices = factoryarrays[0].second->getNumElements();


       for(std::vector<AFactAndArray>::iterator arrit = factoryarrays.begin(); arrit != factoryarrays.end(); ++arrit)
       {
            osg::Array* a = arrit->second;
            int index = static_cast<osg::IntValueObject*>(a->getUserData())->getValue();
            geom->setVertexAttribArray(index, a, osg::Array::BIND_PER_VERTEX );
            a->setUserData(NULL);
        }

        // Assuming Second Element has a list with primitiveset indices
        elementarraysit++;
        PFactAndDrawElements &factoryprs = elementarraysit->second.second;

        // Print points if the file contains unsupported primitives
        if(factoryprs.empty())
            geom->addPrimitiveSet(new osg::DrawArrays(GL_POINTS, 0, numvertices));

        for(std::vector<PFactAndDrawElement>::iterator arrit = factoryprs.begin(); arrit != factoryprs.end(); ++arrit)
        {
            osg::DrawElements* a = arrit->second;
            geom->addPrimitiveSet(a);
            a->setUserData(NULL);
        }

        for(; elementarraysit!=_factoryarrayspair.end(); ++elementarraysit)
        {
            factoryarrays = elementarraysit->second.first;
            for(std::vector<AFactAndArray>::iterator arrit = factoryarrays.begin(); arrit != factoryarrays.end(); ++arrit)
            {
                 osg::Array* a = arrit->second;
                 geom->getOrCreateUserDataContainer()->addUserObject(a);
                 a->setUserData(NULL);
             }
            factoryprs = elementarraysit->second.second;
            for(std::vector<PFactAndDrawElement>::iterator arrit = factoryprs.begin(); arrit != factoryprs.end(); ++arrit)
            {
                osg::DrawElements* a = arrit->second;
                geom->getOrCreateUserDataContainer()->addUserObject(a);
                a->setUserData(NULL);
            }
        }
        // set flage true to activate the vertex buffer object of drawable
        geom->setUseVertexBufferObjects(true);

        osg::ref_ptr<osg::Image> image;
        if (!textureFile.empty() && (image = osgDB::readRefImageFile(textureFile)) != NULL)
        {
            osg::Texture2D *texture = new osg::Texture2D;
            texture->setImage(image.get());
            texture->setResizeNonPowerOfTwoHint(false);

            osg::TexEnv *texenv = new osg::TexEnv;
            texenv->setMode(osg::TexEnv::REPLACE);

            osg::StateSet *stateset = geom->getOrCreateStateSet();
            stateset->setTextureAttributeAndModes(0, texture, osg::StateAttribute::ON);
            stateset->setTextureAttribute(0, texenv);
        }

        osg::Geode* geode = new osg::Geode;
        geode->addDrawable(geom);
        return geode;
    }

    return NULL;
}


