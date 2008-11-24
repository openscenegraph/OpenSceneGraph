// El siguiente bloque ifdef muestra la forma estándar de crear macros que facilitan 
// la exportación de archivos DLL. Todos los archivos de este archivo DLL se compilan con el símbolo Q3BSP_EXPORTS
// definido en la línea de comandos. Este símbolo no se debe definir en ningún proyecto
// que utilice este archivo DLL. De este modo, otros proyectos cuyos archivos de código fuente incluyan el archivo
// interpreta que las funciones Q3BSP_API se importan de un archivo DLL, mientras que este archivo DLL interpreta los símbolos
// definidos en esta macro como si fueran exportados.
/*
#ifdef Q3BSP_EXPORTS
#define Q3BSP_API __declspec(dllexport)
#else
#define Q3BSP_API __declspec(dllimport)
#endif

// Clase exportada de q3bsp.dll
class Q3BSP_API Cq3bsp {
public:
    Cq3bsp(void);
    // TODO: agregar métodos aquí.
};

extern Q3BSP_API int nq3bsp;

Q3BSP_API int fnq3bsp(void);
*/


#include <osg/Array>
#include <osg/Node>
#include <osg/Texture2D>
#include <osgDB/ReadFile>

#include "Q3BSPLoad.h"


namespace bsp
{


class Q3BSPReader
{
public:

    Q3BSPReader();

    bool readFile(const std::string& fileName,
                  const osgDB::ReaderWriter::Options*);

    osg::ref_ptr<osg::Node>  getRootNode();

private:

    osg::ref_ptr<osg::Node>   root_node;

    osg::Geode* convertFromBSP(Q3BSPLoad& aLoadData,
                               const osgDB::ReaderWriter::Options*) const;

    osg::Geometry* createMeshFace(
                            const BSP_LOAD_FACE& aLoadFace,
                            const std::vector<osg::Texture2D*>& aTextureArray,
                            osg::Vec3Array& aVertexArray,
                            std::vector<GLuint>& aIndices,
                            osg::Vec2Array& aTextureDecalCoords,
                            osg::Vec2Array& aTextureLMapCoords) const;

    osg::Geometry* createPolygonFace(
                         const BSP_LOAD_FACE& aLoadFace,
                         const std::vector<osg::Texture2D*>& aTextureArray,
                         const std::vector<osg::Texture2D*>& aTextureLMapArray,
                         osg::Vec3Array& aVertexArray,
                         osg::Vec2Array& aTextureDecalCoords,
                         osg::Vec2Array& aTextureLMapCoords) const;

    bool        loadTextures(
                          const Q3BSPLoad& aLoadData,
                          std::vector<osg::Texture2D*>& aTextureArray) const;

    bool        loadLightMaps(
                           const Q3BSPLoad& aLoadData,
                           std::vector<osg::Texture2D*>& aTextureArray) const;
};


}
