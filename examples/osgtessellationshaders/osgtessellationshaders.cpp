/* A demonstration of Tessellation Shaders in OpenScenegraph.
 *
 * Instructions:
 *   Press plus to increase tesselation and minus to decrease it.
 *   Press right arrow to increase inner tesselation and left arrow to decrease it.
 *   Press up arrow to increase outer tesselation and down arrow to decrease it.
 *
 * Original code by Philip Rideout
 * Adapted to OpenScenegraph by John Kaniarz
 * Additional work by Michael Mc Donnell
 */

#include <osg/Program>
#include <osg/PatchParameter>
#include <osg/Texture3D>
#include <osg/ShapeDrawable>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgGA/TrackballManipulator>

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include "osgvolume.cpp"
///Node used in order to readback image from texture
	///Put it where you want it removes itself after one shot
	class  ImageReadBackNode :public osg::Geode{

	public:
		META_Node(osgMG, ImageReadBackNode);
		ImageReadBackNode(){}
		ImageReadBackNode(const ImageReadBackNode& rhs, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY);
		ImageReadBackNode(osg::Texture3D * tex) :ImageReadBackNode(){
			setTexture(tex);
		}
		void setTexture(osg::Texture3D * tex);


	};

/**update call back writing back from RAM to HDD*/
class ImageWriterCallback:public osg::NodeCallback
{
    osg::ref_ptr<osg::Image> _daim;
    osg::ref_ptr<osgDB::Options> _options;

public:

    void setOptions(osgDB::Options* opt)
    {
        _options=opt;
    }
    void setImage(osg::Image* im)
    {
        _daim=im;
    }
    virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
    {
        if(!_daim->getModifiedCount()==0)
        {

            osgDB::writeImageFile(*_daim,_daim->getFileName(),_options);
_daim->dirty();
            node->removeUpdateCallback(this);

        }
    }
    ImageWriterCallback( )
    {
        _options=new osgDB::Options;
        _daim=0;
    }
    ImageWriterCallback( osg::Image* im,osgDB::Options* opt=new osgDB::Options)
    {
        _daim=im;
        _options=opt;
    }
};

