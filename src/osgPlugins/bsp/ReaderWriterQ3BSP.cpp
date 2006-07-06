
#include <osg/TexEnv>
#include <osg/CullFace>

#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Image>
#include <osg/Texture2D>
#include <osg/PolygonMode>
#include <osg/BlendColor>

#include <osgDB/ReadFile>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>

#include <assert.h>
#include <fstream>

#include "BSPLoad.h"


class ReaderWriterQ3BSP: public osgDB::ReaderWriter
{
public:
    ReaderWriterQ3BSP() { }

    virtual const char* className() const
      {
        return "Quake3 BSP Reader";
      }

    virtual bool acceptsExtension(const std::string& extension) const
      { 
        return osgDB::equalCaseInsensitive(extension,"bsp");
      }

    virtual ReadResult readNode(const std::string& fileName, const osgDB::ReaderWriter::Options* options) const;

private:
    osg::Geode* convertFromBSP(BSPLoad& aLoadData,const osgDB::ReaderWriter::Options* options) const;
    osg::Geometry* createMeshFace(const BSP_LOAD_FACE& aLoadFace,const std::vector<osg::Texture2D*>& aTextureArray,
                                  osg::Vec3Array& aVertexArray,std::vector<GLuint>& aIndices,
                                  osg::Vec2Array& aTextureDecalCoords,osg::Vec2Array& aTextureLMapCoords
                                 ) const;
    osg::Geometry* createPolygonFace(const BSP_LOAD_FACE& aLoadFace,const std::vector<osg::Texture2D*>& aTextureArray,const std::vector<osg::Texture2D*>& aTextureLMapArray,
                                  osg::Vec3Array& aVertexArray,
                                  osg::Vec2Array& aTextureDecalCoords,osg::Vec2Array& aTextureLMapCoords
                                 ) const;
    bool        loadTextures(const BSPLoad& aLoadData,std::vector<osg::Texture2D*>& aTextureArray) const;
    bool        loadLightMaps(const BSPLoad& aLoadData,std::vector<osg::Texture2D*>& aTextureArray) const;
};

// Register with Registry to instantiate the above reader/writer.
osgDB::RegisterReaderWriterProxy<ReaderWriterQ3BSP> g_readerWriter_Q3BSP_Proxy;











// Read node
osgDB::ReaderWriter::ReadResult ReaderWriterQ3BSP::readNode(const std::string& file, const osgDB::ReaderWriter::Options* options) const
{
    std::string ext = osgDB::getLowerCaseFileExtension(file);
    if (!acceptsExtension(ext)) 
      return ReadResult::FILE_NOT_HANDLED;

    std::string file_name = osgDB::findDataFile( file, options );
    if (file_name.empty())
      return ReadResult::FILE_NOT_FOUND;

    //osg::notify(osg::INFO) << "ReaderWriterQ3BSP::readNode(" << fileName.c_str() << ")\n";
    BSPLoad load_data;
    load_data.Load(file_name,8);

    osg::Geode* geode = convertFromBSP(load_data, options);
    if (!geode)
      return ReadResult::FILE_NOT_HANDLED;

    //osg::StateSet* state_set=geode->getOrCreateStateSet();
    //state_set->setMode(osg::CullFace::BACK,osg::StateAttribute::ON);
    return geode;


    return ReadResult::FILE_NOT_HANDLED;
}





enum BSP_FACE_TYPE
{
  bspPolygonFace=1,
  bspPatch,
  bspMeshFace,
  bspBillboard
};





class BSP_VERTEX
{
public:
    osg::Vec3f m_position;
    float m_decalS, m_decalT;
    float m_lightmapS, m_lightmapT;

