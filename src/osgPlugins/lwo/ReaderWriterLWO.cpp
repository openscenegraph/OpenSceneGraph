// -*-c++-*-

/*
 * Lightwave Object loader for Open Scene Graph
 *
 * Copyright (C) 2001 Ulrich Hertlein <u.hertlein@web.de>
 *
 * The Open Scene Graph (OSG) is a cross platform C++/OpenGL library for 
 * real-time rendering of large 3D photo-realistic models. 
 * The OSG homepage is http://www.openscenegraph.org/
 */

#if defined(_MSC_VER)
    #pragma warning( disable : 4786 )
#endif

#include <string>
#include <algorithm>

#include <osg/Notify>
#include <osg/Node>
#include <osg/Group>
#include <osg/Geode>
#include <osg/Group>
#include <osg/Texture2D>
#include <osg/Geometry>
#include <osg/StateSet>

#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <osgUtil/SmoothingVisitor>
#include <osgUtil/Tesselator>

#include "lw.h"
#include "Lwo2.h"

class ReaderWriterLWO : public osgDB::ReaderWriter
{
public:
    ReaderWriterLWO() { }

    virtual const char* className() { return "Lightwave Object Reader"; }
    virtual bool acceptsExtension(const std::string& extension) {
        return (extension == "lwo" || extension == "lw" || extension == "geo");
    }

    virtual ReadResult readNode(const std::string& fileName, const osgDB::ReaderWriter::Options* options)
    {
        ReadResult result = readNode_LWO1(fileName,options);
        if (result.success()) return result;
        
        return readNode_LWO2(fileName,options);
    }

    virtual ReadResult readNode_LWO2(const std::string& fileName, const osgDB::ReaderWriter::Options*);
    virtual ReadResult readNode_LWO1(const std::string& fileName, const osgDB::ReaderWriter::Options*);

protected:

    

};


// register with Registry to instantiate the above reader/writer.
osgDB::RegisterReaderWriterProxy<ReaderWriterLWO> g_lwoReaderWriterProxy;


osgDB::ReaderWriter::ReadResult ReaderWriterLWO::readNode_LWO2(const std::string& fileName, const osgDB::ReaderWriter::Options*)
{
    std::auto_ptr<Lwo2> lwo2(osgNew Lwo2());
    if (lwo2->ReadFile(fileName))
    {
        osg::ref_ptr<Group> group = osgNew osg::Group();
        if (lwo2->GenerateGroup(*group)) return group.take();
    }
    return ReadResult::FILE_NOT_HANDLED;
}





// collect all the data relavent to a particular osg::Geometry being created.
struct GeometryCollection
{
    GeometryCollection():
        _numPrimitives(0),
        _numPrimitivesWithTexCoords(0),
        _numPoints(0),
        _texturesActive(false),
        _vertices(osg::Vec3Array::iterator()),
        _texcoords(osg::Vec2Array::iterator()),
        _coordCount(0),
        _geom(0) {}

    int                         _numPrimitives;
    int                         _numPrimitivesWithTexCoords;
    int                         _numPoints;
    bool                        _texturesActive;
    osg::Vec3Array::iterator    _vertices;
    osg::Vec2Array::iterator    _texcoords;
    int                         _coordCount;
    osg::Geometry*              _geom;
};