ImageReadBackNode::ImageReadBackNode(const ImageReadBackNode& rhs, const osg::CopyOp& copyop ) :osg::Geode(rhs, copyop)
{


}
void ImageReadBackNode::setTexture(osg::Texture3D*tex)
{
    //assert(tex->getImage());
    class ImageReadBackNodeCallback : public osg::Drawable::DrawCallback
    {
        osg::Texture3D *_texture;
        osg::Node *_n;
    public:
        ImageReadBackNodeCallback(osg::Texture3D *texture, osg::Node*toremove) :_texture(texture), _n(toremove) {  };
        virtual void drawImplementation(osg::RenderInfo& renderInfo,const osg::Drawable* drawable) const

        //virtual void operator () (osg::RenderInfo& renderInfo) const
        {

 
if (osg::isGLExtensionSupported(renderInfo.getContextID(),"GL_NV_conservative_raster"))
std::cout<<"yeah::Fermi"<<std::endl;
else std::cout<<"no::Fermi"<<std::endl;
        _texture->apply(*renderInfo.getState());
            osg::Image *texim=_texture->getImage();

            texim->readImageFromCurrentTexture(renderInfo.getContextID(),	 true,texim->getDataType () );//_texture->getSourceType());
             texim->dirty();
            std::list<osg::Group*> gr;
            for (int i = 0; i < _n->getNumParents(); i++)if(_n->getParent(i))gr.push_back((_n->getParent(i)));
            int cpt=0;
        /*    for (std::list<osg::Group*>::iterator it = gr.begin(); it != gr.end(); it++)
            {
///THREADSAFE?
                (*it)->removeChild(_n);

                if(cpt!=0)std::cerr<<" WARNING ImageReadBackNode with MULTIPLE PARENTS "<<gr.size()<<std::endl;
                cpt++;

                ///add a Callbakck to write back image to HDD
               if(!texim->getFileName().empty())
                {
                    osg::ref_ptr< osgDB::Options> options=new osgDB::Options;
                    options->setOptionString(options->getOptionString() + " compressed=1");
                    (*it)->addUpdateCallback(new ImageWriterCallback(texim,options));
                }
            }*/
        }
    };



    getOrCreateStateSet()->setTextureAttributeAndModes(0, tex, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
    osg::ref_ptr<osg::Texture2D> tex2D = new osg::Texture2D;
    getOrCreateStateSet()->setAttributeAndModes( tex2D, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE );

    osg::ref_ptr<osg::Geometry> geom=new osg::Geometry();
    geom->setUseVertexBufferObjects(true);
    geom->setUseDisplayList(false);

    geom->setDrawCallback(new ImageReadBackNodeCallback(tex,this));
    this->addDrawable(geom);
    //this->addChild(cam);

}
static const char* vertSource = {"//#version 330\n"\
                               "\n"\
                               "\n"\
                               //"varying vec3 vNormali;\n "
                               ///   "varying int instance;\n "
//"in vec4 VertexPosition;\n"\
                               "in vec3 VertexNormal;\n"\
                               "out int instance;\n"\
                               "out vec4 OrigPosition;\n"\
                               "out vec3 vNormal;\n"
                               "\n"\
                               "void main(void)\n"\
                               "{\n"\
//"OrigPosition =vec4(VertexPosition.xyz,1);\n"\
//"vNormal=VertexNormal;   \n"
                               ///   "instance=gl_InstanceID;//+vec3(0.5,0.5,0.5)\n"

                               //"vNormali = gl_Normal;\n"
                               " gl_Position = /*gl_ModelViewProjectionMatrix*/ gl_Vertex; \n"
                               "}\n"
};
static const char* tessControlSource = {
"#version 400\n"
"#extension GL_EXT_geometry_shader4 : enable\n" "\n"\
"layout(vertices = 3) out;\n"
//"in vec3 vPosition[];\n"
"out vec3 tcPosition[];\n"
"out vec3 patchOrigin[];\n"
"uniform float TessLevelInner;\n"
"uniform float TessLevelOuter;\n"
"#define ID gl_InvocationID\n"
"#define POS(XXX) vec4(gl_in[(XXX)].gl_Position.xy,-1+2.0*round((gl_in[(XXX)].gl_Position.z*0.5+0.5)*128.0)/128.0,1)\n"
"#define NBCUTZ(XXXX) (abs((XXXX))*TessLevelOuter*0.5)\n"
//"#define POS(XXX) gl_in[(XXX)].gl_Position\n"
"void main(){\n"
"    tcPosition[ID] = POS(ID).xyz;\n"
"    if (ID==0) {\n"
"float m=0;\n"
"vec3 mi=POS(0).xyz;\n"
"if(mi.z>POS(1).z)mi=POS(1).xyz;\n"
"if(mi.z>POS(2).z)mi=POS(2).xyz;\n"
"patchOrigin[ID]=mi;\n"
"float mo=10000000;\n"
"#define LEVELOUT(l) (1.0+round(l))\n"
" float l=NBCUTZ(POS(0).z-POS(2).z) ;\n"
"if(l<mo){mo=l;patchOrigin[ID]=POS(1).xyz;}\n"
"gl_TessLevelOuter[1]=LEVELOUT(l);m=max(m,l);\n"
"  l=NBCUTZ(POS(1).z-POS(2).z) ;\n"
"if(l<mo){mo=l;patchOrigin[ID]=POS(0).xyz;}\n"
"gl_TessLevelOuter[0]=LEVELOUT(l);m=max(m,l);\n"
"  l=NBCUTZ(POS(0).z-POS(1).z) ;\n"
"if(l<mo){mo=l;patchOrigin[ID]=POS(2).xyz;}\n"
"gl_TessLevelOuter[2]=LEVELOUT(l);m=max(m,l);\n"
"        gl_TessLevelInner[0] =0;\n"

"    }\n"
"}\n"
};
static const char* tessEvalSource = {
"#version 400\n"
"#extension GL_EXT_geometry_shader4 : enable\n" "\n"\
"layout(triangles, equal_spacing, cw) in;\n"
"in vec3 tcPosition[];\n"
"in vec3 patchOrigin[];\n"
"out vec3 tePosition;\n"
"out vec3 tePatchOrigin;\n"
"out vec3 tePatchDistance;\n"
"uniform mat4 osg_ProjectionMatrix;\n"
"uniform mat4 osg_ModelViewMatrix;\n"
"void main(){\n"
"    vec3 p0 = gl_TessCoord.x * tcPosition[0];\n"
"    vec3 p1 = gl_TessCoord.y * tcPosition[1];\n"
"    vec3 p2 = gl_TessCoord.z * tcPosition[2];\n"
"    tePatchDistance = gl_TessCoord;\n"
"    tePatchOrigin=patchOrigin[0];\n"
"    tePosition =  (p0 + p1 + p2);\n"
"    gl_Position = vec4(tePosition, 1);\n"
"}\n"
};

static const char* geomSource = {
"#version 400\n"
"uniform mat4 osg_ProjectionMatrix;\n"
"uniform mat4 osg_ModelViewMatrix;\n"
"uniform mat3 osg_NormalMatrix;\n"
"layout(triangles) in;\n"
"layout(triangle_strip, max_vertices = 6) out;\n"
"uniform float TessLevelOuter;\n"
"in vec3 tePosition[3];\n"
"in vec3 tePatchOrigin[3];\n"
"in vec3 tePatchDistance[3];\n"
"out vec3 gFacetNormal;\n"
"out vec3 gPatchDistance;\n"
"out vec3 gTriDistance;\n"
"out vec4 gColor;\n"
"void main(){\n"
"    vec3 A = tePosition[2] - tePosition[0];\n"
"    vec3 B = tePosition[1] - tePosition[0];\n"
"    gFacetNormal = osg_NormalMatrix * normalize(cross(A, B));\n"
"    gPatchDistance = tePatchDistance[0];\n"
"    gTriDistance = vec3(1, 0, 0);\n"
"    gColor = osg_ModelViewMatrix[0];\n"
//"    gl_Position = osg_ProjectionMatrix * osg_ModelViewMatrix * vec4(gl_in[0].gl_Position.xy, gl_in[1].gl_Position.z,1); EmitVertex();\n"
//"   A=tePosition[2] - tePosition[1]; gl_Position =osg_ProjectionMatrix * osg_ModelViewMatrix *vec4(tePosition[1]+dot((A),(-B))*A,1); EmitVertex();\n"
 
" vec3 BA=gl_in[0].gl_Position.xyz-tePatchOrigin[0].xyz;\n"
" vec3 BC=gl_in[2].gl_Position.xyz-tePatchOrigin[0].xyz;\n"
" vec3 CA=gl_in[1].gl_Position.xyz-tePatchOrigin[0].xyz;\n"

"if(BA.z>0.00001||BA.z<-0.00001) BA*=BC.z/BA.z;else BA=vec3(0);\n"
//"BA=(dot((BA),(BC)))*BA;\n"
"A=tePatchOrigin[0].xyz+BA;//A=tePatchOrigin[0];\n"


///ensure at least one pixel projeted
"vec4 Bout=gl_in[1].gl_Position;vec4 Cout=gl_in[2].gl_Position;\n"
#if 0
"float rdx=1.0/128.0;\n"
"for(int i=0;i<0;i++){\n"
"if(A[i]-gl_in[1].gl_Position[i]>0){\n"
"if(A[i]-gl_in[1].gl_Position[i]<rdx)Bout[i]=A[i]-rdx;\n"
"}else{\n"
"if(A[i]-gl_in[1].gl_Position[i]>-rdx)Bout[i]=A[i]+rdx;\n"
"}\n"

"if(A[i]-gl_in[2].gl_Position[i]>0){\n"
"if(A[i]-gl_in[2].gl_Position[i]<rdx)Cout[i]=A[i]-rdx;\n"
"}else{\n"
"if(A[i]-gl_in[2].gl_Position[i]>-rdx)Cout[i]=A[i]+rdx;\n"
"}}\n"
#endif

"gl_Layer=int(floor(TessLevelOuter*((A.z+1)*0.5)));\n"
"   gl_Position =  osg_ProjectionMatrix * osg_ModelViewMatrix *vec4(A,1); EmitVertex();\n"

"    gPatchDistance = tePatchDistance[1];\n"
"    gTriDistance = vec3(0, 1, 0);\n"
"    gColor = osg_ModelViewMatrix[1];\n"
"    gl_Position =  osg_ProjectionMatrix * osg_ModelViewMatrix *Bout; EmitVertex();\n"
"    gPatchDistance = tePatchDistance[2];\n"
"    gTriDistance = vec3(0, 0, 1);\n"
"    gColor = osg_ModelViewMatrix[2];\n"
"    gl_Position =  osg_ProjectionMatrix * osg_ModelViewMatrix *Cout; EmitVertex();\n"
"    EndPrimitive();\n"
///second triangel
 #if 0
"//gl_Layer=int(round(128*(max(max(A.z,Bout.z),Cout.z)*0.5+0.5)));\n"
"  BA=gl_in[0].gl_Position.xyz-tePatchOrigin[0].xyz;\n"
"  BC=gl_in[2].gl_Position.xyz-tePatchOrigin[0].xyz;\n"
"  CA=gl_in[1].gl_Position.xyz-tePatchOrigin[0].xyz;\n"
 "  gPatchDistance = tePatchDistance[0];\n"
"    gTriDistance = vec3(1, 0, 0);\n"
"    gColor = osg_ModelViewMatrix[0];\n"
"if(BA.z!=0) BA*=CA.z/BA.z;\n"

"gl_Layer=int(floor(TessLevelOuter*((tePatchOrigin[0].z+BA.z)*0.5+0.5)));\n"
//"BA=(dot((BA),(BC)))*BA;\n"
"   gl_Position =osg_ProjectionMatrix * osg_ModelViewMatrix *vec4(A,1); EmitVertex();\n"
"A=tePatchOrigin[0].xyz+BA;//A=tePatchOrigin[0];\n"
 "  gPatchDistance = tePatchDistance[1];\n"
"    gTriDistance = vec3(0, 1, 0);\n"
"    gColor = osg_ModelViewMatrix[0];\n"
"   gl_Position =osg_ProjectionMatrix * osg_ModelViewMatrix *vec4(A,1); EmitVertex();\n"


"    gPatchDistance = tePatchDistance[2];\n"
"    gTriDistance = vec3(0, 0, 1);\n"
"    gColor = osg_ModelViewMatrix[2];\n"
"    gl_Position = osg_ProjectionMatrix * osg_ModelViewMatrix * Bout; EmitVertex();\n"
"    EndPrimitive();\n"
#endif
"}\n"
};
static const char* geom2voxSource = {
"#version 400\n"
"uniform mat4 osg_ProjectionMatrix;\n"
"uniform mat4 osg_ModelViewMatrix;\n"
"uniform mat3 osg_NormalMatrix;\n"
"layout(triangles) in;\n"
"layout(triangle_strip, max_vertices = 6) out;\n"
"in vec3 tePosition[3];\n"
"in vec3 tePatchOrigin[3];\n"
"in vec3 tePatchDistance[3];\n"
"out vec3 gFacetNormal;\n"
"out vec3 gPatchDistance;\n"
"out vec3 gTriDistance;\n"
"out vec4 gColor;\n"
"void main(){\n"
"    vec3 A = tePosition[2] - tePosition[0];\n"
"    vec3 B = tePosition[1] - tePosition[0];\n"
"    gFacetNormal = osg_NormalMatrix * normalize(cross(A, B));\n"
"    gPatchDistance = tePatchDistance[0];\n"
"    gTriDistance = vec3(1, 0, 0);\n"
"    gColor = osg_ModelViewMatrix[0];\n"
//"    gl_Position = osg_ProjectionMatrix * osg_ModelViewMatrix * vec4(gl_in[0].gl_Position.xy, gl_in[1].gl_Position.z,1); EmitVertex();\n"
//"   A=tePosition[2] - tePosition[1]; gl_Position =osg_ProjectionMatrix * osg_ModelViewMatrix *vec4(tePosition[1]+dot((A),(-B))*A,1); EmitVertex();\n"
 
" vec3 BA=gl_in[0].gl_Position.xyz-tePatchOrigin[0].xyz;\n"
" vec3 BC=gl_in[2].gl_Position.xyz-tePatchOrigin[0].xyz;\n"
" vec3 CA=gl_in[1].gl_Position.xyz-tePatchOrigin[0].xyz;\n"

"if(BA.z!=0) BA*=BC.z/BA.z;//else BA=vec3(0);\n"
//"BA=(dot((BA),(BC)))*BA;\n"
"A=tePatchOrigin[0].xyz+BA;//A=tePatchOrigin[0];\n"


///ensure at least one pixel projeted
"vec4 Bout=gl_in[1].gl_Position;vec4 Cout=gl_in[2].gl_Position;\n"
"float rdx=2.0/128.0;\n"
"for(int i=0;i<3;i++){\n"
"if(A[i]-gl_in[1].gl_Position[i]>0){\n"
"if(A[i]-gl_in[1].gl_Position[i]<rdx)Bout[i]=A[i]-rdx;\n"
"}else{\n"
"if(A[i]-gl_in[1].gl_Position[i]>-rdx)Bout[i]=A[i]+rdx;\n"
"}\n"

"if(A[i]-gl_in[2].gl_Position[i]>0){\n"
"if(A[i]-gl_in[2].gl_Position[i]<rdx)Cout[i]=A[i]-rdx;\n"
"}else{\n"
"if(A[i]-gl_in[2].gl_Position[i]>-rdx)Cout[i]=A[i]+rdx;\n"
"}}\n"


"gl_Layer=int(round(128.0*((A.z+1)*0.5)));\n"
"   gl_Position =  vec4(A,1); EmitVertex();\n"

"    gPatchDistance = tePatchDistance[1];\n"
"    gTriDistance = vec3(0, 1, 0);\n"
"    gColor = osg_ModelViewMatrix[1];\n"
"    gl_Position =  Bout; EmitVertex();\n"
"    gPatchDistance = tePatchDistance[2];\n"
"    gTriDistance = vec3(0, 0, 1);\n"
"    gColor = osg_ModelViewMatrix[2];\n"
"    gl_Position =  Cout; EmitVertex();\n"
"    EndPrimitive();\n"
///second triangel
 
"//gl_Layer=int(round(128.0*(max(max(A.z,Bout.z),Cout.z)*0.5+0.5)));\n"
"  BA=gl_in[0].gl_Position.xyz-tePatchOrigin[0].xyz;\n"
"  BC=gl_in[2].gl_Position.xyz-tePatchOrigin[0].xyz;\n"
"  CA=gl_in[1].gl_Position.xyz-tePatchOrigin[0].xyz;\n"
 "  gPatchDistance = tePatchDistance[0];\n"
"    gTriDistance = vec3(1, 0, 0);\n"
"    gColor = osg_ModelViewMatrix[0];\n"
"if(BA.z!=0) BA*=CA.z/BA.z;\n"

//"gl_Layer=int(floor(128.0*((tePatchOrigin[0].z+BA.z)*0.5+0.5)));\n"
//"BA=(dot((BA),(BC)))*BA;\n"
"   gl_Position =vec4(A,1); EmitVertex();\n"
"A=tePatchOrigin[0].xyz+BA;//A=tePatchOrigin[0];\n"
 "  gPatchDistance = tePatchDistance[1];\n"
"    gTriDistance = vec3(0, 1, 0);\n"
"    gColor = osg_ModelViewMatrix[0];\n"
"   gl_Position =vec4(A,1); EmitVertex();\n"


"    gPatchDistance = tePatchDistance[2];\n"
"    gTriDistance = vec3(0, 0, 1);\n"
"    gColor = osg_ModelViewMatrix[2];\n"
"    gl_Position =  Bout; EmitVertex();\n"
"    EndPrimitive();\n"
"}\n"
};
static const char* fragSource = {
"#version 400\n"
"out vec4 FragColor;\n"
"in vec3 gFacetNormal;\n"
"in vec3 gTriDistance;\n"
"in vec3 gPatchDistance;\n"
"in vec4 gColor;\n"
"in float gPrimitive;\n"
"uniform vec3 LightPosition;\n"
"uniform vec3 DiffuseMaterial;\n"
"uniform vec3 AmbientMaterial;\n"
"float amplify(float d, float scale, float offset){\n"
"    d = scale * d + offset;\n"
"    d = clamp(d, 0, 1);\n"
"    d = 1 - exp2(-2*d*d);\n"
"    return d;\n"
"}\n"
"void main(){\n"
"    vec3 N = normalize(gFacetNormal);\n"
"    vec3 L = LightPosition;\n"
"    float df = abs(dot(N, L));\n"
"    vec3 color = AmbientMaterial + df * DiffuseMaterial;\n"
"    float d1 = min(min(gTriDistance.x, gTriDistance.y), gTriDistance.z);\n"
"    float d2 = min(min(gPatchDistance.x, gPatchDistance.y), gPatchDistance.z);\n"
"    color = gTriDistance;//amplify(d1, 40, -0.5) * amplify(d2, 60, -0.5) * color;\n"
"    FragColor = vec4(color,1.0);\n"
"}\n"
};
class PatchModePrimSetVisitor :public osg::NodeVisitor
{ 
public: 
    PatchModePrimSetVisitor() :osg::NodeVisitor(TRAVERSE_ALL_CHILDREN){


}
 void apply(osg::Geode &n)
    {
        for(int i=0; i<n.getNumDrawables(); i++){
            osg::Geometry*geom=dynamic_cast<osg::Geometry*>( n.getDrawable(i));
if(geom){
for(int j=0;j<geom->getNumPrimitiveSets();j++)
geom->getPrimitiveSet(j)->setMode(GL_PATCHES);

}
}
}
};
osg::ref_ptr<osg::Geode> CreateIcosahedron( )
{
    osg::Geode *geode=new osg::Geode();
    osg::Geometry *geometry = new osg::Geometry();
    const unsigned int Faces[] = {
        2, 1, 0,
        3, 2, 0,
        4, 3, 0,
        5, 4, 0,
        1, 5, 0,

        11, 6,  7,
        11, 7,  8,
        11, 8,  9,
        11, 9,  10,
        11, 10, 6,

        1, 2, 6,
        2, 3, 7,
        3, 4, 8,
        4, 5, 9,
        5, 1, 10,

        2,  7, 6,
        3,  8, 7,
        4,  9, 8,
        5, 10, 9,
        1, 6, 10 };
    int IndexCount = sizeof(Faces) / sizeof(Faces[0]);
    const float Verts[] = {
         0.000f,  0.000f,  1.000f,
         0.894f,  0.000f,  0.447f,
         0.276f,  0.851f,  0.447f,
        -0.724f,  0.526f,  0.447f,
        -0.724f, -0.526f,  0.447f,
         0.276f, -0.851f,  0.447f,
         0.724f,  0.526f, -0.447f,
        -0.276f,  0.851f, -0.447f,
        -0.894f,  0.000f, -0.447f,
        -0.276f, -0.851f, -0.447f,
         0.724f, -0.526f, -0.447f,
         0.000f,  0.000f, -1.000f };

    int VertexCount = sizeof(Verts)/sizeof(float);
    osg::Vec3Array* vertices = new osg::Vec3Array();
    for(int i=0;i<VertexCount;i+=3){
        vertices->push_back(osg::Vec3(Verts[i],Verts[i+1],Verts[i+2]));
    }
///MYSTUFF
#if 1
#define LGDIM sqrt(5.0f)
float t=0.5f*(1.0f+LGDIM);
float u=1.0f;///sqrt(1.0f+t*t);
t*=u;
vertices = new osg::Vec3Array();
 
vertices->push_back(osg::Vec3f(-u,  t,  0));
vertices->push_back(osg::Vec3f( u,  t,  0));
vertices->push_back(osg::Vec3f(-u, -t,  0));
vertices->push_back(osg::Vec3f( u, -t,  0));

vertices->push_back(osg::Vec3f( 0, -u,  t));
vertices->push_back(osg::Vec3f( 0,  u,  t));
vertices->push_back(osg::Vec3f( 0, -u, -t));
vertices->push_back(osg::Vec3f( 0,  u, -t));

vertices->push_back(osg::Vec3f( t,  0, -u));
vertices->push_back(osg::Vec3f( t,  0,  u));
vertices->push_back(osg::Vec3f(-t,  0, -u));
vertices->push_back(osg::Vec3f(-t,  0,  u));
// 5 faces around point 0
 const unsigned int faces[] = {
0, 11, 5,
0, 5, 1,
0, 1, 7,
0, 7, 10,
0, 10, 11,

1, 5, 9,
5, 11, 4,
11, 10, 2,
10, 7, 6,
7, 1, 8,

3, 9, 4,
3, 4, 2,
3, 2, 6,
3, 6, 8,
3, 8, 9,

4, 9, 5,
2, 4, 11,
6, 2, 10,
8, 6, 7,
9, 8, 1};

geometry->setVertexArray(vertices);
   osg::DrawElementsUInt * pr=new osg::DrawElementsUInt(osg::PrimitiveSet::PATCHES);
for(int i=0;i<20;i++){
pr->push_back(faces[i*3+0]);
pr->push_back(faces[i*3+1]);
pr->push_back(faces[i*3+2]);
}
    geometry->addPrimitiveSet(pr);
#else
geometry->setVertexArray(vertices);
    geometry->addPrimitiveSet(new osg::DrawElementsUInt(osg::PrimitiveSet::PATCHES,IndexCount,Faces));
  // Expand the bounding box, otherwise the geometry is clipped in front when tessellating.
    osg::BoundingBox bbox(osg::Vec3(-1.0f, -1.0f, -1.0f), osg::Vec3(1.0f, 1.0f, 1.0f));
    geometry->setInitialBound(bbox);

#endif

    
  
    geode->addDrawable(geometry);
    return geode;
}

osg::ref_ptr<osg::Program> createProgram(bool vox)
{
    osg::Program *program = new osg::Program();
    program->addShader(new osg::Shader(osg::Shader::VERTEX,vertSource));
    program->addShader(new osg::Shader(osg::Shader::TESSCONTROL,tessControlSource));
    program->addShader(new osg::Shader(osg::Shader::TESSEVALUATION,tessEvalSource));
    if(!vox)program->addShader(new osg::Shader(osg::Shader::GEOMETRY,geomSource));
else    program->addShader(new osg::Shader(osg::Shader::GEOMETRY,geom2voxSource));
    program->addShader(new osg::Shader(osg::Shader::FRAGMENT,fragSource));
    program->setParameter(GL_GEOMETRY_VERTICES_OUT_EXT, 6);
    program->setParameter(GL_GEOMETRY_INPUT_TYPE_EXT, GL_TRIANGLES);
    program->setParameter(GL_GEOMETRY_OUTPUT_TYPE_EXT, GL_TRIANGLE_STRIP);
    return program;
}

class KeyboardEventHandler : public osgGA::GUIEventHandler
{
public:
    KeyboardEventHandler(osg::ref_ptr<osg::Uniform> tessInnerU, osg::ref_ptr<osg::Uniform> tessOuterU,osg::Texture3D*tex=0,osg::Camera*cam=0, osgVolume::VolumeTile * layer=0):
        _tessInnerU(tessInnerU),
        _tessOuterU(tessOuterU),_tex(tex),_cam(cam),_tile(layer)

    {
        tessInnerU->get(_tessInner);
        tessOuterU->get(_tessOuter);
    }

    virtual bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& gaa)
    {
        if(ea.getEventType()==osgGA::GUIEventAdapter::KEYDOWN){
            switch (ea.getKey()){
                case osgGA::GUIEventAdapter::KEY_Up:
                    increaseOuterTesselation();
                    return true;
                case osgGA::GUIEventAdapter::KEY_Down:
                    decreaseOuterTesselation();
                    return true;
                case osgGA::GUIEventAdapter::KEY_Left:
                    decreaseInnerTesselation();
                    return true;
                case osgGA::GUIEventAdapter::KEY_Right:
                    increaseInnerTesselation();
                    return true;
                case osgGA::GUIEventAdapter::KEY_Plus:
                case osgGA::GUIEventAdapter::KEY_KP_Add:
                    increaseInnerTesselation();
                    increaseOuterTesselation();
                    return true;
                case osgGA::GUIEventAdapter::KEY_Minus:
                case osgGA::GUIEventAdapter::KEY_KP_Subtract:
                    decreaseInnerTesselation();
                    decreaseOuterTesselation();
                    return true;
            }
        }
        return osgGA::GUIEventHandler::handle(ea, gaa);
    }

private:
    osg::ref_ptr<osg::Uniform> _tessInnerU;
    osg::ref_ptr<osg::Uniform> _tessOuterU;
osg::Texture3D* _tex;osg::Camera*_cam; osgVolume::VolumeTile *_tile;
    float _tessInner;
    float _tessOuter;

    void increaseInnerTesselation()
    {
        _tessInnerU->set(++_tessInner);
    }

    void decreaseInnerTesselation()
    {
        _tessInner = std::max(1.0f, _tessInner-1.0f);
        _tessInnerU->set(_tessInner);
    }

    void increaseOuterTesselation()
    {
        _tessOuterU->set(++_tessOuter);
std::cout<<_tessOuter<<std::endl;
    }

    void decreaseOuterTesselation()
    {
        _tessOuter = std::max(1.0f, _tessOuter-1.0f);
        _tessOuterU->set(_tessOuter);
if(_cam){
_cam->addChild(new ImageReadBackNode(_tex));
osg::ref_ptr<osgVolume::ImageLayer> layer=new osgVolume::ImageLayer;
layer->setImage(_tex->getImage());
_tile->setLayer(layer);
//_tile->dirty();
 
std::cout<<_tessOuter<<std::endl;
    }
}
};