    BSP_VERTEX operator+(const BSP_VERTEX & rhs) const
      {
        BSP_VERTEX result;
        result.m_position=m_position+rhs.m_position;
        result.m_decalS=m_decalS+rhs.m_decalS;
        result.m_decalT=m_decalT+rhs.m_decalT;
        result.m_lightmapS=m_lightmapS+rhs.m_lightmapS;
        result.m_lightmapT=m_lightmapT+rhs.m_lightmapT;

        return result;
      }

    BSP_VERTEX operator*(const float rhs) const
      {
        BSP_VERTEX result;
        result.m_position=m_position*rhs;
        result.m_decalS=m_decalS*rhs;
        result.m_decalT=m_decalT*rhs;
        result.m_lightmapS=m_lightmapS*rhs;
        result.m_lightmapT=m_lightmapT*rhs;

        return result;
      }
};





//every patch (curved surface) is split into biquadratic (3x3) patches
class BSP_BIQUADRATIC_PATCH
{
public:
    BSP_BIQUADRATIC_PATCH():m_vertices(32),m_indices(32)
    {
    }
    ~BSP_BIQUADRATIC_PATCH()
    {
    }

    bool Tesselate(int newTesselation,osg::Geometry* aGeometry);

    BSP_VERTEX m_controlPoints[9];  // Se accede a ellos en la carga

protected:
        
    int m_tesselation;
    std::vector<BSP_VERTEX> m_vertices;
    std::vector<GLuint> m_indices;

    //arrays for multi_draw_arrays
    std::vector<int>  m_trianglesPerRow;
    std::vector<GLuint *>  m_rowIndexPointers;

};


//curved surface
class BSP_PATCH
{
public:

    BSP_PATCH():m_quadraticPatches(32)
    {
    }
    ~BSP_PATCH()
    {
    }

    int m_textureIndex;
    int m_lightmapIndex;
    int m_width, m_height;

    int m_numQuadraticPatches;
    std::vector<BSP_BIQUADRATIC_PATCH>  m_quadraticPatches;
};