// read file and convert to OSG.
osgDB::ReaderWriter::ReadResult ReaderWriterLWO::readNode_LWO1(const std::string& fileName, const osgDB::ReaderWriter::Options*)
{
    lwObject* lw = lw_object_read(fileName.c_str(),osg::notify(osg::INFO));
    if (!lw)
        return ReadResult::FILE_NOT_HANDLED;

    osg::notify(osg::INFO) << "faces " << lw->face_cnt << std::endl;
    osg::notify(osg::INFO) << "materials " << lw->material_cnt << std::endl;
    osg::notify(osg::INFO) << "vertices " << lw->vertex_cnt << std::endl;

    typedef std::map<int,GeometryCollection> MaterialToGeometryCollectionMap;
    MaterialToGeometryCollectionMap mtgcm;
    
    // bin the indices for each material into the mtis;
    int i;
    for (i = 0; i < lw->face_cnt; ++i)
    {
        lwFace& face = lw->face[i];
        if (face.index_cnt>=3)
        {
            GeometryCollection& gc = mtgcm[face.material];
            gc._numPoints += face.index_cnt;
            gc._numPrimitives += 1;
            if (face.texcoord) gc._numPrimitivesWithTexCoords += 1;
        }
    }
    
    MaterialToGeometryCollectionMap::iterator itr;
    for(itr=mtgcm.begin();
        itr!=mtgcm.end();
        ++itr)
    {
        GeometryCollection& gc = itr->second;

        if (gc._numPrimitives)        
        {
            lwMaterial& lw_material = lw->material[itr->first];

            gc._geom = osgNew osg::Geometry;
            
            osg::Vec3Array* vertArray = osgNew osg::Vec3Array(gc._numPoints);
            gc._vertices = vertArray->begin();
            gc._geom->setVertexArray(vertArray);

            // set up color.
            osg::Vec4Array* colors = osgNew osg::Vec4Array(1);
            (*colors)[0].set(lw_material.r,
                             lw_material.g,
                             lw_material.b,
                             1.0f);
                             
            gc._geom->setColorArray(colors);
            gc._geom->setColorBinding(osg::Geometry::BIND_OVERALL);
    
            // set up texture if needed.
            if (gc._numPrimitivesWithTexCoords==gc._numPrimitives)
            {
                if (strlen(lw_material.name)!=0)
                {
                    osg::Image* image = osgDB::readImageFile(lw_material.name);
                    if (image)
                    {
                        // create state
                        osg::StateSet* stateset = osgNew osg::StateSet;

                        osg::Texture2D* texture = osgNew osg::Texture2D;
                        texture->setImage(image);
                        
                        stateset->setTextureAttributeAndModes(0,texture,osg::StateAttribute::ON);
                        gc._texturesActive=true;
                        
                        gc._geom->setStateSet(stateset);

                        osg::Vec2Array* texcoordArray = osgNew osg::Vec2Array(gc._numPoints);
                        gc._texcoords = texcoordArray->begin();
                        gc._geom->setTexCoordArray(0,texcoordArray);
                    }
                }
            }
        }        
    }
    
    
    for (i = 0; i < lw->face_cnt; ++i)
    {
        lwFace& face = lw->face[i];
        if (face.index_cnt>=3)
        {
            GeometryCollection& gc = mtgcm[face.material];
            
            osg::PrimitiveSet::Mode mode;
            switch(face.index_cnt)
            {
                case(0):
                    mode = osg::PrimitiveSet::POINTS;
                    break;
                case(1):
                    mode = osg::PrimitiveSet::POINTS;
                    break;
                case(2):
                    mode = osg::PrimitiveSet::LINES;
                    break;
                case(3):
                    mode = osg::PrimitiveSet::TRIANGLES;
                    break;
                case(4):
                    mode = osg::PrimitiveSet::QUADS;
                    break;
                default:
                    mode = osg::PrimitiveSet::POLYGON;
                    break;
            }
                        
            gc._geom->addPrimitive(osgNew osg::DrawArrays(mode,gc._coordCount,face.index_cnt));
            gc._coordCount += face.index_cnt;

            // From the spec_low.lxt :
            //   "By convention, the +X direction is to the right or east, the +Y
            //    direction is upward, and the +Z direction is forward or north"
            // However, the osg sticks to the more conventional, y to the north,
            // z upwards, x is the same - rigth/east.  To handle this difference
            // simple exchange osg_z for lwo_y, and osg_y for lwo_z.

            // add the corners in reverse order to reverse the windings, to keep the anticlockwise rotation of polys.
            int j;
            for(j=face.index_cnt-1;j>=0;--j)
            {
                (*gc._vertices++).set(lw->vertex[face.index[j]*3], lw->vertex[face.index[j]*3+2], lw->vertex[face.index[j]*3+1]);
            }
            
            if (gc._texturesActive && face.texcoord)
            {
                for(j=face.index_cnt-1;j>=0;--j)
                {
                    (*gc._texcoords++).set(face.texcoord[j*2],face.texcoord[j*2+1]);
                }            
            }            
        }
    }

    osg::Geode* geode = osgNew osg::Geode;
    
    osgUtil::Tesselator tesselator;
    
    // add everthing into the Geode.    
    osgUtil::SmoothingVisitor smoother;
    for(itr=mtgcm.begin();
        itr!=mtgcm.end();
        ++itr)
    {
        GeometryCollection& gc = itr->second;
        if (gc._geom)
        {
            
            tesselator.retesselatePolygons(*gc._geom);
        
            smoother.smooth(*gc._geom);
            
            geode->addDrawable(gc._geom);
        }

    }

    // free
    lw_object_free(lw);

    return geode;
}
