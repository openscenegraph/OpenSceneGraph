/*    
 *  Wavefront .obj file format reader.
 *
 *  author: Nate Robins
 *  email: ndr@pobox.com
 *  www: http://www.pobox.com/~ndr
 */


/* includes */
/* #include "glut.h" */
//#include <GL/glu.h>
//#include<GL/glut.h>

// Replace about glu/glut calls with the more x-platform friendly osg version,
// also neither glu or glut were requied?!  Only gl.h... even this I think
// should be removed since all the GLfloat etc could just as easily be floats.
// Will leave for now as it works... Robert Osfield, June 5th 2001. 
#include <osg/GL>
#include <osg/UByte4>

#ifndef M_PI
#define M_PI 3.14159265
#endif


/* defines */
#define GLM_NONE     (0)        /* render with only vertices */
#define GLM_FLAT     (1 << 0)        /* render with facet normals */
#define GLM_SMOOTH   (1 << 1)        /* render with vertex normals */
#define GLM_TEXTURE  (1 << 2)        /* render with texture coords */
#define GLM_COLOR    (1 << 3)        /* render with colors */
#define GLM_MATERIAL (1 << 4)        /* render with materials */


/* structs */

/* GLMmaterial: Structure that defines a material in a model. 
 */
struct GLMmaterial
{
  char*   name;                /* name of material */
  GLfloat diffuse[4];            /* diffuse component */
  GLfloat ambient[4];            /* ambient component */
  GLfloat specular[4];            /* specular component */
  GLfloat emmissive[4];            /* emmissive component */
  GLfloat shininess;            /* specular exponent */
  char*   textureName;                  /* name of any attached texture, add by RO */
  bool    textureReflection;            /* true if texture is a reflection map */    
  float   alpha;                /* alpha */

  void init()
  {
    name = NULL;
    diffuse[0] = diffuse[1] = diffuse[2] = 0.8f; diffuse[3] = 1.0f;
    ambient[0] = ambient[1] = ambient[2] = 0.2f; ambient[3] = 1.0f;
    specular[0] = specular[1] = specular[2] = 0.0f; specular[3] = 1.0f;
    emmissive[0] = emmissive[1] = emmissive[2] = 0.0f; emmissive[3] = 1.0f;
    shininess = 0.0f;
    textureName = NULL;
    textureReflection = false;
    alpha = 1.0f;
  }

};

/* GLMtriangle: Structure that defines a triangle in a model.
 */
struct GLMtriangle {
  unsigned int vindices[3];            /* array of triangle vertex indices */
  unsigned int nindices[3];            /* array of triangle normal indices */
  unsigned int tindices[3];            /* array of triangle texcoord indices*/
  unsigned int findex;                /* index of triangle facet normal */
  void init()
  {
    vindices[0] = vindices[2] = vindices[2] = 0 ;
    nindices[0] = nindices[2] = nindices[2] = 0 ;
    tindices[0] = tindices[2] = tindices[2] = 0 ;
    findex=0;
  }
};

/* GLMgroup: Structure that defines a group in a model.
 */
struct GLMgroup {
  char*             name;        /* name of this group */
  unsigned int            numtriangles;    /* number of triangles in this group */
  unsigned int*           triangles;        /* array of triangle indices */
  unsigned int            material;           /* index to material for group */
  bool              hastexcoords;       /* set to true if triangles have texture coords */
  struct GLMgroup*  next;        /* pointer to next group in model */

  void init()
  {
    name = NULL;        
    numtriangles = 0;    
    triangles = NULL;    
    material = 0;       
        hastexcoords = false;
    next = NULL;        
  }
};

/* GLMmodel: Structure that defines a model.
 */
struct  GLMmodel {
  char*    pathname;            /* path to this model */
  char*    mtllibname;            /* name of the material library */

  unsigned int   numvertices;            /* number of vertices in model */
  GLfloat* vertices;            /* array of vertices  */

  bool useColors;               /* true if per vertex colors are present.*/
  osg::UByte4* colors;            /* array of per vertex colors  */

  unsigned int   numnormals;            /* number of normals in model */
  GLfloat* normals;            /* array of normals */

  unsigned int   numtexcoords;        /* number of texcoords in model */
  GLfloat* texcoords;            /* array of texture coordinates */

  unsigned int   numfacetnorms;        /* number of facetnorms in model */
  GLfloat* facetnorms;            /* array of facetnorms */

  unsigned int       numtriangles;        /* number of triangles in model */
  GLMtriangle* triangles;        /* array of triangles */

  unsigned int       nummaterials;        /* number of materials in model */
  GLMmaterial* materials;        /* array of materials */

  unsigned int       numgroups;        /* number of groups in model */
  GLMgroup*    groups;            /* linked list of groups */

  GLfloat position[3];            /* position of the model */


