// -*-c++-*-

#include "ReaderWriterVRML2.h"

#include <complex>

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

osg::ref_ptr<osg::Geometry> ReaderWriterVRML2::convertVRML97Box(openvrml::vrml97_node::box_node* vrml_box) const
{
    const openvrml::vec3f& size = static_cast<const openvrml::sfvec3f&>(vrml_box->field("size")).value;
                    
    osg::Vec3 halfSize(size[0] * 0.5f, size[1] * 0.5f, size[2] * 0.5f);  

    BoxLibrary::const_iterator it = m_boxLibrary.find(halfSize);
    if (it != m_boxLibrary.end())
    {
        return (*it).second.get();
    } 
  
    osg::ref_ptr<osg::Geometry> osg_geom = new osg::Geometry();
    osg::ref_ptr<osg::Vec3Array> osg_vertices = new osg::Vec3Array();
    osg::ref_ptr<osg::Vec2Array> osg_texcoords = new osg::Vec2Array(); 
    osg::ref_ptr<osg::Vec3Array> osg_normals = new osg::Vec3Array();  
                    
    osg::ref_ptr<osg::DrawArrays> box = new osg::DrawArrays(osg::PrimitiveSet::QUADS);
        
    osg_vertices->push_back(osg::Vec3(-halfSize[0], halfSize[1], halfSize[2]));           
    osg_vertices->push_back(osg::Vec3(-halfSize[0], -halfSize[1], halfSize[2]));           
    osg_vertices->push_back(osg::Vec3(halfSize[0], -halfSize[1], halfSize[2])); 
    osg_vertices->push_back(osg::Vec3(halfSize[0], halfSize[1], halfSize[2])); 

    osg_vertices->push_back(osg::Vec3(halfSize[0], halfSize[1], -halfSize[2])); 
    osg_vertices->push_back(osg::Vec3(halfSize[0], -halfSize[1], -halfSize[2]));           
    osg_vertices->push_back(osg::Vec3(-halfSize[0], -halfSize[1], -halfSize[2]));           
    osg_vertices->push_back(osg::Vec3(-halfSize[0], halfSize[1], -halfSize[2]));  

    osg_vertices->push_back(osg::Vec3(halfSize[0], halfSize[1], halfSize[2])); 
    osg_vertices->push_back(osg::Vec3(halfSize[0], -halfSize[1], halfSize[2]));           
    osg_vertices->push_back(osg::Vec3(halfSize[0], -halfSize[1], -halfSize[2]));           
    osg_vertices->push_back(osg::Vec3(halfSize[0], halfSize[1], -halfSize[2]));

    osg_vertices->push_back(osg::Vec3(-halfSize[0], halfSize[1], -halfSize[2])); 
    osg_vertices->push_back(osg::Vec3(-halfSize[0], -halfSize[1], -halfSize[2]));           
    osg_vertices->push_back(osg::Vec3(-halfSize[0], -halfSize[1], halfSize[2]));           
    osg_vertices->push_back(osg::Vec3(-halfSize[0], halfSize[1], halfSize[2])); 

    osg_vertices->push_back(osg::Vec3(-halfSize[0], halfSize[1], -halfSize[2])); 
    osg_vertices->push_back(osg::Vec3(-halfSize[0], halfSize[1], halfSize[2]));           
    osg_vertices->push_back(osg::Vec3(halfSize[0], halfSize[1], halfSize[2]));           
    osg_vertices->push_back(osg::Vec3(halfSize[0], halfSize[1], -halfSize[2]));      
                
    osg_vertices->push_back(osg::Vec3(-halfSize[0], -halfSize[1], halfSize[2])); 
    osg_vertices->push_back(osg::Vec3(-halfSize[0], -halfSize[1], -halfSize[2]));           
    osg_vertices->push_back(osg::Vec3(halfSize[0], -halfSize[1], -halfSize[2]));           
    osg_vertices->push_back(osg::Vec3(halfSize[0], -halfSize[1], halfSize[2])); 

    for (int i = 0; i != 6; ++i)
    {
        osg_texcoords->push_back(osg::Vec2(0.0f, 1.0f));
        osg_texcoords->push_back(osg::Vec2(0.0f, 0.0f));
        osg_texcoords->push_back(osg::Vec2(1.0f, 0.0f));
        osg_texcoords->push_back(osg::Vec2(1.0f, 1.0f));
    }

    osg_normals->push_back(osg::Vec3(0.0f, 0.0f, 1.0f));
    osg_normals->push_back(osg::Vec3(0.0f, 0.0f, -1.0f));
    osg_normals->push_back(osg::Vec3(1.0f, 0.0f, 0.0f));
    osg_normals->push_back(osg::Vec3(-1.0f, 0.0f, 0.0f));
    osg_normals->push_back(osg::Vec3(0.0f, 1.0f, 0.0f));
    osg_normals->push_back(osg::Vec3(0.0f, -1.0f, 0.0f));

    box->setCount(osg_vertices->size());
                    
    osg_geom->addPrimitiveSet(box.get());
                
    osg_geom->setVertexArray(osg_vertices.get());
    osg_geom->setTexCoordArray(0, osg_texcoords.get());
    osg_geom->setNormalArray(osg_normals.get());
    osg_geom->setNormalBinding(osg::Geometry::BIND_PER_PRIMITIVE);

    osg_geom->getOrCreateStateSet()->setAttributeAndModes(new osg::CullFace(osg::CullFace::BACK));

    m_boxLibrary[halfSize] = osg_geom;

    return osg_geom.get();
}


