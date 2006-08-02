

#ifndef BSPLOAD_H
#define BSPLOAD_H


#include <osg/Vec3f>
#include <osg/GL>

#include <vector>
#include <string>

#include <fstream>

//Directory entry in header
class BSP_DIRECTORY_ENTRY
{
public:
    int m_offset;
    int m_length;
};

//Types of directory entry
enum BSP_DIRECTORY_ENTRY_TYPE
{
    bspEntities=0,
    bspTextures,
    bspPlanes,
    bspNodes,
    bspLeaves,
    bspLeafFaces,
    bspLeafBrushes,
    bspModels,
    bspBrushes,
    bspBrushSides,
    bspVertices,
    bspMeshIndices,
    bspEffect,
    bspFaces,
    bspLightmaps,
    bspLightVols,
    bspVisData
};



//BSP file header
class BSP_HEADER
{
public:
  char m_string[4];
  int m_version;
  BSP_DIRECTORY_ENTRY m_directoryEntries[17];
};



//vertex as found in file
class BSP_LOAD_VERTEX
{
public:
  osg::Vec3f m_position;
  float m_decalS, m_decalT;
  float m_lightmapS, m_lightmapT;
  osg::Vec3f m_normal;
  unsigned char m_color[4];
};

struct BSP_LoadPlane
{
  osg::Vec3f m_Normal;
  float      m_Dist;
};



//face as found in the file
class BSP_LOAD_FACE
{
public:
  int m_texture;
  int m_effect;  // No se usa
  int m_type;
  int m_firstVertexIndex;
  int m_numVertices;
  unsigned int m_firstMeshIndex;
  unsigned int m_numMeshIndices;
  int m_lightmapIndex;
  int m_lightmapStart[2]; // No se usa
  int m_lightmapSize[2];  // No se usa
  //VECTOR3D m_lightmapOrigin;  // No se usa
  //VECTOR3D m_sTangent, m_tTangent;  // No se usa
  //VECTOR3D m_normal;  // No se usa

  osg::Vec3f m_lightmapOrigin;  // No se usa
  osg::Vec3f m_sTangent, m_tTangent;  // No se usa
  osg::Vec3f m_normal;  // No se usa


  int m_patchSize[2];
};






//texture as found in file
class BSP_LOAD_TEXTURE
{
public:
  char m_name[64];
  int m_flags, m_contents;    //unknown, no se usa
};

//lightmap as found in file
class BSP_LOAD_LIGHTMAP
{
public:
  unsigned char m_lightmapData[128*128*3];
};





//leaf of bsp tree as found in file
class BSP_LOAD_LEAF
{
public:
  int m_cluster;    //cluster index for visdata
  int m_area;       //areaportal area,   No se usa
  int m_mins[3];    //min x,y,z (bounding box)
  int m_maxs[3];
  int m_firstLeafFace;  //first index in leafFaces array
  int m_numFaces;
  int m_firstLeafBrush; //first index into leaf brushes array, No se usa
  int m_numBrushes;     // No se usa
};




//node of BSP tree
class BSP_NODE
{
public:
  int m_planeIndex;
  int m_front, m_back;    //child nodes
  int m_mins[3];    //min x,y,z (bounding box)  No se usa
  int m_maxs[3];    // No se usa
};




//VIS data table
class BSP_VISIBILITY_DATA
{
public:
  int m_numClusters;
  int m_bytesPerCluster;
  std::vector<unsigned char> m_bitset;

};







class BSPLoad
{
public:

  bool Load(const std::string& filename, int curveTesselation);
  void LoadVertices(std::ifstream& aFile);
  void LoadFaces(std::ifstream& aFile, int curveTesselation);
  void LoadTextures(std::ifstream& aFile);
  void LoadLightmaps(std::ifstream& aFile);
  void LoadBSPData(std::ifstream& aFile);


  std::string m_entityString;
  //header
  BSP_HEADER m_header;

  // Load Data
  std::vector<BSP_LOAD_VERTEX>    m_loadVertices;
  std::vector<GLuint>             m_loadMeshIndices;
  std::vector<BSP_LOAD_FACE>      m_loadFaces;
  std::vector<BSP_LOAD_TEXTURE>   m_loadTextures;
  std::vector<BSP_LOAD_LIGHTMAP>  m_loadLightmaps;
  std::vector<BSP_LOAD_LEAF>      m_loadLeaves;
  std::vector<int>                m_loadLeafFaces;
  std::vector<BSP_LoadPlane>      m_loadPlanes;
  std::vector<BSP_NODE>           m_loadNodes;
  BSP_VISIBILITY_DATA             m_loadVisibilityData;
};



#endif  // BSPLOAD_H 

