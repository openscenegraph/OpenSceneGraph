/* OpenSceneGraph example, osgvertexprogram.
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
*  THE SOFTWARE.
*/

#include <osg/Vec3>
#include <osg/Vec4>
#include <osg/Quat>
#include <osg/Matrix>
#include <osg/ShapeDrawable>
#include <osg/Geometry>
#include <osg/Geode>
#include <osg/Transform>
#include <osg/Material>
#include <osg/Depth>
#include <osg/CullFace>
#include <osg/TexMat>
#include <osg/TexGen>
#include <osg/TexEnv>
#include <osg/TexEnvCombine>
#include <osg/TextureCubeMap>
#include <osg/VertexProgram>

#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <osgUtil/SmoothingVisitor>
#include <osgUtil/Optimizer>

#include <osgViewer/Viewer>

#include <iostream>


float refract = 1.02;          // ratio of indicies of refraction
float fresnel = 0.2;           // Fresnel multiplier


const char vpstr[] =
    "!!ARBvp1.0 # Refraction                                    \n"
    "                                                           \n"
    "ATTRIB iPos         = vertex.position;                     \n"
    "#ATTRIB iCol        = vertex.color.primary;                \n"
    "ATTRIB iNormal      = vertex.normal;                       \n"
    "PARAM  esEyePos     = { 0, 0, 0, 1 };                      \n"
    "PARAM  const0123    = { 0, 1, 2, 3 };                      \n"
    "PARAM  fresnel      = program.local[0];                    \n"
    "PARAM  refract      = program.local[1];                    \n"
    "PARAM  itMV[4]      = { state.matrix.modelview.invtrans }; \n"
    "PARAM  MVP[4]       = { state.matrix.mvp };                \n"
    "PARAM  MV[4]        = { state.matrix.modelview };          \n"
    "PARAM  texmat[4]    = { state.matrix.texture[0] };         \n"
    "TEMP   esPos;        # position in eye-space               \n"
    "TEMP   esNormal;     # normal in eye-space                 \n"
    "TEMP   tmp, IdotN, K;                                      \n"
    "TEMP   esE;          # eye vector                          \n"
    "TEMP   esI;          # incident vector (=-E)               \n"
    "TEMP   esR;          # first refract- then reflect-vector  \n"
    "OUTPUT oPos         = result.position;                     \n"
    "OUTPUT oColor       = result.color;                        \n"
    "OUTPUT oRefractMap  = result.texcoord[0];                  \n"
    "OUTPUT oReflectMap  = result.texcoord[1];                  \n"
    "                                                           \n"
    "# transform vertex to clip space                           \n"
    "DP4    oPos.x, MVP[0], iPos;                               \n"
    "DP4    oPos.y, MVP[1], iPos;                               \n"
    "DP4    oPos.z, MVP[2], iPos;                               \n"
    "DP4    oPos.w, MVP[3], iPos;                               \n"
    "                                                           \n"
    "# Transform the normal to eye space.                       \n"
    "DP3    esNormal.x, itMV[0], iNormal;                       \n"
    "DP3    esNormal.y, itMV[1], iNormal;                       \n"
    "DP3    esNormal.z, itMV[2], iNormal;                       \n"
    "                                                           \n"
    "# normalize normal                                         \n"
    "DP3    esNormal.w, esNormal, esNormal;                     \n"
    "RSQ    esNormal.w, esNormal.w;                             \n"
    "MUL    esNormal, esNormal, esNormal.w;                     \n"
    "                                                           \n"
    "# transform vertex position to eye space                   \n"
    "DP4    esPos.x, MV[0], iPos;                               \n"
    "DP4    esPos.y, MV[1], iPos;                               \n"
    "DP4    esPos.z, MV[2], iPos;                               \n"
    "DP4    esPos.w, MV[3], iPos;                               \n"
    "                                                           \n"
    "# vertex to eye vector                                     \n"
    "ADD    esE, -esPos, esEyePos;                              \n"
    "#MOV   esE, -esPos;                                        \n"
    "                                                           \n"
    "# normalize eye vector                                     \n"
    "DP3    esE.w, esE, esE;                                    \n"
    "RSQ    esE.w, esE.w;                                       \n"
    "MUL    esE, esE, esE.w;                                    \n"
    "                                                           \n"
    "# calculate some handy values                              \n"
    "MOV    esI, -esE;                                          \n"
    "DP3    IdotN, esNormal, esI;                               \n"
    "                                                           \n"
    "# calculate refraction vector, Renderman style             \n"
    "                                                           \n"
    "# k = 1-index*index*(1-(I dot N)^2)                        \n"
    "MAD    tmp, -IdotN, IdotN, const0123.y;                    \n"
    "MUL    tmp, tmp, refract.y;                                \n"
    "ADD    K.x, const0123.y, -tmp;                             \n"
    "                                                           \n"
    "# k<0,  R = [0,0,0]                                        \n"
    "# k>=0, R = index*I-(index*(I dot N) + sqrt(k))*N          \n"
    "RSQ    K.y, K.x;                                           \n"
    "RCP    K.y, K.y;                           # K.y = sqrt(k) \n"
    "MAD    tmp.x, refract.x, IdotN, K.y;                       \n"
    "MUL    tmp, esNormal, tmp.x;                               \n"
    "MAD    esR, refract.x, esI, tmp;                           \n"
    "                                                           \n"
    "# transform refracted ray by cubemap transform             \n"
    "DP3    oRefractMap.x, texmat[0], esR;                      \n"
    "DP3    oRefractMap.y, texmat[1], esR;                      \n"
    "DP3    oRefractMap.z, texmat[2], esR;                      \n"
    "                                                           \n"
    "# calculate reflection vector                              \n"
    "# R = 2*N*(N dot E)-E                                      \n"
    "MUL    tmp, esNormal, const0123.z;                         \n"
    "DP3    esR.w, esNormal, esE;                               \n"
    "MAD    esR, esR.w, tmp, -esE;                              \n"
    "                                                           \n"
    "# transform reflected ray by cubemap transform             \n"
    "DP3    oReflectMap.x, texmat[0], esR;                      \n"
    "DP3    oReflectMap.y, texmat[1], esR;                      \n"
    "DP3    oReflectMap.z, texmat[2], esR;                      \n"
    "                                                           \n"
    "# Fresnel approximation = fresnel*(1-(N dot I))^2          \n"
    "ADD    tmp.x, const0123.y, -IdotN;                         \n"
    "MUL    tmp.x, tmp.x, tmp.x;                                \n"
    "MUL    oColor, tmp.x, fresnel;                             \n"
    "                                                           \n"
    "END                                                        \n";