osg::Geode* ReaderWriterQ3BSP::convertFromBSP(BSPLoad& aLoadData,const osgDB::ReaderWriter::Options*) const
{

  std::vector<osg::Texture2D*> texture_array;
  loadTextures(aLoadData,texture_array);

  std::vector<osg::Texture2D*> lightmap_array;
  loadLightMaps(aLoadData,lightmap_array);

  osg::Geode* map_geode=new osg::Geode;

  // Convertir los vertices
  unsigned int num_load_vertices=aLoadData.m_loadVertices.size();
  osg::Vec3Array* vertex_array = new osg::Vec3Array(num_load_vertices);
  osg::Vec2Array* text_decal_array = new osg::Vec2Array(num_load_vertices);
  osg::Vec2Array* text_lmap_array = new osg::Vec2Array(num_load_vertices);

  float scale = 0.03;
  unsigned int i;
  for(i=0; i<num_load_vertices; ++i)
    {
      BSP_LOAD_VERTEX& vtx=aLoadData.m_loadVertices[i];
      //swap y and z and negate z
      (*vertex_array)[i]=(osg::Vec3d(  vtx.m_position[0]*scale,
                                       -vtx.m_position[1]*scale,
                                       vtx.m_position[2]*scale ) );

      //Transfer texture coordinates (Invert t)
      (*text_decal_array)[i]=(osg::Vec2d(vtx.m_decalS,-vtx.m_decalT) );

      //Transfer lightmap coordinates
      (*text_lmap_array)[i]=(osg::Vec2d(vtx.m_lightmapS,vtx.m_lightmapT) );
    }



  unsigned int num_loadfaces=aLoadData.m_loadFaces.size();

  //convert loadFaces to faces
  for(i=0; i<num_loadfaces; ++i)
    {
      const BSP_LOAD_FACE& current_load_face=aLoadData.m_loadFaces[i];
      if(current_load_face.m_type!=bspMeshFace)      //skip this loadFace if it is not a mesh face
          continue;

      osg::Geometry* mesh_geom=createMeshFace(current_load_face,texture_array,*vertex_array,aLoadData.m_loadMeshIndices,*text_decal_array,*text_lmap_array);
      map_geode->addDrawable(mesh_geom);
    }


  for(i=0; i<num_loadfaces; ++i)
    {
      const BSP_LOAD_FACE& current_face=aLoadData.m_loadFaces[i];
      if(current_face.m_type!=bspPolygonFace)      //skip this loadFace if it is not a polygon face
          continue;

      osg::Geometry* polygon_geom=createPolygonFace(current_face,texture_array,lightmap_array,*vertex_array,*text_decal_array,*text_lmap_array);
      map_geode->addDrawable(polygon_geom);
    }



  for(i=0; i<num_loadfaces; ++i)
    {
      const BSP_LOAD_FACE& current_face=aLoadData.m_loadFaces[i];
      if(current_face.m_type!=bspPatch)           //skip this loadFace if it is not a patch face
          continue;


      //osg::Group* patch_group=new osg::Group;

      BSP_PATCH current_patch;

      current_patch.m_textureIndex=current_face.m_texture;
      current_patch.m_lightmapIndex=current_face.m_lightmapIndex;
      current_patch.m_width=current_face.m_patchSize[0];
      current_patch.m_height=current_face.m_patchSize[1];


      osg::Texture2D *texture=texture_array[current_face.m_texture];
      osg::Texture2D *lightmap_texture=NULL;
      if(current_face.m_lightmapIndex>=0)
        {
          lightmap_texture=lightmap_array[current_face.m_lightmapIndex];
        }
      else
        {
          lightmap_texture=lightmap_array[lightmap_array.size()-1];
        }

      //Create space to hold quadratic patches
      int numPatchesWide=(current_patch.m_width-1)/2;
      int numPatchesHigh=(current_patch.m_height-1)/2;

      current_patch.m_numQuadraticPatches =  numPatchesWide*numPatchesHigh;
      current_patch.m_quadraticPatches.resize(current_patch.m_numQuadraticPatches);

      //fill in the quadratic patches
      for(int y=0; y<numPatchesHigh; ++y)
        {
          for(int x=0; x<numPatchesWide; ++x)
            {
              for(int row=0; row<3; ++row)
                {
                  for(int point=0; point<3; ++point)
                    {
                      BSP_BIQUADRATIC_PATCH& curr_quadraticpatch=current_patch.m_quadraticPatches[y*numPatchesWide+x];

                      osg::Vec3f vtx= (*vertex_array) [aLoadData.m_loadFaces[i].m_firstVertexIndex+(y*2*current_patch.m_width+x*2)+
                                                                                              row*current_patch.m_width+point];

                      curr_quadraticpatch.m_controlPoints[row*3+point].m_position[0] = vtx.x();
                      curr_quadraticpatch.m_controlPoints[row*3+point].m_position[1] = vtx.y();
                      curr_quadraticpatch.m_controlPoints[row*3+point].m_position[2] = vtx.z();

                    }
                }

              
  
              osg::Geometry* patch_geom = new osg::Geometry;
              //tesselate the patch

              osg::StateSet* stateset = patch_geom->getOrCreateStateSet();
              if(texture)
              {
                stateset->setTextureAttributeAndModes(0,texture,osg::StateAttribute::ON);
              }
              
              if(lightmap_texture)
              {
                  stateset->setTextureAttributeAndModes(1,lightmap_texture,osg::StateAttribute::ON);
              }
                
              //patch_group->addChild(map_geode);

              current_patch.m_quadraticPatches[y*numPatchesWide+x].Tesselate(8/*aCurveTesselation*/,patch_geom);
              map_geode->addDrawable(patch_geom);
            }
        }
      

    }


  //int num_primitive_sets=geom->getNumPrimitiveSets();
  //const osg::BoundingSphere& bs=map_geom->getBound();


  map_geode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);


  return map_geode;
}