osg::ref_ptr<osg::Geometry> ReaderWriterVRML2::convertVRML97Sphere(openvrml::vrml97_node::sphere_node* vrml_sphere) const
{
    float radius = static_cast<const openvrml::sffloat&>(vrml_sphere->field("radius")).value;

    SphereLibrary::const_iterator it = m_sphereLibrary.find(radius);
    if (it != m_sphereLibrary.end())
    {
        return (*it).second.get();
    }

    osg::ref_ptr<osg::Geometry> osg_geom = new osg::Geometry();
    osg::ref_ptr<osg::Vec3Array> osg_vertices = new osg::Vec3Array();
    osg::ref_ptr<osg::Vec2Array> osg_texcoords = new osg::Vec2Array(); 
    osg::ref_ptr<osg::Vec3Array> osg_normals = new osg::Vec3Array();
    
    unsigned int numSegments = 40;
    unsigned int numRows = 20;

    const float thetaDelta = 2.0f * float(osg::PI) / float(numSegments);
    const float texCoordSDelta = 1.0f / float(numSegments);
    const float phiDelta = float(osg::PI) / float(numRows);
    const float texCoordTDelta = 1.0f / float(numRows);

    float phi = -0.5f * float(osg::PI);
    float texCoordT = 0.0f;
                    
    osg::ref_ptr<osg::DrawArrayLengths> sphere = new osg::DrawArrayLengths(osg::PrimitiveSet::QUAD_STRIP);
                    
    for (unsigned int i = 0; i < numRows; ++i, phi += phiDelta, texCoordT += texCoordTDelta)
    {
        std::complex<float> latBottom = std::polar(1.0f, phi);
        std::complex<float> latTop = std::polar(1.0f, phi + phiDelta);
        std::complex<float> eBottom = latBottom * radius;
        std::complex<float> eTop = latTop * radius;

        float theta = 0.0f;
        float texCoordS = 0.0f;
                     
        for (unsigned int j = 0; j < numSegments; ++j, theta += thetaDelta, texCoordS += texCoordSDelta)
        {
            std::complex<float> n = -std::polar(1.0f, theta);

            osg_normals->push_back(osg::Vec3(latTop.real() * n.imag(), latTop.imag(), latTop.real() * n.real()));
            osg_normals->push_back(osg::Vec3(latBottom.real() * n.imag(), latBottom.imag(), latBottom.real() * n.real()));
                          
            osg_texcoords->push_back(osg::Vec2(texCoordS, texCoordT + texCoordTDelta));
            osg_texcoords->push_back(osg::Vec2(texCoordS, texCoordT));
                  
            osg_vertices->push_back(osg::Vec3(eTop.real() * n.imag(), eTop.imag(), eTop.real() * n.real()));
            osg_vertices->push_back(osg::Vec3(eBottom.real() * n.imag(), eBottom.imag(), eBottom.real() * n.real()));
        }

        osg_normals->push_back(osg::Vec3(0.0f, latTop.imag(), -latTop.real()));
        osg_normals->push_back(osg::Vec3(0.0f, latBottom.imag(), -latBottom.real()));
              
        osg_texcoords->push_back(osg::Vec2(1.0f, texCoordT + texCoordTDelta));
        osg_texcoords->push_back(osg::Vec2(1.0f, texCoordT));
              
        osg_vertices->push_back(osg::Vec3(0.0f, eTop.imag(), -eTop.real()));
        osg_vertices->push_back(osg::Vec3(0.0f, eBottom.imag(), -eBottom.real()));
                         
        sphere->push_back(numSegments * 2 + 2);
    }
                    
    osg_geom->addPrimitiveSet(sphere.get());
                
    osg_geom->setVertexArray(osg_vertices.get());
    osg_geom->setTexCoordArray(0, osg_texcoords.get());
    osg_geom->setNormalArray(osg_normals.get());
    osg_geom->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);

    osg_geom->getOrCreateStateSet()->setAttributeAndModes(new osg::CullFace(osg::CullFace::BACK));

    m_sphereLibrary[radius] = osg_geom;

    return osg_geom.get();
}


