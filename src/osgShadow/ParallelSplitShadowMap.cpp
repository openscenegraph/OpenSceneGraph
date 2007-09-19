/* OpenSceneGraph example, osgshadow.
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
 
/* ParallelSplitShadowMap written by Adrian Egli */
 
#include <osgShadow/ParallelSplitShadowMap>
#include <osgShadow/ShadowedScene>
#include <osg/Notify>
#include <osg/ComputeBoundsVisitor>
#include <osg/PolygonOffset>
#include <osg/CullFace>
#include <osg/io_utils>
#include <iostream>
#include <sstream>
#include <osg/Geode>
#include <osg/Geometry>
#include <osgDB/ReadFile>
#include <osg/Texture1D>

using namespace osgShadow;

// split scheme
unsigned int NUM_SPLITS    = 1;
#define TEXTURE_RESOLUTION  1024 
//#define ADAPTIVE_TEXTURE_RESOLUTION


#define LINEAR_SPLIT false


#define ZNEAR_MIN_FROM_LIGHT_SOURCE 1.0
 
#define SHOW_SHADOW_TEXTURE_DEBUG    // DEPTH instead of color for debug information texture display in a rectangle


//#define SHADOW_TEXTURE_DEBUG         // COLOR instead of DEPTH

#ifndef SHADOW_TEXTURE_DEBUG
    #define SHADOW_TEXTURE_GLSL 
    
    //#define SHADOW_TEXTURE_GLSL_DEBUG

#endif


std::string generateGLSL_FragmentShader_BaseTex(unsigned int splitCount) {
    std::stringstream sstr;
    
    /// base texture
    sstr << "uniform sampler2D baseTexture; "      << std::endl;                
    sstr << "uniform float enableBaseTexture; "     << std::endl;        

    for (unsigned int i=0;i<splitCount;i++)    {
        sstr << "uniform sampler2DShadow shadowTexture"    <<    i    <<"; "    << std::endl;                                    
        sstr << "uniform vec2 ambientBias"                <<    i    <<"; "    << std::endl;                                            
        sstr << "uniform float zShadow"                    <<    i    <<"; "    << std::endl;                            
    }

    sstr << "void main(void)"    << std::endl;                                                            
    sstr << "{"    << std::endl;                                                                        
    for (unsigned int i=0;i<splitCount;i++)    {
        sstr << "    float shadow"    <<    i    <<" = (ambientBias"    <<    i    <<".x+ambientBias"    <<    i    <<".y*(shadow2DProj( shadowTexture"    <<    i    <<",gl_TexCoord["    <<    (i+1)    <<"]).r));"    << std::endl; 
    }

    sstr << "    float v = 1.0-step(";
    sstr << "(";
    unsigned int sum_i = 0;
    for (unsigned int i=0;i<splitCount;i++)    {
        sstr << "step(shadow"    <<    i    <<",0.5)*("<<1.1*(double)(splitCount-i)<<")";
        if ( i+1 < splitCount ){
            sstr << "+";
        }
        sum_i +=(i+1);
    }
    sstr << ")/"<< 1.01*(double)sum_i     << std::endl;    
    sstr << ",0.0);"    << std::endl;    
    #ifdef SHADOW_TEXTURE_GLSL_DEBUG 
        sstr << "    vec4 color = gl_Color*0.5 + vec4(step(shadow0,0.5),step(shadow1,0.5),step(shadow2,0.5),1.0)*0.5; "<< std::endl;                                                     
        sstr << "    vec4 texcolor = vec4(1,1,1,1); "    << std::endl;                                            
    #else
        sstr << "    vec4 color = gl_Color; "<< std::endl;                                                     
        sstr << "    vec4 texcolor = texture2D(baseTexture,gl_TexCoord[0].st); "    << std::endl;                                            
    #endif
 
    sstr << "    float enableBaseTextureFilter = enableBaseTexture*(1.0 - step(texcolor.x+texcolor.y+texcolor.z+texcolor.a,0.0)); "    << std::endl;                                                //18
    sstr << "    vec4  colorTex = color*texcolor;" << std::endl;    
    sstr << "    gl_FragColor = (color*(1.0-enableBaseTextureFilter) + colorTex*enableBaseTextureFilter)*(1.0-0.30*v); "<< std::endl;          
    sstr << "}"<< std::endl;
    //std::cout << sstr.str() << std::endl;
    return sstr.str();
}
 

