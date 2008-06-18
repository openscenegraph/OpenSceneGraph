// -*-c++-*-

#include "ReaderWriterVRML2.h"


#if defined(_MSC_VER)
#   pragma warning(disable: 4250) 
#   pragma warning(disable: 4290) 
#   pragma warning(disable: 4800) 
#endif 

#include <openvrml/vrml97node.h>
#include <openvrml/common.h>
#include <openvrml/node.h>
#include <openvrml/node_ptr.h>
#include <openvrml/field.h>

#include <osg/CullFace>


osg::ref_ptr<osg::Geometry> ReaderWriterVRML2::convertVRML97IndexedFaceSet(openvrml::vrml97_node::indexed_face_set_node *vrml_ifs) const
{
    osg::ref_ptr<osg::Geometry> osg_geom = new osg::Geometry();
       
    osg_geom->addPrimitiveSet(new osg::DrawArrayLengths(osg::PrimitiveSet::POLYGON));
    osg::StateSet *osg_stateset = osg_geom->getOrCreateStateSet();
        
    // get array of vertex coordinate_nodes
    {
        const openvrml::field_value& fv = vrml_ifs->field("coord");
        const openvrml::sfnode& sfn = dynamic_cast<const openvrml::sfnode&>(fv);
        openvrml::vrml97_node::coordinate_node* vrml_coord_node = dynamic_cast<openvrml::vrml97_node::coordinate_node*>(sfn.value.get());
            
        const std::vector<openvrml::vec3f>& vrml_coord = vrml_coord_node->point();
        osg::ref_ptr<osg::Vec3Array> osg_vertices = new osg::Vec3Array();
            
        unsigned i;
        for (i = 0; i < vrml_coord.size(); i++)
        {
            openvrml::vec3f vec = vrml_coord[i];
            osg_vertices->push_back(osg::Vec3(vec[0], vec[1], vec[2]));
        }
            
        osg_geom->setVertexArray(osg_vertices.get());
            
        // get array of vertex indices
        const openvrml::field_value &fv2 = vrml_ifs->field("coordIndex");
        const openvrml::mfint32 &vrml_coord_index = dynamic_cast<const openvrml::mfint32 &>(fv2);
            
        osg::ref_ptr<osg::IntArray> osg_vert_index = new osg::IntArray();
            
        int num_vert = 0;
        for (i = 0; i < vrml_coord_index.value.size(); i++)
        {
            int index = vrml_coord_index.value[i];
            if (index == -1) 
            {
                static_cast<osg::DrawArrayLengths*>(osg_geom->getPrimitiveSet(0))->push_back(num_vert);
                num_vert = 0;
            } 
            else 
            {
                osg_vert_index->push_back(index);
                ++num_vert;
            }
        }
        if (num_vert)
        {
            //GvdB: Last coordIndex wasn't -1
            static_cast<osg::DrawArrayLengths*>(osg_geom->getPrimitiveSet(0))->push_back(num_vert);
        }
            
        osg_geom->setVertexIndices(osg_vert_index.get());
    }
        
    {
        // get texture coordinate_nodes
        const openvrml::field_value &fv = vrml_ifs->field("texCoord");
        const openvrml::sfnode &sfn = dynamic_cast<const openvrml::sfnode &>(fv);
        openvrml::vrml97_node::texture_coordinate_node *vrml_tex_coord_node =
            dynamic_cast<openvrml::vrml97_node::texture_coordinate_node *>(sfn.value.get());
            
        if (vrml_tex_coord_node != 0) // if no texture, node is NULL pointer
        {
            const std::vector<openvrml::vec2f> &vrml_tex_coord = vrml_tex_coord_node->point();
            osg::ref_ptr<osg::Vec2Array> osg_texcoords = new osg::Vec2Array();
                
            unsigned i;
            for (i = 0; i < vrml_tex_coord.size(); i++)
            {
                openvrml::vec2f vec = vrml_tex_coord[i];
                osg_texcoords->push_back(osg::Vec2(vec[0], vec[1]));
            }
                
            osg_geom->setTexCoordArray(0, osg_texcoords.get());
                
            // get array of texture indices
            const openvrml::field_value &fv2 = vrml_ifs->field("texCoordIndex");
            const openvrml::mfint32 &vrml_tex_coord_index = dynamic_cast<const openvrml::mfint32 &>(fv2);
                
            osg::ref_ptr<osg::IntArray> osg_tex_coord_index = new osg::IntArray();
                
            if(vrml_tex_coord_index.value.size() > 0)
            {
                for (i = 0; i < vrml_tex_coord_index.value.size(); i++)
                {
                    int index = vrml_tex_coord_index.value[i];
                    if (index != -1) {
                        osg_tex_coord_index->push_back(index);
                    }
                }
                osg_geom->setTexCoordIndices(0, osg_tex_coord_index.get());
            } else 
                // no indices defined, use coordIndex
                osg_geom->setTexCoordIndices(0, osg_geom->getVertexIndices());
        }
    }
        
    // get array of normals per vertex (if specified)
    {
        const openvrml::field_value& fv = vrml_ifs->field("normal");
        const openvrml::sfnode& sfn = dynamic_cast<const openvrml::sfnode&>(fv);
        openvrml::vrml97_node::normal_node* vrml_normal_node = dynamic_cast<openvrml::vrml97_node::normal_node*>(sfn.value.get());
            
        if (vrml_normal_node != 0) // if no normals, node is NULL pointer
        {
            const std::vector<openvrml::vec3f>& vrml_normal_coord = vrml_normal_node->vector();
                
            osg::ref_ptr<osg::Vec3Array> osg_normalcoords = new osg::Vec3Array();
                
            unsigned i;
            for (i = 0; i < vrml_normal_coord.size(); i++)
            {
                const openvrml::vec3f vec = vrml_normal_coord[i];
                osg_normalcoords->push_back(osg::Vec3(vec[0], vec[1], vec[2]));
            }
            osg_geom->setNormalArray(osg_normalcoords.get());
                
            // get array of normal indices
            const openvrml::field_value& fv2 = vrml_ifs->field("normalIndex");
            const openvrml::mfint32& vrml_normal_index = dynamic_cast<const openvrml::mfint32&>(fv2);
                
            osg::ref_ptr<osg::IntArray> osg_normal_index = new osg::IntArray();
                
            if (vrml_normal_index.value.size() > 0)
            {
                for (i = 0; i < vrml_normal_index.value.size(); i++)
                {
                    int index = vrml_normal_index.value[i];
                    if (index != -1)
                    {
                        osg_normal_index->push_back(index);
                    }
                }
                osg_geom->setNormalIndices(osg_normal_index.get());
            } 
            else
                // unspecified, use the coordIndex field
                osg_geom->setNormalIndices(osg_geom->getVertexIndices());
                
            // get normal binding
            const openvrml::field_value &fv3 = vrml_ifs->field("normalPerVertex");
            const openvrml::sfbool &vrml_norm_per_vertex = dynamic_cast<const openvrml::sfbool &>(fv3);
                
            if (vrml_norm_per_vertex.value)
            {
                osg_geom->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
            } else
            {
                osg_geom->setNormalBinding(osg::Geometry::BIND_PER_PRIMITIVE);
            }
        }
    }
        
    // get array of colours per vertex (if specified)
    {
        const openvrml::field_value &fv = vrml_ifs->field("color");
        const openvrml::sfnode &sfn = dynamic_cast<const openvrml::sfnode &>(fv);
        openvrml::vrml97_node::color_node *vrml_color_node =
            dynamic_cast<openvrml::vrml97_node::color_node *>(sfn.value.get());
            
        if (vrml_color_node != 0) // if no colors, node is NULL pointer
        {
            const std::vector<openvrml::color> &vrml_colors = vrml_color_node->color();
                
            osg::ref_ptr<osg::Vec3Array> osg_colors = new osg::Vec3Array();
                
            unsigned i;
            for (i = 0; i < vrml_colors.size(); i++)
            {
                const openvrml::color color = vrml_colors[i];
                osg_colors->push_back(osg::Vec3(color.r(), color.g(), color.b()));
            }
            osg_geom->setColorArray(osg_colors.get());
                
            // get array of color indices
            const openvrml::field_value &fv2 = vrml_ifs->field("colorIndex");
            const openvrml::mfint32 &vrml_color_index = dynamic_cast<const openvrml::mfint32 &>(fv2);
                
            osg::ref_ptr<osg::IntArray> osg_color_index = new osg::IntArray();
                
            if(vrml_color_index.value.size() > 0)
            {
                for (i = 0; i < vrml_color_index.value.size(); i++)
                {
                    int index = vrml_color_index.value[i];
                    if (index != -1) {
                        osg_color_index->push_back(index);
                    }
                }
                osg_geom->setColorIndices(osg_color_index.get());
            } else 
                // unspecified, use coordIndices field
                osg_geom->setColorIndices(osg_geom->getVertexIndices());
                
            // get color binding
            const openvrml::field_value &fv3 = vrml_ifs->field("colorPerVertex");
            const openvrml::sfbool &vrml_color_per_vertex = dynamic_cast<const openvrml::sfbool &>(fv3);
                
            if (vrml_color_per_vertex.value)
            {
                osg_geom->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
            } else
            {
                osg_geom->setColorBinding(osg::Geometry::BIND_PER_PRIMITIVE);
            }
        }
    }
        
    if (static_cast<const openvrml::sfbool&>(vrml_ifs->field("solid")).value)
    {
        osg_geom->getOrCreateStateSet()->setAttributeAndModes(new osg::CullFace(osg::CullFace::BACK));
    }
        
    if (!osg_geom->getNormalArray())
    {
#if 0
        // GvdB: This is what I wanted to do, but got zero normals since the triangles were considered temporaries (?)
        osgUtil::SmoothingVisitor().smooth(*osg_geom);
#else
        // GvdB: So I ended up computing the smoothing normals myself. Also, I might add support for "creaseAngle" if a big need for it rises.
        //       However, for now I can perfectly live with the fact that all edges are smoothed despite the use of a crease angle.     
        osg::Vec3Array& coords = *static_cast<osg::Vec3Array*>(osg_geom->getVertexArray());
        assert(coords.size());
            
        osg::Vec3Array* normals = new osg::Vec3Array(coords.size());
            
        for (osg::Vec3Array::iterator it = normals->begin(); it != normals->end(); ++it)
        {
            (*it).set(0.0f, 0.0f, 0.0f);
        }  
            
            
        osg::IntArray& indices = *static_cast<osg::IntArray*>(osg_geom->getVertexIndices());
        osg::DrawArrayLengths& lengths = *static_cast<osg::DrawArrayLengths*>(osg_geom->getPrimitiveSet(0));
        int index = 0;
            
        for (osg::DrawArrayLengths::iterator it = lengths.begin(); it != lengths.end(); ++it) 
        {
            assert(*it >= 3);
            const osg::Vec3& v0 = coords[indices[index]];  
            const osg::Vec3& v1 = coords[indices[index + 1]];  
            const osg::Vec3& v2 = coords[indices[index + 2]];  
                
            osg::Vec3 normal = (v1 - v0) ^ (v2 - v0);
            normal.normalize();
                
            for (int i = 0; i != *it; ++i)
            {
                (*normals)[indices[index + i]] += normal;
            }
                
            index += *it;
        }
            
        assert(index == indices.size());
            
        for(osg::Vec3Array::iterator it = normals->begin(); it != normals->end(); ++it)
        {
            (*it).normalize();
        }
            
        osg_geom->setNormalArray(normals);
        osg_geom->setNormalIndices(osg_geom->getVertexIndices());
        osg_geom->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);                   
            
#endif
    } 
        
        
    osg::DrawArrayLengths& lengths = *static_cast<osg::DrawArrayLengths*>(osg_geom->getPrimitiveSet(0));
                    
    osg::DrawArrayLengths::iterator it = lengths.begin(); 
    if (it != lengths.end())
    {
        switch (*it)
        {
        case 3:
            while (++it != lengths.end() && *it == 3)
                ;
                
            if (it == lengths.end())
            {
                // All polys are triangles
                osg::ref_ptr<osg::DrawArrays> mesh = new osg::DrawArrays(osg::PrimitiveSet::TRIANGLES);
                mesh->setCount(lengths.size() * 3);
                osg_geom->removePrimitiveSet(0);
                osg_geom->addPrimitiveSet(mesh.get());
            }      
            break;
                
        case 4:
            while (++it != lengths.end() && *it == 4)
                ;
                
            if (it == lengths.end())
            {
                // All polys are quads
                osg::ref_ptr<osg::DrawArrays> mesh = new osg::DrawArrays(osg::PrimitiveSet::QUADS);
                mesh->setCount(lengths.size() * 4);
                osg_geom->removePrimitiveSet(0);
                osg_geom->addPrimitiveSet(mesh.get());
            }  
                
            break;
        }
    } 
    return osg_geom.get();
}