osg::ref_ptr<osg::Geometry> ReaderWriterVRML2::convertVRML97Cone(openvrml::vrml97_node::cone_node* vrml_cone) const
{
    float height = static_cast<const openvrml::sffloat&>(vrml_cone->field("height")).value;
    float radius = static_cast<const openvrml::sffloat&>(vrml_cone->field("bottomRadius")).value;
    bool bottom = static_cast<const openvrml::sfbool&>(vrml_cone->field("bottom")).value;
    bool side = static_cast<const openvrml::sfbool&>(vrml_cone->field("side")).value; 

    QuadricKey key(height, radius, bottom, side, false);

    ConeLibrary::const_iterator it = m_coneLibrary.find(key);
    if (it != m_coneLibrary.end())
    {
        return (*it).second.get();
    }
                 
    osg::ref_ptr<osg::Geometry> osg_geom = new osg::Geometry();
    osg::ref_ptr<osg::Vec3Array> osg_vertices = new osg::Vec3Array();
    osg::ref_ptr<osg::Vec2Array> osg_texcoords = new osg::Vec2Array(); 
    osg::ref_ptr<osg::Vec3Array> osg_normals = new osg::Vec3Array();

    unsigned int numSegments = 40;

    const float thetaDelta = 2.0f * float(osg::PI) / float(numSegments);
                                 
    float topY = height * 0.5f;
    float bottomY = height * -0.5f;
                   
    if (side)
    {
        osg::ref_ptr<osg::DrawArrays> side = new osg::DrawArrays(osg::PrimitiveSet::QUAD_STRIP);
                            
        const float texCoordDelta = 1.0f / float(numSegments);
        float theta = 0.0f;
        float texCoord = 0.0f;
                        
        for (unsigned int i = 0; i < numSegments; ++i, theta += thetaDelta, texCoord += texCoordDelta)
        {
            std::complex<float> n = -std::polar(1.0f, theta);
            std::complex<float> e = n * radius;  
                         
            osg::Vec3 normal(n.imag() * height, radius, n.real() * height);
            normal.normalize();

            osg_normals->push_back(normal);
            osg_normals->push_back(normal);
                  
            osg_texcoords->push_back(osg::Vec2(texCoord, 1.0f));
            osg_texcoords->push_back(osg::Vec2(texCoord, 0.0f));
                  
            osg_vertices->push_back(osg::Vec3(0.0f, topY, 0.0f));
            osg_vertices->push_back(osg::Vec3(e.imag(), bottomY, e.real()));
        }
                      
        // do last point by hand to ensure no round off errors.
                        
        osg::Vec3 normal(0.0f, radius, -height);
        normal.normalize();

        osg_normals->push_back(normal);    
        osg_normals->push_back(normal);
                  
        osg_texcoords->push_back(osg::Vec2(1.0f, 1.0f));
        osg_texcoords->push_back(osg::Vec2(1.0f, 0.0f));
                         
        osg_vertices->push_back(osg::Vec3(0.0f, topY, 0.0f));
        osg_vertices->push_back(osg::Vec3(0.0f, bottomY, -radius)); 
                        
        side->setCount(osg_vertices->size());
        osg_geom->addPrimitiveSet(side.get()); 
    }

    if (bottom)
    {
        osg::ref_ptr<osg::DrawArrays> bottom = new osg::DrawArrays(osg::PrimitiveSet::TRIANGLE_FAN);
                            

        size_t first = osg_vertices->size();
        bottom->setFirst(first);

        float theta = 0.0f;
                                               
        for (unsigned int i = 0; i < numSegments; ++i, theta += thetaDelta)
        {
            std::complex<float> n = -std::polar(1.0f, theta);
            std::complex<float> e = n * radius;  
                                                    
            osg_normals->push_back(osg::Vec3(0.0f, -1.0f, 0.0f));
            osg_texcoords->push_back(osg::Vec2(0.5f - 0.5f * n.imag(), 0.5f + 0.5f * n.real()));
            osg_vertices->push_back(osg::Vec3(-e.imag(), bottomY, e.real()));
        }
                      
        // do last point by hand to ensure no round off errors.
                        
        osg_normals->push_back(osg::Vec3(0.0f, -1.0f, 0.0f));
        osg_texcoords->push_back(osg::Vec2(0.5f, 0.0f));
        osg_vertices->push_back(osg::Vec3(0.0f, bottomY, -radius));
                        
        bottom->setCount(osg_vertices->size() - first);
        osg_geom->addPrimitiveSet(bottom.get());
    }
 
    osg_geom->setVertexArray(osg_vertices.get());
    osg_geom->setTexCoordArray(0, osg_texcoords.get());
    osg_geom->setNormalArray(osg_normals.get());
    osg_geom->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);

    osg_geom->getOrCreateStateSet()->setAttributeAndModes(new osg::CullFace(osg::CullFace::BACK));

    m_coneLibrary[key] = osg_geom;

    return osg_geom.get();
}