osg::Geometry* ReaderWriterQ3BSP::createMeshFace( const BSP_LOAD_FACE& aLoadFace,const std::vector<osg::Texture2D*>& aTextureArray,
                                                  osg::Vec3Array& aVertexArray,std::vector<GLuint>& aIndices,
                                                  osg::Vec2Array& aTextureDecalCoords,osg::Vec2Array& aTextureLMapCoords
                                                ) const
{

  osg::Geometry* obj_geom = new osg::Geometry;


  osg::Vec3Array* obj_vertex_array = new osg::Vec3Array(aLoadFace.m_numMeshIndices,
                                                        &(aVertexArray)[aLoadFace.m_firstVertexIndex]
                                                        );
  obj_geom->setVertexArray(obj_vertex_array);

  osg::DrawElementsUInt* face_indices = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES,
                                                  aLoadFace.m_numMeshIndices,
                                                  &(aIndices)[0]+aLoadFace.m_firstMeshIndex
                                                  );

  obj_geom->addPrimitiveSet(face_indices);

  osg::Texture2D *texture=aTextureArray[aLoadFace.m_texture];
  if(texture)
    {
      osg::StateSet* stateset = obj_geom->getOrCreateStateSet();
      stateset->setTextureAttributeAndModes(0,texture,osg::StateAttribute::ON);
      stateset->setTextureAttributeAndModes(1,texture,osg::StateAttribute::ON);

      osg::Vec2Array* obj_texcoords_array = new osg::Vec2Array(aLoadFace.m_numMeshIndices,
                                                        &(aTextureDecalCoords)[aLoadFace.m_firstVertexIndex]
                                                        );
      obj_geom->setTexCoordArray(0,obj_texcoords_array);

      osg::Vec2Array* obj_lmapcoords_array = new osg::Vec2Array(aLoadFace.m_numMeshIndices,
                                                        &(aTextureLMapCoords)[aLoadFace.m_firstVertexIndex]
                                                        );
      obj_geom->setTexCoordArray(1,obj_lmapcoords_array);
    }

  return obj_geom;
     

}







osg::Geometry* ReaderWriterQ3BSP::createPolygonFace(const BSP_LOAD_FACE& aLoadFace,const std::vector<osg::Texture2D*>& aTextureArray,const std::vector<osg::Texture2D*>& aTextureLMapArray,
                                  osg::Vec3Array& aVertexArray,
                                  osg::Vec2Array& aTextureDecalCoords,osg::Vec2Array& aTextureLMapCoords
                                 ) const
{
  osg::Texture2D *texture=aTextureArray[aLoadFace.m_texture];

  osg::Geometry* polygon_geom = new osg::Geometry;
  polygon_geom->setVertexArray(&aVertexArray);
  polygon_geom->setTexCoordArray(0, &aTextureDecalCoords);
  polygon_geom->setTexCoordArray(1, &aTextureLMapCoords);

  osg::DrawArrays* face_indices = new osg::DrawArrays(osg::PrimitiveSet::TRIANGLE_FAN,
                                                      aLoadFace.m_firstVertexIndex,
                                                      aLoadFace.m_numVertices
                                                     );

  osg::StateSet* stateset = polygon_geom->getOrCreateStateSet();
  if(texture)
    {
      stateset->setTextureAttributeAndModes(0,texture,osg::StateAttribute::ON);
      if(aLoadFace.m_lightmapIndex>=0)
        {
          texture=aTextureLMapArray[aLoadFace.m_lightmapIndex];
          if(texture)
            {
              stateset->setTextureAttributeAndModes(1,texture,osg::StateAttribute::ON);
            }
        }
      else
        {
          texture=aTextureLMapArray[aTextureLMapArray.size()-1];
          if(texture)
            {
              stateset->setTextureAttributeAndModes(1,texture,osg::StateAttribute::ON);
            }
        }

    }
  else
    {
      osg::PolygonMode* polygon_mode=new osg::PolygonMode;
      polygon_mode->setMode(osg::PolygonMode::FRONT_AND_BACK,osg::PolygonMode::LINE);
      stateset->setAttributeAndModes(polygon_mode, osg::StateAttribute::ON);
    }

  polygon_geom->addPrimitiveSet(face_indices);
  return polygon_geom;
}












