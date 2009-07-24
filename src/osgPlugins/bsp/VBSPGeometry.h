
#ifndef VBSP_GEOMETRY_H
#define VBSP_GEOMETRY_H


#include <osg/Array>
#include <osg/Geometry>

#include "VBSPData.h"


namespace bsp
{


class VBSPGeometry
{
    protected:

        VBSPData *   bsp_data;

        osg::ref_ptr<osg::Vec3Array>          vertex_array;
        osg::ref_ptr<osg::Vec3Array>          normal_array;
        osg::ref_ptr<osg::Vec2Array>          texcoord_array;
        osg::ref_ptr<osg::DrawArrayLengths>   primitive_set;

        osg::ref_ptr<osg::Vec3Array>          disp_vertex_array;
        osg::ref_ptr<osg::Vec3Array>          disp_normal_array;
        osg::ref_ptr<osg::Vec2Array>          disp_texcoord_array;
        osg::ref_ptr<osg::Vec4Array>          disp_vertex_attr_array;
        osg::ref_ptr<osg::DrawElementsUInt>   disp_primitive_set;

        bool         doesEdgeExist(int row, int col, int direction,
                                   int vertsPerEdge);
        osg::Vec3f   getNormalFromEdges(int row, int col,
                                        unsigned char edgeBits,
                                        int firstVertex, int vertsPerEdge);
        void         createDispSurface(Face & face, DisplaceInfo & dispInfo);

    public:

        VBSPGeometry(VBSPData * bspData);
        virtual ~VBSPGeometry();

        void                       addFace(int faceIndex);
        osg::ref_ptr<osg::Group>   createGeometry();
};


}


#endif

