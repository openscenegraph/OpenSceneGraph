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

#include <osg/GeoSet>
#include <osg/StateSet>
#include <osg/Material>

#include <osgDB/Registry>

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

    virtual osg::Node* readNode(const std::string& fileName, const osgDB::ReaderWriter::Options*);

protected:
};


// register with Registry to instantiate the above reader/writer.
osgDB::RegisterReaderWriterProxy<ReaderWriterLWO> g_lwoReaderWriterProxy;


// read file and convert to OSG.
osg::Node* ReaderWriterLWO::readNode(const std::string& fileName, const osgDB::ReaderWriter::Options*)
{
    lwObject* lw = lw_object_read(fileName.c_str());
    if (!lw)
        return NULL;

    osg::notify(osg::INFO) << "faces " << lw->face_cnt << endl;
    osg::notify(osg::INFO) << "materials " << lw->material_cnt << endl;
    osg::notify(osg::INFO) << "vertices " << lw->vertex_cnt << endl;

    // shared coordinates
    typedef std::map<int,osgUtil::Tesselator::IndexVec> MaterialTriangles;
    MaterialTriangles mts;
    

    osgUtil::Tesselator tess;
    for (int i = 0; i < lw->face_cnt; i++)
    {
        if (lw->face[i].index_cnt>=3)
        {
            tess.tesselate((osg::Vec3*)lw->vertex,lw->face[i].index_cnt,lw->face[i].index,osgUtil::Tesselator::CLOCK_WISE);
            osgUtil::Tesselator::IndexVec& iv = mts[lw->face[i].material];
            const osgUtil::Tesselator::IndexVec& result = tess.getResult();
            iv.insert(iv.end(),result.begin(),result.end());
        }
    }
    
    typedef std::vector<int> CoordIndexMap;
    CoordIndexMap cim(lw->vertex_cnt,-1);
    
    // create geode, to fill in for loop below, a geoset per material.
    osg::Geode* geode = new osg::Geode;

    osgUtil::SmoothingVisitor smoother;

    for(MaterialTriangles::iterator itr=mts.begin();
        itr!=mts.end();
        ++itr)
    {
        // set up material
        lwMaterial& lw_material = lw->material[itr->first];
        osg::Material* osg_material = new osg::Material;

        osg::Vec4 diffuse(lw_material.r,
                          lw_material.g,
                          lw_material.b,
                          1.0f);
        osg::Vec4 ambient(lw_material.r * 0.25f,
                          lw_material.g * 0.25f,
                          lw_material.b * 0.25f,
                          1.0f);

        osg_material->setAmbient(osg::Material::FRONT_AND_BACK, ambient);
        osg_material->setDiffuse(osg::Material::FRONT_AND_BACK, diffuse);

        // create state
        osg::StateSet* state = new osg::StateSet;
        state->setAttribute(osg_material);
        
        // intialize cim vector with -1 to represent unassigned vertices.
        std::fill(cim.begin(),cim.end(),-1);
        
        osgUtil::Tesselator::IndexVec& iv = itr->second;
        
        // first fill in the references.
        int vertexCount = 0;
        osgUtil::Tesselator::IndexVec::iterator iv_itr;
        for(iv_itr=iv.begin();
            iv_itr!=iv.end();
            ++iv_itr)
        {
            int i = *iv_itr;
            if (cim[i]<0) cim[i] = vertexCount++;
        }

        // copy across lw shared coords into local osg coords.
        osg::Vec3* coord = new osg::Vec3[vertexCount];
        for (int i=0;i<lw->vertex_cnt;++i)
        {
			// From the spec_low.lxt :
			//   "By convention, the +X direction is to the right or east, the +Y
			//    direction is upward, and the +Z direction is forward or north"
			// However, the osg sticks to the more conventional, y to the north,
			// z upwards, x is the same - rigth/east.  To handle this difference
			// simple exchange osg_z for lwo_y, and osg_y for lwo_z.

            if (cim[i]>=0) coord[cim[i]].set(lw->vertex[i*3], lw->vertex[i*3+2], lw->vertex[i*3+1]);
        }

        // copy across indices and remap them to the local coords offsets.        
        osg::ushort* cindex = new osg::ushort[iv.size()];
        osg::ushort* cindex_ptr = cindex;
        for (iv_itr=iv.begin();
            iv_itr!=iv.end();
            ++iv_itr)
        {
            (*cindex_ptr) = cim[*iv_itr];
            ++cindex_ptr;
        }
        
        int numPrim = iv.size()/3;
        
        // create geoset
        
        osg::GeoSet* gset = new osg::GeoSet;
        gset->setPrimType(osg::GeoSet::TRIANGLES);
        gset->setNumPrims(numPrim);
        gset->setCoords(coord, cindex);
        gset->setStateSet(state);

        smoother.smooth(*gset);

        geode->addDrawable(gset);

    }

    // free
    lw_object_free(lw);

    return geode;
}
