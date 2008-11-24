
#include "Q3BSPLoad.h"


using namespace bsp;


bool Q3BSPLoad::Load(const std::string& filename, int curveTessellation)
{
  std::ifstream file(filename.c_str(),std::ios::binary);
  if(!file.is_open())
    {
      //errorLog.OutputError("Unable to open %s", filename);
      return false;
    }

  //read in header
  file.read((char*)&m_header, sizeof(BSP_HEADER));

  //check header data is correct
  if( m_header.m_string[0]!='I' || m_header.m_string[1]!='B' ||
      m_header.m_string[2]!='S' || m_header.m_string[3]!='P' ||
      m_header.m_version  !=0x2E )
    {
      //errorLog.OutputError("%s is not a version 0x2E .bsp map file", filename);
      return false;
    }


  //Load in vertices
  LoadVertices(file);

  //Load in mesh indices
  //Calculate number of indices
  int numMeshIndices=m_header.m_directoryEntries[bspMeshIndices].m_length/sizeof(int);

  //Create space
  m_loadMeshIndices.resize(numMeshIndices);

  //read in the mesh indices
  file.seekg(m_header.m_directoryEntries[bspMeshIndices].m_offset,std::ios::beg);
  file.read((char*) &m_loadMeshIndices[0], m_header.m_directoryEntries[bspMeshIndices].m_length);


  //Load in faces
  LoadFaces(file, curveTessellation);
  
  //Load textures
  LoadTextures(file);

  //Load Lightmaps
  LoadLightmaps(file);

  //Load BSP Data
  LoadBSPData(file);

  //Load in entity string
  m_entityString.resize(m_header.m_directoryEntries[bspEntities].m_length);

  //Go to entity string in file
  file.seekg(m_header.m_directoryEntries[bspEntities].m_offset,std::ios::beg);
  file.read(&m_entityString[0], m_header.m_directoryEntries[bspEntities].m_length);

  //errorLog.OutputSuccess("%s Loaded successfully", filename);
  return true;
}






void Q3BSPLoad::LoadVertices(std::ifstream& aFile)
{
  //calculate number of vertices
  int num_vertices=m_header.m_directoryEntries[bspVertices].m_length/sizeof(BSP_LOAD_VERTEX);

  //Create space for this many BSP_LOAD_VERTICES
  m_loadVertices.resize(num_vertices);

  //go to vertices in file
  aFile.seekg(m_header.m_directoryEntries[bspVertices].m_offset,std::ios::beg);

  //read in the vertices
  aFile.read((char*)&m_loadVertices[0], m_header.m_directoryEntries[bspVertices].m_length);
}










void Q3BSPLoad::LoadFaces(std::ifstream& aFile, int /*curveTessellation*/)
{
  //calculate number of load faces
  int numTotalFaces=m_header.m_directoryEntries[bspFaces].m_length/sizeof(BSP_LOAD_FACE);

  //Create space for this many BSP_LOAD_FACES
  m_loadFaces.resize(numTotalFaces);

  //go to faces in file
  aFile.seekg(m_header.m_directoryEntries[bspFaces].m_offset,std::ios::beg);

  //read in the faces
  aFile.read((char*)&m_loadFaces[0], m_header.m_directoryEntries[bspFaces].m_length);
}













void Q3BSPLoad::LoadTextures(std::ifstream& aFile)
{
    //Calculate number of textures
    int num_textures=m_header.m_directoryEntries[bspTextures].m_length/sizeof(BSP_LOAD_TEXTURE);

    //Create space for this many BSP_LOAD_TEXTUREs
    m_loadTextures.resize(num_textures);

    //Load textures
    aFile.seekg(m_header.m_directoryEntries[bspTextures].m_offset,std::ios::beg);
    aFile.read((char*)&m_loadTextures[0], m_header.m_directoryEntries[bspTextures].m_length);
}