  void init()
  {
    pathname = NULL;    
    mtllibname = NULL;    

    numvertices = 0;    
    vertices = NULL;
    
    useColors = false;
    colors = NULL;

    numnormals = 0;        
    normals = NULL;    

    numtexcoords = 0;    
    texcoords = NULL;

    numfacetnorms = 0;    
    facetnorms = NULL;

    numtriangles = 0;    
    triangles = NULL;        

    nummaterials = 0;    
    materials = NULL;

    numgroups = 0;
    groups = NULL;

    position[0] = position[1] = position[2] = 0.0f;

  }

};


/* public functions */

/* glmUnitize: "unitize" a model by translating it to the origin and
 * scaling it to fit in a unit cube around the origin.  Returns the
 * scalefactor used.
 *
 * model - properly initialized GLMmodel structure 
 */
GLfloat
glmUnitize(GLMmodel* model);

/* glmDimensions: Calculates the dimensions (width, height, depth) of
 * a model.
 *
 * model      - initialized GLMmodel structure
 * dimensions - array of 3 GLfloats (GLfloat dimensions[3])
 */
GLvoid
glmDimensions(GLMmodel* model, GLfloat* dimensions);

/* glmScale: Scales a model by a given amount.
 * 
 * model - properly initialized GLMmodel structure
 * scale - scalefactor (0.5 = half as large, 2.0 = twice as large)
 */
GLvoid
glmScale(GLMmodel* model, GLfloat scale);

/* glmReverseWinding: Reverse the polygon winding for all polygons in
 * this model.  Default winding is counter-clockwise.  Also changes
 * the direction of the normals.
 * 
 * model - properly initialized GLMmodel structure 
 */
GLvoid
glmReverseWinding(GLMmodel* model);

/* glmFacetNormals: Generates facet normals for a model (by taking the
 * cross product of the two vectors derived from the sides of each
 * triangle).  Assumes a counter-clockwise winding.
 *
 * model - initialized GLMmodel structure
 */
GLvoid
glmFacetNormals(GLMmodel* model);

/* glmVertexNormals: Generates smooth vertex normals for a model.
 * First builds a list of all the triangles each vertex is in.  Then
 * loops through each vertex in the the list averaging all the facet
 * normals of the triangles each vertex is in.  Finally, sets the
 * normal index in the triangle for the vertex to the generated smooth
 * normal.  If the dot product of a facet normal and the facet normal
 * associated with the first triangle in the list of triangles the
 * current vertex is in is greater than the cosine of the angle
 * parameter to the function, that facet normal is not added into the
 * average normal calculation and the corresponding vertex is given
 * the facet normal.  This tends to preserve hard edges.  The angle to
 * use depends on the model, but 90 degrees is usually a good start.
 *
 * model - initialized GLMmodel structure
 * angle - maximum angle (in degrees) to smooth across
 */
GLvoid
glmVertexNormals(GLMmodel* model, GLfloat angle);

/* glmLinearTexture: Generates texture coordinates according to a
 * linear projection of the texture map.  It generates these by
 * linearly mapping the vertices onto a square.
 *
 * model - pointer to initialized GLMmodel structure
 */
GLvoid
glmLinearTexture(GLMmodel* model);

/* glmSpheremapTexture: Generates texture coordinates according to a
 * spherical projection of the texture map.  Sometimes referred to as
 * spheremap, or reflection map texture coordinates.  It generates
 * these by using the normal to calculate where that vertex would map
 * onto a sphere.  Since it is impossible to map something flat
 * perfectly onto something spherical, there is distortion at the
 * poles.  This particular implementation causes the poles along the X
 * axis to be distorted.
 *
 * model - pointer to initialized GLMmodel structure
 */
GLvoid
glmSpheremapTexture(GLMmodel* model);

/* glmDelete: Deletes a GLMmodel structure.
 *
 * model - initialized GLMmodel structure
 */
GLvoid
glmDelete(GLMmodel* model);

/* glmReadOBJ: Reads a model description from a Wavefront .OBJ file.
 * Returns a pointer to the created object which should be free'd with
 * glmDelete().
 *
 * filename - name of the file containing the Wavefront .OBJ format data.  
 */
GLMmodel* 
glmReadOBJ(char* filename);

/* glmWriteOBJ: Writes a model description in Wavefront .OBJ format to
 * a file.
 *
 * model    - initialized GLMmodel structure
 * filename - name of the file to write the Wavefront .OBJ format data to
 * mode     - a bitwise or of values describing what is written to the file
 *            GLM_NONE    -  write only vertices
 *            GLM_FLAT    -  write facet normals
 *            GLM_SMOOTH  -  write vertex normals
 *            GLM_TEXTURE -  write texture coords
 *            GLM_FLAT and GLM_SMOOTH should not both be specified.
 */
GLvoid
glmWriteOBJ(GLMmodel* model, char* filename, unsigned int mode);


/* glmWeld: eliminate (weld) vectors that are within an epsilon of
 * each other.
 *
 * model      - initialized GLMmodel structure
 * epsilon    - maximum difference between vertices
 *              ( 0.00001 is a good start for a unitized model)
 *
 */
GLvoid
glmWeld(GLMmodel* model, GLfloat epsilon);