// clamp variables of any type
template<class Type> inline Type Clamp(Type A, Type Min, Type Max) {
    if(A<Min) return Min;
    if(A>Max) return Max;
    return A;
}

#define min(a,b)            (((a) < (b)) ? (a) : (b))
#define max(a,b)            (((a) > (b)) ? (a) : (b))


ParallelSplitShadowMap::ParallelSplitShadowMap(osg::Geode** gr, int icountplanes){
    _displayTexturesGroupingNode = gr;
    NUM_SPLITS = icountplanes;
    _textureUnitOffset = 1;
}

ParallelSplitShadowMap::ParallelSplitShadowMap(const ParallelSplitShadowMap& copy, const osg::CopyOp& copyop):
    ShadowTechnique(copy,copyop),
    _textureUnitOffset(copy._textureUnitOffset)
{
}


void ParallelSplitShadowMap::init(){
    if (!_shadowedScene) return;
           
    osg::StateSet* sharedStateSet = new osg::StateSet;
    
    unsigned int iCamerasMax=NUM_SPLITS;
    for (unsigned int iCameras=0;iCameras<iCamerasMax;iCameras++)
    {
        PSSMShadowSplitTexture pssmShadowSplitTexture;
        pssmShadowSplitTexture._splitID = iCameras;
        pssmShadowSplitTexture._textureUnit = iCameras+_textureUnitOffset;
        //pssmShadowSplitTexture._ambientBias = osg::Vec2(0.9-(double)iCameras/10.0,0.1+(double)iCameras/10.0);
        pssmShadowSplitTexture._ambientBias = osg::Vec2(0.0,1.0);

        pssmShadowSplitTexture._resolution =  
                #ifdef ADAPTIVE_TEXTURE_RESOLUTION
                        TEXTURE_RESOLUTION    / pow(2,iCameras);
                #else 
                        TEXTURE_RESOLUTION    ;
                #endif
        pssmShadowSplitTexture._resolution = max(pssmShadowSplitTexture._resolution ,128);
        std::cout << "Texture ID=" << iCameras << " Resolution=" << pssmShadowSplitTexture._resolution << std::endl;
        // set up the texture to render into
        {
            pssmShadowSplitTexture._texture = new osg::Texture2D;
            pssmShadowSplitTexture._texture->setTextureSize(pssmShadowSplitTexture._resolution, pssmShadowSplitTexture._resolution);
            #ifndef SHADOW_TEXTURE_DEBUG
                pssmShadowSplitTexture._texture->setInternalFormat(GL_DEPTH_COMPONENT);
                pssmShadowSplitTexture._texture->setShadowComparison(true);
                pssmShadowSplitTexture._texture->setShadowTextureMode(osg::Texture2D::LUMINANCE);
            #else
                pssmShadowSplitTexture._texture->setInternalFormat(GL_RGBA);
            #endif
            pssmShadowSplitTexture._texture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
            pssmShadowSplitTexture._texture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);
            pssmShadowSplitTexture._texture->setBorderColor(osg::Vec4d(1.0,1.0,1.0,1.0));
            pssmShadowSplitTexture._texture->setWrap(osg::Texture2D::WRAP_S, osg::Texture2D::CLAMP_TO_BORDER);
            pssmShadowSplitTexture._texture->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::CLAMP_TO_BORDER);
        }
        // set up the render to texture camera.
        {
            // create the camera
            pssmShadowSplitTexture._camera = new osg::Camera;

            pssmShadowSplitTexture._camera->setCullCallback(new CameraCullCallback(this));
    
            #ifndef SHADOW_TEXTURE_DEBUG
                pssmShadowSplitTexture._camera->setClearMask(GL_DEPTH_BUFFER_BIT);
                pssmShadowSplitTexture._camera->setClearColor(osg::Vec4(1.0,1.0,1.0,1.0));
            #else
                pssmShadowSplitTexture._camera->setClearMask(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
                switch(iCameras)
                {
                case 0:
                    pssmShadowSplitTexture._camera->setClearColor(osg::Vec4(1.0,0.0,0.0,1.0));
                    break;
                case 1:
                    pssmShadowSplitTexture._camera->setClearColor(osg::Vec4(0.0,1.0,0.0,1.0));
                    break;
                case 2:
                    pssmShadowSplitTexture._camera->setClearColor(osg::Vec4(0.0,0.0,1.0,1.0));
                    break;
                default:
                    pssmShadowSplitTexture._camera->setClearColor(osg::Vec4(1.0,1.0,1.0,1.0));
                    break;
                }
            #endif        
             pssmShadowSplitTexture._camera->setComputeNearFarMode(osg::Camera::DO_NOT_COMPUTE_NEAR_FAR);

            // set viewport
            pssmShadowSplitTexture._camera->setViewport(0,0,pssmShadowSplitTexture._resolution,pssmShadowSplitTexture._resolution);

            // set the camera to render before the main camera.
            pssmShadowSplitTexture._camera->setRenderOrder(osg::Camera::PRE_RENDER);

            // tell the camera to use OpenGL frame buffer object where supported.
            pssmShadowSplitTexture._camera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
 
            // attach the texture and use it as the color buffer.
            #ifndef SHADOW_TEXTURE_DEBUG
                pssmShadowSplitTexture._camera->attach(osg::Camera::DEPTH_BUFFER, pssmShadowSplitTexture._texture.get());
            #else
                pssmShadowSplitTexture._camera->attach(osg::Camera::COLOR_BUFFER, pssmShadowSplitTexture._texture.get());
            #endif
            osg::StateSet* stateset = pssmShadowSplitTexture._camera->getOrCreateStateSet();
            if ( 1 ) {
                float factor = -0.001;// * (1.0+pssmShadowSplitTexture._splitID);
                float units  = 0.5 * (1.0+0.1*pssmShadowSplitTexture._splitID);

                osg::ref_ptr<osg::PolygonOffset> polygon_offset = new osg::PolygonOffset;
                polygon_offset->setFactor(factor);
                polygon_offset->setUnits(units);
                stateset->setAttribute(polygon_offset.get(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
                stateset->setMode(GL_POLYGON_OFFSET_FILL, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
            }
        }        
        // set up stateset and append texture, texGen ,...
        {
            pssmShadowSplitTexture._stateset = sharedStateSet;//new osg::StateSet; 
            pssmShadowSplitTexture._stateset->setTextureAttributeAndModes(pssmShadowSplitTexture._textureUnit,pssmShadowSplitTexture._texture.get(),osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
            pssmShadowSplitTexture._stateset->setTextureMode(pssmShadowSplitTexture._textureUnit,GL_TEXTURE_GEN_S,osg::StateAttribute::ON);
            pssmShadowSplitTexture._stateset->setTextureMode(pssmShadowSplitTexture._textureUnit,GL_TEXTURE_GEN_T,osg::StateAttribute::ON);
            pssmShadowSplitTexture._stateset->setTextureMode(pssmShadowSplitTexture._textureUnit,GL_TEXTURE_GEN_R,osg::StateAttribute::ON);
            pssmShadowSplitTexture._stateset->setTextureMode(pssmShadowSplitTexture._textureUnit,GL_TEXTURE_GEN_Q,osg::StateAttribute::ON);


            /// generate a TexGen object
            pssmShadowSplitTexture._texgen = new osg::TexGen;
 
        }
        
        // set up shader (GLSL)
        #ifdef SHADOW_TEXTURE_GLSL

            osg::Program* program = new osg::Program;
            pssmShadowSplitTexture._stateset->setAttribute(program);

            //osg::Shader* fragment_shader = new osg::Shader(osg::Shader::FRAGMENT, fragmentShaderSource_BaseTex);
            osg::Shader* fragment_shader = new osg::Shader(osg::Shader::FRAGMENT, generateGLSL_FragmentShader_BaseTex(iCamerasMax).c_str());
            program->addShader(fragment_shader);

            std::stringstream strST; strST << "shadowTexture" << (pssmShadowSplitTexture._textureUnit-1);
            osg::Uniform* shadowTextureSampler = new osg::Uniform(strST.str().c_str(),(int)(pssmShadowSplitTexture._textureUnit));
            pssmShadowSplitTexture._stateset->addUniform(shadowTextureSampler);

            std::stringstream strAB; strAB << "ambientBias" << (pssmShadowSplitTexture._textureUnit-1);
            osg::Uniform* ambientBias = new osg::Uniform(strAB.str().c_str(),pssmShadowSplitTexture._ambientBias);
            pssmShadowSplitTexture._stateset->addUniform(ambientBias);


            std::stringstream strzShadow; strzShadow << "zShadow" << (pssmShadowSplitTexture._textureUnit-1);
            pssmShadowSplitTexture._farDistanceSplit = new osg::Uniform(strzShadow.str().c_str(),(float)1.0);
            pssmShadowSplitTexture._stateset->addUniform(pssmShadowSplitTexture._farDistanceSplit);

            osg::Uniform* baseTextureSampler = new osg::Uniform("baseTexture",0);
            pssmShadowSplitTexture._stateset->addUniform(baseTextureSampler);

            if ( _textureUnitOffset > 0 ) {
                osg::Uniform* enableBaseTexture = new osg::Uniform("enableBaseTexture",1.0f);
                pssmShadowSplitTexture._stateset->addUniform(enableBaseTexture);
            } else {
                osg::Uniform* enableBaseTexture = new osg::Uniform("enableBaseTexture",0.0f);
                pssmShadowSplitTexture._stateset->addUniform(enableBaseTexture);
            }


            // fake texture for baseTexture
            osg::Image* image = new osg::Image;
            // allocate the image data, noPixels x 1 x 1 with 4 rgba floats - equivilant to a Vec4!
            int noPixels = 1;
            image->allocateImage(noPixels,1,1,GL_RGBA,GL_FLOAT);
            image->setInternalTextureFormat(GL_RGBA);
            // fill in the image data.    
            osg::Vec4* dataPtr = (osg::Vec4*)image->data();
            osg::Vec4 color(0,0,0,0);
            *dataPtr = color;
            // make fake texture 
            osg::Texture2D* texture = new osg::Texture2D;
            texture->setWrap(osg::Texture2D::WRAP_S,osg::Texture2D::CLAMP_TO_BORDER);
            texture->setWrap(osg::Texture2D::WRAP_T,osg::Texture2D::CLAMP_TO_BORDER);
            texture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
            texture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);
            texture->setImage(image);
            // add fake texture
            pssmShadowSplitTexture._stateset->setTextureAttribute(0,texture,osg::StateAttribute::ON);
            pssmShadowSplitTexture._stateset->setTextureMode(0,GL_TEXTURE_1D,osg::StateAttribute::OFF);
            pssmShadowSplitTexture._stateset->setTextureMode(0,GL_TEXTURE_2D,osg::StateAttribute::ON);
            pssmShadowSplitTexture._stateset->setTextureMode(0,GL_TEXTURE_3D,osg::StateAttribute::OFF);


        #endif
        

        //////////////////////////////////////////////////////////////////////////
        // DEBUG
        if ( _displayTexturesGroupingNode ) {
            {
                pssmShadowSplitTexture._debug_textureUnit = 1;
                pssmShadowSplitTexture._debug_texture = new osg::Texture2D;
                pssmShadowSplitTexture._debug_texture->setTextureSize(TEXTURE_RESOLUTION, TEXTURE_RESOLUTION);
                #ifdef SHOW_SHADOW_TEXTURE_DEBUG
                    pssmShadowSplitTexture._debug_texture->setInternalFormat(GL_DEPTH_COMPONENT);
                    pssmShadowSplitTexture._debug_texture->setShadowTextureMode(osg::Texture2D::LUMINANCE);
                #else
                    pssmShadowSplitTexture._debug_texture->setInternalFormat(GL_RGBA);
                #endif
                pssmShadowSplitTexture._debug_texture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
                pssmShadowSplitTexture._debug_texture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);
                // create the camera
                pssmShadowSplitTexture._debug_camera = new osg::Camera;
                pssmShadowSplitTexture._debug_camera->setCullCallback(new CameraCullCallback(this));
                pssmShadowSplitTexture._debug_camera->setClearMask(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
                pssmShadowSplitTexture._debug_camera->setClearColor(osg::Vec4(1.0,1.0,1.0,1.0));
                pssmShadowSplitTexture._debug_camera->setComputeNearFarMode(osg::Camera::DO_NOT_COMPUTE_NEAR_FAR);

                // set viewport
                pssmShadowSplitTexture._debug_camera->setViewport(0,0,TEXTURE_RESOLUTION,TEXTURE_RESOLUTION);
                // set the camera to render before the main camera.
                pssmShadowSplitTexture._debug_camera->setRenderOrder(osg::Camera::PRE_RENDER);
                // tell the camera to use OpenGL frame buffer object where supported.
                pssmShadowSplitTexture._debug_camera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
                   // attach the texture and use it as the color buffer.
                #ifdef SHOW_SHADOW_TEXTURE_DEBUG
                    pssmShadowSplitTexture._debug_camera->attach(osg::Camera::DEPTH_BUFFER, pssmShadowSplitTexture._debug_texture.get());
                #else
                    pssmShadowSplitTexture._debug_camera->attach(osg::Camera::COLOR_BUFFER, pssmShadowSplitTexture._debug_texture.get());
                #endif
                osg::StateSet* stateset = pssmShadowSplitTexture._debug_camera->getOrCreateStateSet();

                pssmShadowSplitTexture._debug_stateset = new osg::StateSet;        
                pssmShadowSplitTexture._debug_stateset->setTextureAttributeAndModes(pssmShadowSplitTexture._debug_textureUnit,pssmShadowSplitTexture._debug_texture.get(),osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
                pssmShadowSplitTexture._debug_stateset->setTextureMode(pssmShadowSplitTexture._debug_textureUnit,GL_TEXTURE_GEN_S,osg::StateAttribute::ON);
                pssmShadowSplitTexture._debug_stateset->setTextureMode(pssmShadowSplitTexture._debug_textureUnit,GL_TEXTURE_GEN_T,osg::StateAttribute::ON);
                pssmShadowSplitTexture._debug_stateset->setTextureMode(pssmShadowSplitTexture._debug_textureUnit,GL_TEXTURE_GEN_R,osg::StateAttribute::ON);
                pssmShadowSplitTexture._debug_stateset->setTextureMode(pssmShadowSplitTexture._debug_textureUnit,GL_TEXTURE_GEN_Q,osg::StateAttribute::ON);
             }

            osg::Geode* geode = _displayTexturesGroupingNode[iCameras];
            geode->getOrCreateStateSet()->setTextureAttributeAndModes(0,pssmShadowSplitTexture._debug_texture.get(),osg::StateAttribute::ON);

        }        
        //////////////////////////////////////////////////////////////////////////

        _PSSMShadowSplitTextureMap.insert(PSSMShadowSplitTextureMap::value_type(iCameras,pssmShadowSplitTexture));


    }


    _dirty = false;
}

void ParallelSplitShadowMap::update(osg::NodeVisitor& nv){
     getShadowedScene()->osg::Group::traverse(nv);
}

void ParallelSplitShadowMap::cull(osgUtil::CullVisitor& cv){
    // record the traversal mask on entry so we can reapply it later.
    unsigned int traversalMask = cv.getTraversalMask();
    osgUtil::RenderStage* orig_rs = cv.getRenderStage();

    // do traversal of shadow recieving scene which does need to be decorated by the shadow map
    for (PSSMShadowSplitTextureMap::iterator it=_PSSMShadowSplitTextureMap.begin();it!=_PSSMShadowSplitTextureMap.end();it++)
    {
        PSSMShadowSplitTexture pssmShadowSplitTexture = it->second;
        cv.pushStateSet(pssmShadowSplitTexture._stateset.get());
        
        //////////////////////////////////////////////////////////////////////////
        // DEGUBG
        if ( _displayTexturesGroupingNode ) {
            cv.pushStateSet(pssmShadowSplitTexture._debug_stateset.get());
        }
        //////////////////////////////////////////////////////////////////////////

        _shadowedScene->osg::Group::traverse(cv);

        cv.popStateSet();

    }

    // need to compute view frustum for RTT camera.
    // get the bounds of the model.    
    osg::ComputeBoundsVisitor cbbv(osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN);
    cbbv.setTraversalMask(getShadowedScene()->getCastsShadowTraversalMask());
    _shadowedScene->osg::Group::traverse(cbbv);
    osg::BoundingBox bb = cbbv.getBoundingBox();
 
    const osg::Light* selectLight = 0;
    osg::Vec4 lightpos;
    osg::Vec3 lightDirection;

    osgUtil::PositionalStateContainer::AttrMatrixList& aml = orig_rs->getPositionalStateContainer()->getAttrMatrixList();
    for(osgUtil::PositionalStateContainer::AttrMatrixList::iterator itr = aml.begin();
        itr != aml.end();
        ++itr)
    {
        const osg::Light* light = dynamic_cast<const osg::Light*>(itr->first.get());
        if (light)
        {
            osg::RefMatrix* matrix = itr->second.get();
            if (matrix) lightpos = light->getPosition() * (*matrix);
            else lightpos = light->getPosition();
            if (matrix) lightDirection = light->getDirection() * (*matrix);
            else lightDirection = light->getDirection();

            selectLight = light;
        }
    }

    osg::Matrix eyeToWorld;
    eyeToWorld.invert(*cv.getModelViewMatrix());

    lightpos = lightpos * eyeToWorld;
    lightDirection = lightDirection * eyeToWorld;
                  
     if (selectLight)
    {
 
        // do traversal of shadow recieving scene which does need to be decorated by the shadow map
        unsigned int iMaxSplit = _PSSMShadowSplitTextureMap.size();
        
        for (PSSMShadowSplitTextureMap::iterator it=_PSSMShadowSplitTextureMap.begin();it!=_PSSMShadowSplitTextureMap.end();it++)
        {
            PSSMShadowSplitTexture pssmShadowSplitTexture = it->second;

            //////////////////////////////////////////////////////////////////////////
            // SETUP pssmShadowSplitTexture for rendering 
            // 
            lightDirection.normalize();
            pssmShadowSplitTexture._lightDirection = lightDirection;
            pssmShadowSplitTexture._cameraView    = cv.getRenderInfo().getView()->getCamera()->getViewMatrix();
            pssmShadowSplitTexture._cameraProj    = cv.getRenderInfo().getView()->getCamera()->getProjectionMatrix();

             //////////////////////////////////////////////////////////////////////////
            // CALCULATE

 
 
            // Calculate corner points of frustum split
            //
            // To avoid edge problems, scale the frustum so
            // that it's at least a few pixels larger
            //
            osg::Vec3d pCorners[8];
            calculateFrustumCorners(pssmShadowSplitTexture,pCorners);

            // Init Light (Directional Light)
            //
            calculateLightInitalPosition(pssmShadowSplitTexture,pCorners);

            // Calculate near and far for light view
            //
            calculateLightNearFarFormFrustum(pssmShadowSplitTexture,pCorners);
            
            // Calculate view and projection matrices
            //
            calculateLightViewProjectionFormFrustum(pssmShadowSplitTexture,pCorners);

            //////////////////////////////////////////////////////////////////////////
            // set up shadow rendering camera
            pssmShadowSplitTexture._camera->setReferenceFrame(osg::Camera::ABSOLUTE_RF); 

            //////////////////////////////////////////////////////////////////////////
            // DEBUG
            if ( _displayTexturesGroupingNode ) {
                pssmShadowSplitTexture._debug_camera->setViewMatrix(pssmShadowSplitTexture._camera->getViewMatrix());
                pssmShadowSplitTexture._debug_camera->setProjectionMatrix(pssmShadowSplitTexture._camera->getProjectionMatrix());
                pssmShadowSplitTexture._debug_camera->setReferenceFrame(osg::Camera::ABSOLUTE_RF); 
            }

            //////////////////////////////////////////////////////////////////////////
             // compute the matrix which takes a vertex from local coords into tex coords
            // will use this later to specify osg::TexGen..

             osg::Matrix MVPT = pssmShadowSplitTexture._camera->getViewMatrix() * 
                pssmShadowSplitTexture._camera->getProjectionMatrix() *
                osg::Matrix::translate(1.0,1.0,1.0) * 
                osg::Matrix::scale(0.5,0.5,0.5);

            pssmShadowSplitTexture._texgen->setMode(osg::TexGen::EYE_LINEAR);
            pssmShadowSplitTexture._texgen->setPlanesFromMatrix(MVPT);
            //////////////////////////////////////////////////////////////////////////


            //////////////////////////////////////////////////////////////////////////
            cv.setTraversalMask( traversalMask & getShadowedScene()->getCastsShadowTraversalMask() );

            // do RTT camera traversal
            pssmShadowSplitTexture._camera->accept(cv);
            
            //////////////////////////////////////////////////////////////////////////
            // DEBUG
            if ( _displayTexturesGroupingNode ) {
                pssmShadowSplitTexture._debug_camera->accept(cv);
            }

            
            orig_rs->getPositionalStateContainer()->addPositionedTextureAttribute(pssmShadowSplitTexture._textureUnit, cv.getModelViewMatrix(), pssmShadowSplitTexture._texgen.get());


        }
    } // if light



    // reapply the original traversal mask
    cv.setTraversalMask( traversalMask );
}

void ParallelSplitShadowMap::cleanSceneGraph(){

}



// Computes corner points of a frustum
//
//
//unit box representing frustum in clip space
const osg::Vec3f const_pointFarBR(1.0f, -1.0f, 1.0f);
const osg::Vec3f const_pointNearBR(1.0f, -1.0f, -1.0f);
const osg::Vec3f const_pointNearTR(1.0f, 1.0f, -1.0f);    
const osg::Vec3f const_pointFarTR(1.0f, 1.0f, 1.0f);
const osg::Vec3f const_pointFarTL(-1.0f, 1.0f, 1.0f);
const osg::Vec3f const_pointFarBL(-1.0f, -1.0f, 1.0f);
const osg::Vec3f const_pointNearBL(-1.0f, -1.0f, -1.0f);
const osg::Vec3f const_pointNearTL(-1.0f, 1.0f, -1.0f);
//

void ParallelSplitShadowMap::calculateFrustumCorners(
    PSSMShadowSplitTexture &pssmShadowSplitTexture,
    osg::Vec3d *frustumCorners
) {
    double fovy,aspectRatio,camNear,camFar;
    pssmShadowSplitTexture._cameraProj.getPerspective(fovy,aspectRatio,camNear,camFar);

    //////////////////////////////////////////////////////////////////////////
    /// CALCULATE SPLIT 
    double maxFar = camFar;
    double camNearFar_Dist = maxFar - camNear;
    bool linear = LINEAR_SPLIT; 
    if ( linear ) {
         camFar  = camNear + (camNearFar_Dist) * ((double)(pssmShadowSplitTexture._splitID+1))/((double)(NUM_SPLITS));
        camNear = camNear + (camNearFar_Dist) * ((double)(pssmShadowSplitTexture._splitID))/((double)(NUM_SPLITS));
    } else {

        // Exponential split scheme:
        //
        // Ci = (n - f)*(i/numsplits)^(bias+1) + n;
        //
        static float fSplitSchemeBias[2]={0.25f,0.66f};
        fSplitSchemeBias[1]=Clamp(fSplitSchemeBias[1],0.0f,3.0f);
        float* pSplitDistances =new float[NUM_SPLITS+1];

        for(int i=0;i<(int)NUM_SPLITS;i++) {
            float fIDM=i/(float)NUM_SPLITS;
            pSplitDistances[i]=(camFar-camNear)*(powf(fIDM,fSplitSchemeBias[1]+1))+camNear;
        }
        // make sure border values are right
        pSplitDistances[0]=camNear;
        pSplitDistances[NUM_SPLITS]=camFar;
 
        camNear = pSplitDistances[pssmShadowSplitTexture._splitID];
        camFar  = pSplitDistances[pssmShadowSplitTexture._splitID+1];
        
        delete[] pSplitDistances;

    }



    #ifdef SHADOW_TEXTURE_GLSL
        pssmShadowSplitTexture._farDistanceSplit->set((float)((maxFar-camNear)/camNearFar_Dist));
    #endif

    //////////////////////////////////////////////////////////////////////////
    /// TRANSFORM frustum corners (Optimized for Orthogonal)
     osg::Matrix projMat;
     projMat.makePerspective(fovy,aspectRatio,camNear,camFar);
 
    osg::Matrix projViewMat = pssmShadowSplitTexture._cameraView*projMat;
    osg::Matrix invProjViewMat;
    invProjViewMat.invert(projViewMat);

    //transform frustum vertices to world space
    frustumCorners[0] = const_pointFarBR * invProjViewMat;    
    frustumCorners[1] = const_pointNearBR* invProjViewMat;    
    frustumCorners[2] = const_pointNearTR* invProjViewMat;    
    frustumCorners[3] = const_pointFarTR * invProjViewMat;    
    frustumCorners[4] = const_pointFarTL * invProjViewMat;    
    frustumCorners[5] = const_pointFarBL * invProjViewMat;
    frustumCorners[6] = const_pointNearBL* invProjViewMat;
    frustumCorners[7] = const_pointNearTL* invProjViewMat;
 
}
 
//////////////////////////////////////////////////////////////////////////
//
// compute directional light inital postion;
void ParallelSplitShadowMap::calculateLightInitalPosition(PSSMShadowSplitTexture &pssmShadowSplitTexture,osg::Vec3d *frustumCorners){
     pssmShadowSplitTexture._frustumSplitCenter = frustumCorners[0];
    for(int i=1;i<8;i++) {
        pssmShadowSplitTexture._frustumSplitCenter +=frustumCorners[i];
    }
    pssmShadowSplitTexture._frustumSplitCenter /= 8.0;
    pssmShadowSplitTexture._lightCameraSource = pssmShadowSplitTexture._frustumSplitCenter;
}

void ParallelSplitShadowMap::calculateLightNearFarFormFrustum(
    PSSMShadowSplitTexture &pssmShadowSplitTexture,
    osg::Vec3d *frustumCorners
) {
 
    //calculate near, far
    double zNear=-1;
    double zFar =-1;


 

    // force zNear > 0.0 
    // set 2.0m distance to the nearest point
    int count = 0;
    while (zNear <= ZNEAR_MIN_FROM_LIGHT_SOURCE && count++ < 10) {
        zNear= DBL_MAX;
        zFar =-DBL_MAX;
        for(int i=0;i<8;i++) {
            double dist_z_from_light = pssmShadowSplitTexture._lightDirection*(frustumCorners[i] - pssmShadowSplitTexture._lightCameraSource);
            if ( zNear > dist_z_from_light ) zNear = dist_z_from_light;
            if ( zFar  < dist_z_from_light ) zFar  = dist_z_from_light;
         }
         
        if ( zNear <= ZNEAR_MIN_FROM_LIGHT_SOURCE ){
            osg::Vec3 dUpdate = - pssmShadowSplitTexture._lightDirection*(fabs(zNear)+ZNEAR_MIN_FROM_LIGHT_SOURCE);
            pssmShadowSplitTexture._lightCameraSource = pssmShadowSplitTexture._lightCameraSource + dUpdate;
        } 

    }


    pssmShadowSplitTexture._lightCameraTarget = pssmShadowSplitTexture._lightCameraSource + pssmShadowSplitTexture._lightDirection*zFar; 
    pssmShadowSplitTexture._lightNear = zNear;
    pssmShadowSplitTexture._lightFar  = zFar;
}

void ParallelSplitShadowMap::calculateLightViewProjectionFormFrustum(PSSMShadowSplitTexture &pssmShadowSplitTexture,osg::Vec3d *frustumCorners) {
    //////////////////////////////////////////////////////////////////////////

    // light dir
    osg::Vec3d lightDir = pssmShadowSplitTexture._lightDirection;

    osg::Vec3d up(0,1,0);
    osg::Vec3d left;
    osg::Vec3d top;
 
    left = up ^ lightDir;
    top = lightDir ^ left;
 

     double maxLeft,maxTop;
    double minLeft,minTop;
    
    osg::Vec3d fCenter = pssmShadowSplitTexture._frustumSplitCenter;
    
    maxLeft = maxTop = -DBL_MAX;
    minLeft = minTop = DBL_MAX;
    for(int i = 0; i < 8; i++)
    {
        osg::Vec3d diffCorner = fCenter - frustumCorners[i];
         double lLeft = (diffCorner*left) * 1.5; // scale, removes edges problem, faster for calculation
        double lTop  = (diffCorner*top)  * 1.5; // scale, removes edges problem, faster for calculation

        if ( lLeft > maxLeft ) maxLeft  =  lLeft;
        if ( lTop  > maxTop  ) maxTop   =  lTop ;

        if ( lLeft < minLeft ) minLeft  =  lLeft;
        if ( lTop  < minTop  ) minTop   =  lTop ;
     }    
 
    osg::Matrixd lightView;
    lightView.makeLookAt(pssmShadowSplitTexture._lightCameraSource,pssmShadowSplitTexture._lightCameraTarget,up);
    osg::Matrixd lightProj;
 
    double zNear = pssmShadowSplitTexture._lightNear;
    double zFar  = pssmShadowSplitTexture._lightFar;

    lightProj.makeOrtho(minLeft,maxLeft,minTop,maxTop,zNear,zFar);

    pssmShadowSplitTexture._camera->setViewMatrix(lightView);
    pssmShadowSplitTexture._camera->setProjectionMatrix(lightProj);

    
 
}


 