osg::TextureCubeMap* readCubeMap()
{
    osg::TextureCubeMap* cubemap = new osg::TextureCubeMap;
    //#define CUBEMAP_FILENAME(face) "nvlobby_" #face ".png"
    //#define CUBEMAP_FILENAME(face) "Cubemap_axis/" #face ".png"
    #define CUBEMAP_FILENAME(face) "Cubemap_snow/" #face ".jpg"

    osg::Image* imagePosX = osgDB::readImageFile(CUBEMAP_FILENAME(posx));
    osg::Image* imageNegX = osgDB::readImageFile(CUBEMAP_FILENAME(negx));
    osg::Image* imagePosY = osgDB::readImageFile(CUBEMAP_FILENAME(posy));
    osg::Image* imageNegY = osgDB::readImageFile(CUBEMAP_FILENAME(negy));
    osg::Image* imagePosZ = osgDB::readImageFile(CUBEMAP_FILENAME(posz));
    osg::Image* imageNegZ = osgDB::readImageFile(CUBEMAP_FILENAME(negz));

    if (imagePosX && imageNegX && imagePosY && imageNegY && imagePosZ && imageNegZ)
    {
        cubemap->setImage(osg::TextureCubeMap::POSITIVE_X, imagePosX);
        cubemap->setImage(osg::TextureCubeMap::NEGATIVE_X, imageNegX);
        cubemap->setImage(osg::TextureCubeMap::POSITIVE_Y, imagePosY);
        cubemap->setImage(osg::TextureCubeMap::NEGATIVE_Y, imageNegY);
        cubemap->setImage(osg::TextureCubeMap::POSITIVE_Z, imagePosZ);
        cubemap->setImage(osg::TextureCubeMap::NEGATIVE_Z, imageNegZ);

        cubemap->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
        cubemap->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
        cubemap->setWrap(osg::Texture::WRAP_R, osg::Texture::CLAMP_TO_EDGE);

        cubemap->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR);
        cubemap->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
    }

    return cubemap;
}