bool ReaderWriterQ3BSP::loadTextures(const BSPLoad& aLoadData,std::vector<osg::Texture2D*>& aTextureArray) const
{
  int num_textures=aLoadData.m_loadTextures.size();

  int i;
  for(i=0;i<num_textures;i++)
    {
      //add file extension to the name
      std::string tgaExtendedName(aLoadData.m_loadTextures[i].m_name);
      tgaExtendedName+=".tga";
      std::string jpgExtendedName(aLoadData.m_loadTextures[i].m_name);
      jpgExtendedName+=".jpg";

      osg::Image* image = osgDB::readImageFile(tgaExtendedName);
      if (!image)
        {
          image = osgDB::readImageFile(jpgExtendedName);
          if (!image)
            {
              aTextureArray.push_back(NULL);
              continue; //?
            }
        }

      osg::Texture2D* texture= new osg::Texture2D;
      texture->setImage(image);
      texture->setDataVariance(osg::Object::DYNAMIC); // protect from being optimized away as static state.
      texture->setWrap(osg::Texture2D::WRAP_S, osg::Texture2D::REPEAT);
      texture->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::REPEAT);
      aTextureArray.push_back(texture);
    }

  return true;
}





bool ReaderWriterQ3BSP::loadLightMaps(const BSPLoad& aLoadData,std::vector<osg::Texture2D*>& aTextureArray) const
{
  int num_textures=aLoadData.m_loadLightmaps.size();

  int i;
  for(i=0;i<num_textures;i++)
    {
      osg::Image* image=new osg::Image;

      unsigned char *data=new unsigned char[128*128*3];
      memcpy(data,aLoadData.m_loadLightmaps[i].m_lightmapData,128*128*3);

      image->setImage(128,128,1,GL_RGBA8,GL_RGB,GL_UNSIGNED_BYTE,data,osg::Image::USE_NEW_DELETE);

      osg::Texture2D* texture= new osg::Texture2D;
      texture->setImage(image);
      texture->setDataVariance(osg::Object::DYNAMIC); // protect from being optimized away as static state.
      texture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR_MIPMAP_LINEAR);
      texture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);
      texture->setWrap(osg::Texture2D::WRAP_S, osg::Texture2D::REPEAT);
      texture->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::REPEAT);
      aTextureArray.push_back(texture);
    }



  // A continuación, añado el blanco
  osg::Image* image=new osg::Image;
  unsigned char *data=new unsigned char[3];
  for(int whiteidx=0;whiteidx<3;whiteidx++)
  {
      data[whiteidx]=255;
  }

  image->setImage(1,1,1,GL_RGBA8,GL_RGB,GL_UNSIGNED_BYTE,data,osg::Image::USE_NEW_DELETE);

  osg::Texture2D* texture= new osg::Texture2D;
  texture->setImage(image);
  texture->setDataVariance(osg::Object::DYNAMIC); // protect from being optimized away as static state.
  texture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR_MIPMAP_LINEAR);
  texture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);
  texture->setWrap(osg::Texture2D::WRAP_S, osg::Texture2D::REPEAT);
  texture->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::REPEAT);
  aTextureArray.push_back(texture);


  return true;
}