void Q3BSPLoad::LoadLightmaps(std::ifstream& aFile)
{
    //Calculate number of lightmaps
    int num_lightmaps=m_header.m_directoryEntries[bspLightmaps].m_length/sizeof(BSP_LOAD_LIGHTMAP);

    //Create space for this many BSP_LOAD_LIGHTMAPs
    m_loadLightmaps.resize(num_lightmaps);

    //Load textures
    aFile.seekg(m_header.m_directoryEntries[bspLightmaps].m_offset,std::ios::beg);
    aFile.read((char*)&m_loadLightmaps[0], m_header.m_directoryEntries[bspLightmaps].m_length);

    //Change the gamma settings on the lightmaps (make them brighter)
    float gamma=2.5f;
    for(int i=0; i<num_lightmaps; ++i)
      {
        for(int j=0; j<128*128; ++j)
          {
            float r, g, b;
            r=m_loadLightmaps[i].m_lightmapData[j*3+0];
            g=m_loadLightmaps[i].m_lightmapData[j*3+1];
            b=m_loadLightmaps[i].m_lightmapData[j*3+2];

            r*=gamma/255.0f;
            g*=gamma/255.0f;
            b*=gamma/255.0f;

            //find the value to scale back up
            float scale=1.0f;
            float temp;
            if(r > 1.0f && (temp = (1.0f/r)) < scale) scale=temp;
            if(g > 1.0f && (temp = (1.0f/g)) < scale) scale=temp;
            if(b > 1.0f && (temp = (1.0f/b)) < scale) scale=temp;

            // scale up color values
            scale*=255.0f;      
            r*=scale;
            g*=scale;
            b*=scale;

            //fill data back in
            m_loadLightmaps[i].m_lightmapData[j*3+0]=(unsigned char)r;
            m_loadLightmaps[i].m_lightmapData[j*3+1]=(unsigned char)g;
            m_loadLightmaps[i].m_lightmapData[j*3+2]=(unsigned char)b;
            //m_loadLightmaps[i].m_lightmapData[j*3+0]=(GLubyte)255;
            //m_loadLightmaps[i].m_lightmapData[j*3+1]=(GLubyte)255;
            //m_loadLightmaps[i].m_lightmapData[j*3+2]=(GLubyte)255;
          }
      }
}










void Q3BSPLoad::LoadBSPData(std::ifstream& aFile)
{
  //Load leaves
  //Calculate number of leaves
  int numLeaves=m_header.m_directoryEntries[bspLeaves].m_length/sizeof(BSP_LOAD_LEAF);

  //Create space for this many BSP_LOAD_LEAFS
  m_loadLeaves.resize(numLeaves);

  //Load leaves
  aFile.seekg(m_header.m_directoryEntries[bspLeaves].m_offset,std::ios::beg);
  aFile.read((char*)&m_loadLeaves[0], m_header.m_directoryEntries[bspLeaves].m_length);




  //Load leaf faces array
  int num_leaf_faces=m_header.m_directoryEntries[bspLeafFaces].m_length/sizeof(int);

  //Create space for this many leaf faces
  m_loadLeafFaces.resize(num_leaf_faces);

  //Load leaf faces
  aFile.seekg(m_header.m_directoryEntries[bspLeafFaces].m_offset,std::ios::beg);
  aFile.read((char*)&m_loadLeafFaces[0], m_header.m_directoryEntries[bspLeafFaces].m_length);





  //Load Planes
  int num_planes=m_header.m_directoryEntries[bspPlanes].m_length/sizeof(BSP_LoadPlane);

  //Create space for this many planes
  m_loadPlanes.resize(num_planes);

  aFile.seekg(m_header.m_directoryEntries[bspPlanes].m_offset,std::ios::beg);
  aFile.read((char*)&m_loadPlanes[0], m_header.m_directoryEntries[bspPlanes].m_length);



  //Load nodes
  int num_nodes=m_header.m_directoryEntries[bspNodes].m_length/sizeof(BSP_NODE);

  //Create space for this many nodes
  m_loadNodes.resize(num_nodes);

  aFile.seekg(m_header.m_directoryEntries[bspNodes].m_offset,std::ios::beg);
  aFile.read((char*)&m_loadNodes[0], m_header.m_directoryEntries[bspNodes].m_length);



  //Load visibility data
  //load numClusters and bytesPerCluster
  aFile.seekg(m_header.m_directoryEntries[bspVisData].m_offset,std::ios::beg);
  aFile.read((char*)&m_loadVisibilityData, 2 * sizeof(int));

  //Calculate the size of the bitset
  int bitsetSize=m_loadVisibilityData.m_numClusters*m_loadVisibilityData.m_bytesPerCluster;

  //Create space for bitset
  m_loadVisibilityData.m_bitset.resize(bitsetSize);
  //read bitset
  aFile.read((char*)&m_loadVisibilityData.m_bitset[0], bitsetSize);
}