// Update texture matrix for cubemaps
struct TexMatCallback : public osg::NodeCallback
{
public:

    TexMatCallback(osg::TexMat& tm) :
        _texMat(tm)
    {
    }

    virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
    {
        osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(nv);
        if (cv)
        {
            const osg::Matrix& MV = *(cv->getModelViewMatrix());
            const osg::Matrix R = osg::Matrix::rotate( osg::DegreesToRadians(112.0f), 0.0f,0.0f,1.0f)*
                                  osg::Matrix::rotate( osg::DegreesToRadians(90.0f), 1.0f,0.0f,0.0f);

            osg::Quat q = MV.getRotate();
            const osg::Matrix C = osg::Matrix::rotate( q.inverse() );

            _texMat.setMatrix( C*R );
        }

        traverse(node,nv);
    }

    osg::TexMat& _texMat;
};


class MoveEarthySkyWithEyePointTransform : public osg::Transform
{
public:
    /** Get the transformation matrix which moves from local coords to world coords.*/
    virtual bool computeLocalToWorldMatrix(osg::Matrix& matrix,osg::NodeVisitor* nv) const
    {
        osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(nv);
        if (cv)
        {
            osg::Vec3 eyePointLocal = cv->getEyeLocal();
            matrix.preMultTranslate(eyePointLocal);
        }
        return true;
    }

    /** Get the transformation matrix which moves from world coords to local coords.*/
    virtual bool computeWorldToLocalMatrix(osg::Matrix& matrix,osg::NodeVisitor* nv) const
    {
        osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(nv);
        if (cv)
        {
            osg::Vec3 eyePointLocal = cv->getEyeLocal();
            matrix.postMultTranslate(-eyePointLocal);
        }
        return true;
    }
};


osg::Node* createSkyBox()
{

    osg::StateSet* stateset = new osg::StateSet();

    osg::TexEnv* te = new osg::TexEnv;
    te->setMode(osg::TexEnv::REPLACE);
    stateset->setTextureAttributeAndModes(0, te, osg::StateAttribute::ON);

    osg::TexGen *tg = new osg::TexGen;
    tg->setMode(osg::TexGen::NORMAL_MAP);
    stateset->setTextureAttributeAndModes(0, tg, osg::StateAttribute::ON);

    osg::TexMat *tm = new osg::TexMat;
    stateset->setTextureAttribute(0, tm);

    osg::TextureCubeMap* skymap = readCubeMap();
    stateset->setTextureAttributeAndModes(0, skymap, osg::StateAttribute::ON);

    stateset->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
    stateset->setMode( GL_CULL_FACE, osg::StateAttribute::OFF );

    // clear the depth to the far plane.
    osg::Depth* depth = new osg::Depth;
    depth->setFunction(osg::Depth::ALWAYS);
    depth->setRange(1.0,1.0);
    stateset->setAttributeAndModes(depth, osg::StateAttribute::ON );

    stateset->setRenderBinDetails(-1,"RenderBin");

    osg::Drawable* drawable = new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(0.0f,0.0f,0.0f),1));

    osg::Geode* geode = new osg::Geode;
    geode->setCullingActive(false);
    geode->setStateSet( stateset );
    geode->addDrawable(drawable);


    osg::Transform* transform = new MoveEarthySkyWithEyePointTransform;
    transform->setCullingActive(false);
    transform->addChild(geode);

    osg::ClearNode* clearNode = new osg::ClearNode;
//  clearNode->setRequiresClear(false);
    clearNode->setCullCallback(new TexMatCallback(*tm));
    clearNode->addChild(transform);

    return clearNode;
}