//Tesselate a biquadratic patch
bool BSP_BIQUADRATIC_PATCH::Tesselate(int newTesselation,osg::Geometry* aGeometry)
{
    m_tesselation=newTesselation;

    float px, py;
    BSP_VERTEX temp[3];
    m_vertices.resize((m_tesselation+1)*(m_tesselation+1));

    for(int v=0; v<=m_tesselation; ++v)
      {
        px=(float)v/m_tesselation;

        m_vertices[v]=m_controlPoints[0]*((1.0f-px)*(1.0f-px))+
                      m_controlPoints[3]*((1.0f-px)*px*2)+
                      m_controlPoints[6]*(px*px);
      }

    for(int u=1; u<=m_tesselation; ++u)
      {
        py=(float)u/m_tesselation;

        temp[0]=m_controlPoints[0]*((1.0f-py)*(1.0f-py))+
                m_controlPoints[1]*((1.0f-py)*py*2)+
                m_controlPoints[2]*(py*py);

        temp[1]=m_controlPoints[3]*((1.0f-py)*(1.0f-py))+
                m_controlPoints[4]*((1.0f-py)*py*2)+
                m_controlPoints[5]*(py*py);

        temp[2]=m_controlPoints[6]*((1.0f-py)*(1.0f-py))+
                m_controlPoints[7]*((1.0f-py)*py*2)+
                m_controlPoints[8]*(py*py);

        for(int v=0; v<=m_tesselation; ++v)
          {
            px=(float)v/m_tesselation;

            m_vertices[u*(m_tesselation+1)+v]=temp[0]*((1.0f-px)*(1.0f-px))+
                                              temp[1]*((1.0f-px)*px*2)+
                                              temp[2]*(px*px);
          }
      }

    //Create indices
    m_indices.resize(m_tesselation*(m_tesselation+1)*2);

    int row;
    for(row=0; row<m_tesselation; ++row)
      {
        for(int point=0; point<=m_tesselation; ++point)
          {
            //calculate indices
            //reverse them to reverse winding
            m_indices[(row*(m_tesselation+1)+point)*2+1]= row*(m_tesselation+1)+point;
            m_indices[(row*(m_tesselation+1)+point)*2]=  (row+1)*(m_tesselation+1)+point;
          }
      }


    //Fill in the arrays for multi_draw_arrays
    m_trianglesPerRow.resize(m_tesselation);
    m_rowIndexPointers.resize(m_tesselation);

    for(row=0; row<m_tesselation; ++row)
      {
        m_trianglesPerRow[row]=2*(m_tesselation+1);
        m_rowIndexPointers[row]=&m_indices[row*2*(m_tesselation+1)];
      }


    osg::Vec3Array* patch_vertex_array = new osg::Vec3Array( (m_tesselation+1)*(m_tesselation+1) );
    osg::Vec2Array* patch_textcoord_array = new osg::Vec2Array( (m_tesselation+1)*(m_tesselation+1) );
    osg::Vec2Array* patch_lmapcoord_array = new osg::Vec2Array( (m_tesselation+1)*(m_tesselation+1) );
    for(int i=0;i<(m_tesselation+1)*(m_tesselation+1);i++)
      {
        (*patch_vertex_array)[i].set( m_vertices[ i ].m_position[0],
                                      m_vertices[ i ].m_position[1],
                                      m_vertices[ i ].m_position[2]
                                    );

        (*patch_textcoord_array)[i].set( m_vertices[ i ].m_decalS,
                                         m_vertices[ i ].m_decalT
                                       );

        (*patch_lmapcoord_array)[i].set( m_vertices[ i ].m_lightmapS,
                                         m_vertices[ i ].m_lightmapT
                                       );


      }
    aGeometry->setVertexArray(patch_vertex_array);
    aGeometry->setTexCoordArray(0,patch_textcoord_array);
    aGeometry->setTexCoordArray(1,patch_lmapcoord_array);


    for(row=0; row<m_tesselation; ++row)
      {
        osg::DrawElementsUInt* face_indices = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLE_STRIP,
                                                        m_tesselation*(m_tesselation+1)*2,
                                                        &m_indices[0]
                                                        );
        aGeometry->addPrimitiveSet(face_indices);

      }



    return true;
}