int main(int argc, char* argv[])
{
 osg::ArgumentParser arguments(&argc,argv);
    arguments.getApplicationUsage()->setApplicationName(arguments.getApplicationName());
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is the standard OpenSceneGraph example for opengl tessalateion.");

 osgViewer::Viewer viewer(arguments);
    viewer.setUpViewInWindow(100,100,800,600);

bool render2vox=arguments.read("--rtt");
    osg::ref_ptr<osg::Program> program = createProgram(render2vox);
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readRefNodeFiles(arguments);
osg::ref_ptr<osg::Node> geode ;
    if (!loadedModel)
    {
        geode = CreateIcosahedron( );
    }else 
    {
	geode=loadedModel;
	PatchModePrimSetVisitor nv;
	geode->accept(nv);
   }
osg::Vec3ui _resolution(128,128,128); 
if(render2vox){

osg::Camera *cam=new osg::Camera();
osg::Texture3D *_voxels=new osg::Texture3D();
_voxels->setTextureSize(_resolution.x(), _resolution.y(), _resolution.z());
 cam->setClearMask( GL_COLOR_BUFFER_BIT);
    //	setClearMask( GL_COLOR_BUFFER_BIT);
	 cam->setClearColor(osg::Vec4(0.0f,0,0,0.0f));
 cam->    setComputeNearFarMode(osg::Camera::DO_NOT_COMPUTE_NEAR_FAR);

    // set the camera to render before the main camera.
	 cam->setRenderOrder(osg::Camera::PRE_RENDER, -160);
    // tell the camera to use OpenGL frame buffer object where supported.
 cam->    setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
//setReadBuffer(osg::Camera::COLOR_BUFFER);
	 cam->setProjectionResizePolicy(osg::Camera::FIXED);
 cam->    setReferenceFrame(osg::Transform::ABSOLUTE_RF);
 cam->    setViewMatrix(osg::Matrix::identity());
 cam->    setImplicitBufferAttachmentMask(osg::Camera::IMPLICIT_COLOR_BUFFER_ATTACHMENT, osg::Camera::IMPLICIT_COLOR_BUFFER_ATTACHMENT);
     cam->setAllowEventFocus(false);
     cam->setDrawBuffer(osg::Camera::COLOR_BUFFER0);
     cam->setViewMatrix(osg::Matrix::identity());
     cam->setViewport(0, 0, _resolution.x(), _resolution.y());
	 cam->getOrCreateStateSet()->setAttributeAndModes(new osg::Viewport(0, 0, _resolution.x(), _resolution.y()), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE); 
cam->getOrCreateStateSet()->setTextureAttributeAndModes(0,_voxels, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
     
    ///DEBUG
    	float *data = new float[(_resolution.x()* _resolution.y()* _resolution.z()) * 4];
    	memset(data, 0, sizeof(float)*(_resolution.x()* _resolution.y()* _resolution.z()) * 4);
    	osg::ref_ptr<osg::Image >im = new osg::Image();
    	im->setImage(_resolution.x(), _resolution.y(), _resolution.z(),
    		//	GL_RGBA8,
    		//high accuracy but vary costfull
    		GL_RGBA16F_ARB,
    		GL_RGBA,
    		GL_FLOAT, (unsigned char *)data, osg::Image::USE_NEW_DELETE);
		im->dirty();
	_voxels->setImage(im);

    ///ENDDEBUG

    //  osg::StateSet * ss = getOrCreateStateSet();
     
  //  _voxels->dirtyTextureObject();
   cam->setProjectionMatrix(osg::Matrix::ortho2D(-1, 1, -1, 1));
 //	setProjectionMatrix(osg::Matrix::ortho2D(0, _resolution.x(), 0, _resolution.y()));

	cam->attach(osg::Camera::COLOR_BUFFER0, _voxels, 0, osg::Camera::FACE_CONTROLLED_BY_GEOMETRY_SHADER, true);
cam->addChild(geode);

im->setFileName("outvox.ive");
cam->addChild(new ImageReadBackNode(_voxels));
//  CONSERVATIVE_RASTERIZATION_NV                   0x9346
cam->getOrCreateStateSet()->setMode( 0x9346,osg::StateAttribute::ON);

 osg::ref_ptr<osg::Uniform> tessInnerU = new osg::Uniform("TessLevelInner", 1.0f);
    osg::ref_ptr<osg::Uniform> tessOuterU = new osg::Uniform("TessLevelOuter", float(_resolution.z()));

    osg::StateSet *state;
    state = geode->getOrCreateStateSet();
    state->addUniform(new osg::Uniform("AmbientMaterial",osg::Vec3(0.04f, 0.04f, 0.04f)));
    state->addUniform(new osg::Uniform("DiffuseMaterial",osg::Vec3(0.0f, 0.75f, 0.75f)));
    state->addUniform(new osg::Uniform("LightPosition",osg::Vec3(0.25f, 0.25f, 1.0f)));
state->setAttributeAndModes(new osg::LineWidth(10), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
state->setMode(GL_POLYGON_SMOOTH, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
state->setMode(GL_MULTISAMPLE, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
    state->addUniform(tessInnerU.get());
    state->addUniform(tessOuterU.get());
    state->setAttribute(new osg::PatchParameter(3));
    state->setAttribute(program.get());
osg::ref_ptr<osgVolume::ImageLayer> layer = new osgVolume::ImageLayer(im);
osg::ref_ptr< osgVolume::VolumeTile> tile =new osgVolume::VolumeTile;
tile->setLayer(layer);
   viewer.addEventHandler(new KeyboardEventHandler(tessInnerU, tessOuterU,_voxels,cam,tile));
    viewer.addEventHandler(new osgViewer::StatsHandler);
 
if(im.get())mainosgVolume(arguments,tile,cam,viewer);return 1;
geode=cam;
}   
    osg::ref_ptr<osg::Uniform> tessInnerU = new osg::Uniform("TessLevelInner", 1.0f);
    osg::ref_ptr<osg::Uniform> tessOuterU = new osg::Uniform("TessLevelOuter",float(_resolution.z()));

    osg::StateSet *state;
    state = geode->getOrCreateStateSet();
    state->addUniform(new osg::Uniform("AmbientMaterial",osg::Vec3(0.04f, 0.04f, 0.04f)));
    state->addUniform(new osg::Uniform("DiffuseMaterial",osg::Vec3(0.0f, 0.75f, 0.75f)));
    state->addUniform(new osg::Uniform("LightPosition",osg::Vec3(0.25f, 0.25f, 1.0f)));
    state->addUniform(tessInnerU.get());
    state->addUniform(tessOuterU.get());
    state->setAttribute(new osg::PatchParameter(3));
    state->setAttribute(program.get());

viewer.setThreadingModel(osgViewer::Viewer::SingleThreaded);
    // switch on the uniforms that track the modelview and projection matrices
    osgViewer::Viewer::Windows windows;
    viewer.getWindows(windows);
    for(osgViewer::Viewer::Windows::iterator itr = windows.begin();
        itr != windows.end();
        ++itr)
    {
        osg::State *s=(*itr)->getState();
        s->setUseModelViewAndProjectionUniforms(true);
    //    s->setUseVertexAttributeAliasing(true);
    }

    viewer.addEventHandler(new KeyboardEventHandler(tessInnerU, tessOuterU));
    viewer.addEventHandler(new osgViewer::StatsHandler);
    viewer.setSceneData(geode.get());
    return viewer.run();
}