osg::ref_ptr<osg::Geometry> ReaderWriterVRML2::convertVRML97Cylinder(openvrml::vrml97_node::cylinder_node* vrml_cylinder) const
{     
    float height = static_cast<const openvrml::sffloat&>(vrml_cylinder->field("height")).value;
    float radius = static_cast<const openvrml::sffloat&>(vrml_cylinder->field("radius")).value;
    bool bottom = static_cast<const openvrml::sfbool&>(vrml_cylinder->field("bottom")).value;
    bool side = static_cast<const openvrml::sfbool&>(vrml_cylinder->field("side")).value;
    bool top = static_cast<const openvrml::sfbool&>(vrml_cylinder->field("top")).value;

    QuadricKey key(height, radius, bottom, side, top);

    CylinderLibrary::const_iterator it = m_cylinderLibrary.find(key);
    if (it != m_cylinderLibrary.end())
    {
        return (*it).second.get();
    }

    osg::ref_ptr<osg::Geometry> osg_geom = new osg::Geometry();          
    osg::ref_ptr<osg::Vec3Array> osg_vertices = new osg::Vec3Array();
    osg::ref_ptr<osg::Vec2Array> osg_texcoords = new osg::Vec2Array(); 
    osg::ref_ptr<osg::Vec3Array> osg_normals = new osg::Vec3Array();

    unsigned int numSegments = 40;

    const float thetaDelta = 2.0f * float(osg::PI) / float(numSegments);
                    
                  
    float topY = height * 0.5f;
    float bottomY = height * -0.5f;                  

    if (side)
    {
        osg::ref_ptr<osg::DrawArrays> side = new osg::DrawArrays(osg::PrimitiveSet::QUAD_STRIP);
                        
        const float texCoordDelta = 1.0f / float(numSegments);
        float theta = 0.0f;
        float texCoord = 0.0f;
                        
        for (unsigned int i = 0; i < numSegments; ++i, theta += thetaDelta, texCoord += texCoordDelta)
        {
            std::complex<float> n = -std::polar(1.0f, theta);
            std::complex<float> e = n * radius;  
                         
            osg::Vec3 normal(n.imag(), 0.0f, n.real());
                            
            osg_normals->push_back(normal);
            osg_normals->push_back(normal);
                  
            osg_texcoords->push_back(osg::Vec2(texCoord, 1.0f));
            osg_texcoords->push_back(osg::Vec2(texCoord, 0.0f));
                  
            osg_vertices->push_back(osg::Vec3(e.imag(), topY, e.real()));
            osg_vertices->push_back(osg::Vec3(e.imag(), bottomY, e.real()));
        }
                      
        // do last point by hand to ensure no round off errors.

        osg::Vec3 normal(0.0f, 0.0f, -1.0f);
        osg_normals->push_back(normal);    
        osg_normals->push_back(normal);
                  
        osg_texcoords->push_back(osg::Vec2(1.0f, 1.0f));
        osg_texcoords->push_back(osg::Vec2(1.0f, 0.0f));
                         
        osg_vertices->push_back(osg::Vec3(0.0f, topY, -radius));
        osg_vertices->push_back(osg::Vec3(0.0f, bottomY, -radius));

        side->setCount(osg_vertices->size());
        osg_geom->addPrimitiveSet(side.get());
    }
                     
    if (bottom)
    {
        osg::ref_ptr<osg::DrawArrays> bottom = new osg::DrawArrays(osg::PrimitiveSet::TRIANGLE_FAN);
                            
        size_t first = osg_vertices->size();
        bottom->setFirst(first);

        float theta = 0.0f;
                                               
        for (unsigned int i = 0; i < numSegments; ++i, theta += thetaDelta)
        {
            std::complex<float> n = -std::polar(1.0f, theta);
            std::complex<float> e = n * radius;  
                                                    
            osg_normals->push_back(osg::Vec3(0.0f, -1.0f, 0.0f));
            osg_texcoords->push_back(osg::Vec2(0.5f - 0.5f * n.imag(), 0.5f + 0.5f * n.real()));
            osg_vertices->push_back(osg::Vec3(-e.imag(), bottomY, e.real()));
        }
                      
        // do last point by hand to ensure no round off errors.
                        
        osg_normals->push_back(osg::Vec3(0.0f, -1.0f, 0.0f));
        osg_texcoords->push_back(osg::Vec2(0.5f, 0.0f));
        osg_vertices->push_back(osg::Vec3(0.0f, bottomY, -radius));
                        
        bottom->setCount(osg_vertices->size() - first);
        osg_geom->addPrimitiveSet(bottom.get());
    }

    if (top)
    {
        osg::ref_ptr<osg::DrawArrays> top = new osg::DrawArrays(osg::PrimitiveSet::TRIANGLE_FAN);
                            
        size_t first = osg_vertices->size();
        top->setFirst(first);

        float theta = 0.0f;
                                               
        for (unsigned int i = 0; i < numSegments; ++i, theta += thetaDelta)
        {
            std::complex<float> n = -std::polar(1.0f, theta);
            std::complex<float> e = n * radius;  
                                                    
            osg_normals->push_back(osg::Vec3(0.0f, 1.0f, 0.0f));
            osg_texcoords->push_back(osg::Vec2(0.5f + 0.5f * n.imag(), 0.5f - 0.5f * n.real()));
            osg_vertices->push_back(osg::Vec3(e.imag(), topY, e.real()));
        }
                      
        // do last point by hand to ensure no round off errors.
                        
        osg_normals->push_back(osg::Vec3(0.0f, 1.0f, 0.0f));
        osg_texcoords->push_back(osg::Vec2(0.5f, 1.0f));
        osg_vertices->push_back(osg::Vec3(0.0f, topY, -radius));
                        
        top->setCount(osg_vertices->size() - first);
        osg_geom->addPrimitiveSet(top.get());
    }
       
    osg_geom->setVertexArray(osg_vertices.get());
    osg_geom->setTexCoordArray(0, osg_texcoords.get());
    osg_geom->setNormalArray(osg_normals.get());
    osg_geom->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);

    osg_geom->getOrCreateStateSet()->setAttributeAndModes(new osg::CullFace(osg::CullFace::BACK));

    m_cylinderLibrary[key] = osg_geom;

    return osg_geom.get();
}
