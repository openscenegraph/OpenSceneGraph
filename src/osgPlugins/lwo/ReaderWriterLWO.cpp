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
#include <osg/Texture>
#include <osg/Geometry>
#include <osg/StateSet>

#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <osgUtil/SmoothingVisitor>
#include <osgUtil/Tesselator>

#include "lw.h"

class ReaderWriterLWO : public osgDB::ReaderWriter
{
public:
    ReaderWriterLWO() { }

    virtual const char* className() { return "Lightwave Object Reader"; }
    virtual bool acceptsExtension(const std::string& extension) {
        return (extension == "lwo" || extension == "lw" || extension == "geo");
    }

    virtual ReadResult readNode(const std::string& fileName, const osgDB::ReaderWriter::Options*);

protected:
};


// register with Registry to instantiate the above reader/writer.
osgDB::RegisterReaderWriterProxy<ReaderWriterLWO> g_lwoReaderWriterProxy;


// collect all the data relavent to a particular osg::Geometry being created.
struct GeometryCollection
{
    GeometryCollection():
        _numPrimitives(0),
        _numPrimitivesWithTexCoords(0),
        _numPoints(0),
        _texturesActive(false),
        _vertices(0),
        _texcoords(0),
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
osgDB::ReaderWriter::ReadResult ReaderWriterLWO::readNode(const std::string& fileName, const osgDB::ReaderWriter::Options*)
{
    lwObject* lw = lw_object_read(fileName.c_str());
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

            gc._geom = new osg::Geometry;
            
            osg::Vec3Array* vertArray = new osg::Vec3Array(gc._numPoints);
            gc._vertices = vertArray->begin();
            gc._geom->setVertexArray(vertArray);

            // set up color.
            osg::Vec4Array* colors = new osg::Vec4Array(1);
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
                        osg::StateSet* stateset = new osg::StateSet;

                        osg::Texture* texture = new osg::Texture;
                        texture->setImage(image);
                        
                        stateset->setAttributeAndModes(texture,osg::StateAttribute::ON);
                        gc._texturesActive=true;
                        
                        gc._geom->setStateSet(stateset);

                        osg::Vec2Array* texcoordArray = new osg::Vec2Array(gc._numPoints);
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
            
            gc._geom->addPrimitive(new osg::DrawArrays(osg::Primitive::POLYGON,gc._coordCount,face.index_cnt));
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
            
            if (gc._texcoords && face.texcoord)
            {
                for(j=face.index_cnt-1;j>=0;--j)
                {
                    (*gc._texcoords++).set(face.texcoord[j*2],face.texcoord[j*2+1]);
                }            
            }            
        }
    }

    osg::Geode* geode = new osg::Geode;
    
    // add everthing into the Geode.    
    osgUtil::SmoothingVisitor smoother;
    for(itr=mtgcm.begin();
        itr!=mtgcm.end();
        ++itr)
    {
        GeometryCollection& gc = itr->second;
        if (gc._geom)
        {
            smoother.smooth(*gc._geom);
            geode->addDrawable(gc._geom);
        }

    }

    // free
    lw_object_free(lw);

    return geode;
}