osg::Node* addRefractStateSet(osg::Node* node)
{
    osg::StateSet* stateset = new osg::StateSet();

    osg::TextureCubeMap* reflectmap = readCubeMap();
    stateset->setTextureAttributeAndModes( 0, reflectmap, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );
    stateset->setTextureAttributeAndModes( 1, reflectmap, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );

    osg::TexMat* texMat = new osg::TexMat;
    stateset->setTextureAttribute(0, texMat);

    // ---------------------------------------------------
    // Vertex Program
    // ---------------------------------------------------
    osg::VertexProgram* vp = new osg::VertexProgram();
    vp->setVertexProgram( vpstr );
    vp->setProgramLocalParameter( 0, osg::Vec4( fresnel, fresnel, fresnel, 1.0f ) );
    vp->setProgramLocalParameter( 1, osg::Vec4( refract, refract*refract, 0.0f, 0.0f ) );
    stateset->setAttributeAndModes( vp, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );

    // ---------------------------------------------------
    // fragment = refraction*(1-fresnel) + reflection*fresnel
    // T0 = texture unit 0, refraction map
    // T1 = texture unit 1, reflection map
    // C.rgb = primary color, water color
    // C.a   = primary color, fresnel factor
    // Cp    = result from previous texture environment
    // ---------------------------------------------------

    // REPLACE function: Arg0
    // = T0
    osg::TexEnvCombine *te0 = new osg::TexEnvCombine;
    te0->setCombine_RGB(osg::TexEnvCombine::REPLACE);
    te0->setSource0_RGB(osg::TexEnvCombine::TEXTURE0);
    te0->setOperand0_RGB(osg::TexEnvCombine::SRC_COLOR);

    // INTERPOLATE function: Arg0 * (Arg2) + Arg1 * (1-Arg2)
    // = T1 * C0.a + Cp * (1-C0.a)
    osg::TexEnvCombine *te1 = new osg::TexEnvCombine;

    // rgb = Cp + Ct
    te1->setCombine_RGB(osg::TexEnvCombine::INTERPOLATE);
    te1->setSource0_RGB(osg::TexEnvCombine::TEXTURE1);
    te1->setOperand0_RGB(osg::TexEnvCombine::SRC_COLOR);
    te1->setSource1_RGB(osg::TexEnvCombine::PREVIOUS);
    te1->setOperand1_RGB(osg::TexEnvCombine::SRC_COLOR);
    te1->setSource2_RGB(osg::TexEnvCombine::PRIMARY_COLOR);
    te1->setOperand2_RGB(osg::TexEnvCombine::SRC_COLOR);

    stateset->setTextureAttributeAndModes(0, te0, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
    stateset->setTextureAttributeAndModes(1, te1, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

    osg::Group* group = new osg::Group;
    group->addChild(node);
    group->setCullCallback(new TexMatCallback(*texMat));
    group->setStateSet( stateset );

    return group;
}


int main(int argc, char *argv[])
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    // construct the viewer.
    osgViewer::Viewer viewer;

    osg::Group* rootnode = new osg::Group;

    rootnode->addChild(createSkyBox());

    // load the nodes from the commandline arguments.
    osg::Node* model = osgDB::readNodeFiles(arguments);
    if (!model)
    {
        const float radius = 1.0f;
        osg::Geode* geode = new osg::Geode;
        geode->addDrawable(new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(0.0f,0.0f,0.0f),radius)));
        model = geode;
    }

    // run optimization over the scene graph
    osgUtil::Optimizer optimzer;
    optimzer.optimize(model);

    // create normals.
    osgUtil::SmoothingVisitor smoother;
    model->accept(smoother);

    rootnode->addChild( addRefractStateSet(model) );

    // add a viewport to the viewer and attach the scene graph.
    viewer.setSceneData(rootnode);

    // create the windows and run the threads.
    viewer.realize();

    // now check to see if vertex program is supported.
    osgViewer::Viewer::Windows windows;
    viewer.getWindows(windows);
    for(osgViewer::Viewer::Windows::iterator itr = windows.begin();
        itr != windows.end();
        ++itr)
    {
        unsigned int contextID = (*itr)->getState()->getContextID();
        osg::GLExtensions* vpExt = osg::GLExtensions::Get(contextID,false);
        if (vpExt)
        {
            if (!vpExt->isVertexProgramSupported)
            {
                std::cout<<"Warning: ARB_vertex_program not supported by OpenGL drivers, unable to run application."<<std::endl;
                return 1;
            }
        }
    }

    return viewer.run();
}
