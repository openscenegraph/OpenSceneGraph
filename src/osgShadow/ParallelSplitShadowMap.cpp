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

/* ##################################################################################################### */
/* ParallelSplitShadowMap written by Adrian Egli (3dhelp (at) gmail.com)                                   */
/* ##################################################################################################### */
/*                                                                                                         */
/* the pssm main idea is based on:                                                                            */
/*                                                                                                         */
/* Parallel-Split Shadow Maps for Large-scale Virtual Environments                                         */
/*    Fan Zhang     Hanqiu Sun    Leilei Xu    Lee Kit Lun                                                 */
/*    The Chinese University of Hong Kong                                                                 */
/*                                                                                                         */
/* Refer to our latest project webpage for "Parallel-Split Shadow Maps on Programmable GPUs" in GPU Gems */
/*                                                                                                         */
/* ##################################################################################################### */

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
#include <osg/Depth>
#include <osg/ShadeModel>

using namespace osgShadow;

// split scheme
#define TEXTURE_RESOLUTION  1024


#define LINEAR_SPLIT false

//#define SMOOTH_SHADOW   //experimental

#define ZNEAR_MIN_FROM_LIGHT_SOURCE 2.0
#define MOVE_VIRTUAL_CAMERA_BEHIND_REAL_CAMERA_FACTOR 0.0

//#define SHOW_SHADOW_TEXTURE_DEBUG    // DEPTH instead of color for debug information texture display in a rectangle


//#define SHADOW_TEXTURE_DEBUG         // COLOR instead of DEPTH

#ifndef SHADOW_TEXTURE_DEBUG
    #define SHADOW_TEXTURE_GLSL
#endif
 

std::string ParallelSplitShadowMap::generateGLSL_FragmentShader_BaseTex(bool debug, unsigned int splitCount) {
    std::stringstream sstr;
  
    /// base texture
    sstr << "uniform sampler2D baseTexture; "      << std::endl;
    #ifdef SMOOTH_SHADOW
        sstr << "uniform sampler2D randomTexture; "  << std::endl;
    #endif
    sstr << "uniform float enableBaseTexture; "     << std::endl;
    sstr << "uniform vec2 ambientBias;"    << std::endl;

    for (unsigned int i=0;i<_number_of_splits;i++)    {
        sstr << "uniform sampler2DShadow shadowTexture"    <<    i    <<"; "    << std::endl;
        sstr << "uniform float zShadow"                    <<    i    <<"; "    << std::endl;
    }

    sstr << "void main(void)"    << std::endl;
    sstr << "{"    << std::endl;


 
    if ( debug ) {
        sstr << "    vec4 coord   = vec4(0,0,0,1);"<<std::endl;
    } else {
        sstr << "    vec4 coord   = gl_FragCoord;"<<std::endl;
    }

    #ifdef SMOOTH_SHADOW
        for (unsigned int i=0;i<_number_of_splits;i++)    {
            sstr << "    float shadow"    <<    i    <<" = shadow2DProj( shadowTexture"    <<    i    <<",gl_TexCoord["    <<    (i+_textureUnitOffset)    <<"]).r;"    << std::endl;
            sstr << "        vec4 random"    <<    i    <<" = "<<(1.15/(double)_resolution)<<"*coord.z*texture2D(randomTexture,gl_TexCoord["    <<    (i+_textureUnitOffset)    <<"].st); "    << std::endl;
            sstr << "        float shadow1"    <<    i    <<" = shadow2DProj( shadowTexture"    <<    i    <<",gl_TexCoord["    <<    (i+_textureUnitOffset)    <<"]+random"    <<    i    <<".r*vec4(-1,-1,0,0)).r;"    << std::endl;
            sstr << "        float shadow2"    <<    i    <<" = shadow2DProj( shadowTexture"    <<    i    <<",gl_TexCoord["    <<    (i+_textureUnitOffset)    <<"]+random"    <<    i    <<".g*vec4(1,-1,0,0)).r;"    << std::endl;
            sstr << "        float shadow3"    <<    i    <<" = shadow2DProj( shadowTexture"    <<    i    <<",gl_TexCoord["    <<    (i+_textureUnitOffset)    <<"]+random"    <<    i    <<".b*vec4(1,1,0,0)).r;"    << std::endl;
            sstr << "        float shadow4"    <<    i    <<" = shadow2DProj( shadowTexture"    <<    i    <<",gl_TexCoord["    <<    (i+_textureUnitOffset)    <<"]+random"    <<    i    <<".a*vec4(-1,1,0,0)).r;"    << std::endl;
            sstr << "        shadow"    <<    i    <<" = shadow"    <<    i    <<" + shadow1"    <<    i    <<" + shadow2"    <<    i    <<" + shadow3"    <<    i    <<" + shadow4"    <<    i    <<";"    << std::endl;
            sstr << "        shadow"    <<    i    <<" = shadow"    <<    i    <<"*0.2;"    << std::endl;
        }
    #else
        for (unsigned int i=0;i<_number_of_splits;i++)    {
            sstr << "    float shadow"    <<    i    <<" = shadow2DProj( shadowTexture"    <<    i    <<",gl_TexCoord["    <<    (i+_textureUnitOffset)    <<"]).r;"    << std::endl;
        }
    #endif

    sstr << "    float term0 = (1.0-shadow0); "    << std::endl;
    for (unsigned int i=1;i<_number_of_splits;i++)    {
        sstr << "    float term" << i << " = (1.0-shadow"<<i<<");"<< std::endl;   
    }

    // v => SHADOW filter; "    << std::endl;
    sstr << "    float v = clamp(";
    for (unsigned int i=0;i<_number_of_splits;i++)    {
        sstr << "term"    <<    i;
        if ( i+1 < _number_of_splits ){
            sstr << "+";
        }
    }
    sstr << ",0.0,1.0);"    << std::endl;

    if ( _debug_color_in_GLSL ) {


        sstr << "    float c0=0.0;" << std::endl;
        sstr << "    float c1=0.0;" << std::endl;
        sstr << "    float c2=0.0;" << std::endl;

        sstr << "    float sumTerm=0.0;" << std::endl;

        for (unsigned int i=0;i<_number_of_splits;i++)    {
            if ( i < 3 ) sstr << "    c" << i << "=term" << i << ";" << std::endl;
            sstr << "    sumTerm=sumTerm+term" << i << ";" << std::endl;
        }

        sstr << "    vec4 color    = gl_Color*( 1.0 - sumTerm ) + (sumTerm)* gl_Color*vec4(c0,(1.0-c0)*c1,(1.0-c0)*(1.0-c1)*c2,1.0); "    << std::endl;

    } else {
        sstr << "    vec4 color    = gl_Color; "<< std::endl;
    }


    sstr << "    vec4 texcolor = texture2D(baseTexture,gl_TexCoord[0].st); "    << std::endl;


    sstr << "    float enableBaseTextureFilter = enableBaseTexture*(1.0 - step(texcolor.x+texcolor.y+texcolor.z+texcolor.a,0.0)); "    << std::endl;                                                //18
    sstr << "    vec4 colorTex = color*texcolor;" << std::endl;
    sstr << "    gl_FragColor = ((color*(ambientBias.x+1.0)*(1.0-enableBaseTextureFilter)) + colorTex*(1.0+ambientBias.x)*enableBaseTextureFilter)*(1.0-ambientBias.y*v); "<< std::endl;


    sstr << "}"<< std::endl;


    if ( splitCount == _number_of_splits-1 )    osg::notify(osg::INFO) << std::endl << "ParallelSplitShadowMap: GLSL shader code:" << std::endl << "-------------------------------------------------------------------"  << std::endl << sstr.str() << std::endl;

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


ParallelSplitShadowMap::ParallelSplitShadowMap(osg::Geode** gr, int icountplanes) :
_textureUnitOffset(1),
_debug_color_in_GLSL(false),
_user_polgyonOffset_set(false),
_resolution(TEXTURE_RESOLUTION),
_isSetMaxFarDistance(false),
_split_min_near_dist(ZNEAR_MIN_FROM_LIGHT_SOURCE),
_linearSplit(LINEAR_SPLIT),
_move_vcam_behind_rcam_factor(MOVE_VIRTUAL_CAMERA_BEHIND_REAL_CAMERA_FACTOR),
_userLight(NULL),
_ambientBias(0.1,0.3),
_ambientBiasUniform(NULL)
{
    _displayTexturesGroupingNode = gr;
    _number_of_splits = icountplanes;

    _polgyonOffset.set(0.01f,0.01f);
}

ParallelSplitShadowMap::ParallelSplitShadowMap(const ParallelSplitShadowMap& copy, const osg::CopyOp& copyop):
ShadowTechnique(copy,copyop),
_textureUnitOffset(copy._textureUnitOffset),
_debug_color_in_GLSL(copy._debug_color_in_GLSL),
_user_polgyonOffset_set(copy._user_polgyonOffset_set),
_resolution(copy._resolution),
_isSetMaxFarDistance(copy._isSetMaxFarDistance),
_split_min_near_dist(copy._split_min_near_dist),
_linearSplit(copy._linearSplit),
_number_of_splits(copy._number_of_splits),
_polgyonOffset(copy._polgyonOffset),
_setMaxFarDistance(copy._setMaxFarDistance),
_move_vcam_behind_rcam_factor(copy._move_vcam_behind_rcam_factor),
_userLight(copy._userLight),
_ambientBias(copy._ambientBias)
{
}

void ParallelSplitShadowMap::setAmbientBias(const osg::Vec2& ambientBias)
{
    _ambientBias = ambientBias;
    if (_ambientBiasUniform ) _ambientBiasUniform->set(_ambientBias);
}

void ParallelSplitShadowMap::init(){
    if (!_shadowedScene) return;

    osg::StateSet* sharedStateSet = new osg::StateSet;


    unsigned int iCamerasMax=_number_of_splits;
    for (unsigned int iCameras=0;iCameras<iCamerasMax;iCameras++)
    {
        PSSMShadowSplitTexture pssmShadowSplitTexture;
        pssmShadowSplitTexture._splitID = iCameras;
        pssmShadowSplitTexture._textureUnit = iCameras+_textureUnitOffset;

        pssmShadowSplitTexture._resolution = _resolution;

        osg::notify(osg::DEBUG_INFO) << "ParallelSplitShadowMap : Texture ID=" << iCameras << " Resolution=" << pssmShadowSplitTexture._resolution << std::endl;
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
            pssmShadowSplitTexture._texture->setBorderColor(osg::Vec4(1.0,1.0,1.0,1.0));
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
            pssmShadowSplitTexture._camera->setReferenceFrame(osg::Camera::ABSOLUTE_RF_INHERIT_VIEWPOINT);

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


            pssmShadowSplitTexture._depth = new osg::Depth;
            stateset->setAttribute(pssmShadowSplitTexture._depth.get(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

            //////////////////////////////////////////////////////////////////////////
            float factor = _polgyonOffset.x();
            float units  = _polgyonOffset.y();
             osg::ref_ptr<osg::PolygonOffset> polygon_offset = new osg::PolygonOffset;
             polygon_offset->setFactor(factor);
            polygon_offset->setUnits(units);
            stateset->setAttribute(polygon_offset.get(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
            stateset->setMode(GL_POLYGON_OFFSET_FILL, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);


            //////////////////////////////////////////////////////////////////////////
            osg::ref_ptr<osg::CullFace> cull_face = new osg::CullFace;
            cull_face->setMode(osg::CullFace::FRONT);
            stateset->setAttribute(cull_face.get(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
            stateset->setMode(GL_CULL_FACE, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

            //////////////////////////////////////////////////////////////////////////
            osg::ShadeModel* shademodel = dynamic_cast<osg::ShadeModel*>(stateset->getAttribute(osg::StateAttribute::SHADEMODEL));
            if (!shademodel){shademodel = new osg::ShadeModel;stateset->setAttribute(shademodel);}
            shademodel->setMode( osg::ShadeModel::FLAT );
            stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
        }

        //////////////////////////////////////////////////////////////////////////
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

        //////////////////////////////////////////////////////////////////////////
        // set up shader (GLSL)
        #ifdef SHADOW_TEXTURE_GLSL

            osg::Program* program = new osg::Program;
            pssmShadowSplitTexture._stateset->setAttribute(program);

            //////////////////////////////////////////////////////////////////////////
            // GLSL PROGRAMS
             osg::Shader* fragment_shader = new osg::Shader(osg::Shader::FRAGMENT, generateGLSL_FragmentShader_BaseTex(_displayTexturesGroupingNode!=NULL,iCameras).c_str());
            program->addShader(fragment_shader);

          
            //////////////////////////////////////////////////////////////////////////
            // UNIFORMS
            std::stringstream strST; strST << "shadowTexture" << (pssmShadowSplitTexture._textureUnit-_textureUnitOffset);
            osg::Uniform* shadowTextureSampler = new osg::Uniform(strST.str().c_str(),(int)(pssmShadowSplitTexture._textureUnit));
            pssmShadowSplitTexture._stateset->addUniform(shadowTextureSampler);

            //TODO: NOT YET SUPPORTED in the current version of the shader
            if ( ! _ambientBiasUniform ) {
                _ambientBiasUniform = new osg::Uniform("ambientBias",_ambientBias);
                pssmShadowSplitTexture._stateset->addUniform(_ambientBiasUniform);
            }
            

            std::stringstream strzShadow; strzShadow << "zShadow" << (pssmShadowSplitTexture._textureUnit-_textureUnitOffset);
            pssmShadowSplitTexture._farDistanceSplit = new osg::Uniform(strzShadow.str().c_str(),1.0f);
            pssmShadowSplitTexture._stateset->addUniform(pssmShadowSplitTexture._farDistanceSplit);

            osg::Uniform* baseTextureSampler = new osg::Uniform("baseTexture",0);
            pssmShadowSplitTexture._stateset->addUniform(baseTextureSampler);

            osg::Uniform* randomTextureSampler = new osg::Uniform("randomTexture",(int)(_textureUnitOffset+_number_of_splits));
            pssmShadowSplitTexture._stateset->addUniform(randomTextureSampler);

            if ( _textureUnitOffset > 0 ) {
                osg::Uniform* enableBaseTexture = new osg::Uniform("enableBaseTexture",1.0f);
                pssmShadowSplitTexture._stateset->addUniform(enableBaseTexture);
            } else {
                osg::Uniform* enableBaseTexture = new osg::Uniform("enableBaseTexture",0.0f);
                pssmShadowSplitTexture._stateset->addUniform(enableBaseTexture);
            }

            for (unsigned int textLoop(0);textLoop<_textureUnitOffset;textLoop++)
            {
                // fake texture for baseTexture, add a fake texture
                // we support by default at least one texture layer
                // without this fake texture we can not support
                // textured and not textured scene

                // TODO: at the moment the PSSM supports just one texture layer in the GLSL shader, multitexture are
                //       not yet supported !

                osg::Image* image = new osg::Image;
                // allocate the image data, noPixels x 1 x 1 with 4 rgba floats - equivalent to a Vec4!
                int noPixels = 1;
                image->allocateImage(noPixels,1,1,GL_RGBA,GL_FLOAT);
                image->setInternalTextureFormat(GL_RGBA);
                // fill in the image data.
                osg::Vec4* dataPtr = (osg::Vec4*)image->data();
                osg::Vec4f color(1.0f,1.0f,1.0f,0.0f);
                *dataPtr = color;
                // make fake texture
                osg::Texture2D* texture = new osg::Texture2D;
                texture->setWrap(osg::Texture2D::WRAP_S,osg::Texture2D::CLAMP_TO_BORDER);
                texture->setWrap(osg::Texture2D::WRAP_T,osg::Texture2D::CLAMP_TO_BORDER);
                texture->setBorderColor(osg::Vec4(1.0,1.0,1.0,1.0));
                texture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
                texture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);
                texture->setImage(image);
                // add fake texture
                pssmShadowSplitTexture._stateset->setTextureAttribute(textLoop,texture,osg::StateAttribute::ON);
                pssmShadowSplitTexture._stateset->setTextureMode(textLoop,GL_TEXTURE_1D,osg::StateAttribute::OFF);
                pssmShadowSplitTexture._stateset->setTextureMode(textLoop,GL_TEXTURE_2D,osg::StateAttribute::ON);
                pssmShadowSplitTexture._stateset->setTextureMode(textLoop,GL_TEXTURE_3D,osg::StateAttribute::OFF);
            }



            #ifdef SMOOTH_SHADOW
            {
                // texture for randomTexture (for smoothing shadow edges)
                osg::Image* image = new osg::Image;
                // allocate the image data, noPixels x noPixels x 1 with 4 rgba floats - equivalent to a Vec4!
                int noPixels = 128;
                image->allocateImage(noPixels,noPixels,1,GL_RGBA,GL_FLOAT);
                image->setInternalTextureFormat(GL_RGBA);
                // fill in the image data.
                osg::Vec4* dataPtr = (osg::Vec4*)image->data();
                for (int s=0;s<noPixels;s++) {
                    for (int t=0;t<noPixels;t++) {
                        float randr=(rand()/(RAND_MAX+1.0));
                        float randg=(rand()/(RAND_MAX+1.0));
                        float randb=(rand()/(RAND_MAX+1.0));
                        float randa=(rand()/(RAND_MAX+1.0));
                        (*dataPtr).set(randr,randg,randb,randa);
                        dataPtr++;
                    }
                }
                // make fake texture
                osg::Texture2D* texture = new osg::Texture2D;
                texture->setWrap(osg::Texture2D::WRAP_S,osg::Texture2D::CLAMP_TO_BORDER);
                texture->setWrap(osg::Texture2D::WRAP_T,osg::Texture2D::CLAMP_TO_BORDER);
                texture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
                texture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);
                texture->setImage(image);
                // add fake texture
                pssmShadowSplitTexture._stateset->setTextureAttribute(_textureUnitOffset+_number_of_splits,texture,osg::StateAttribute::ON);
                pssmShadowSplitTexture._stateset->setTextureMode(_textureUnitOffset+_number_of_splits,GL_TEXTURE_1D,osg::StateAttribute::OFF);
                pssmShadowSplitTexture._stateset->setTextureMode(_textureUnitOffset+_number_of_splits,GL_TEXTURE_2D,osg::StateAttribute::ON);
                pssmShadowSplitTexture._stateset->setTextureMode(_textureUnitOffset+_number_of_splits,GL_TEXTURE_3D,osg::StateAttribute::OFF);
            }
            #endif
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

    // do traversal of shadow receiving scene which does need to be decorated by the shadow map
    for (PSSMShadowSplitTextureMap::iterator it=_PSSMShadowSplitTextureMap.begin();it!=_PSSMShadowSplitTextureMap.end();it++)
    {
        PSSMShadowSplitTexture pssmShadowSplitTexture = it->second;
        cv.pushStateSet(pssmShadowSplitTexture._stateset.get());

        //////////////////////////////////////////////////////////////////////////
        // DEBUG
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
    //////////////////////////////////////////////////////////////////////////
    const osg::Light* selectLight = 0;

    /// light pos and light direction 
    osg::Vec4 lightpos;
    osg::Vec3 lightDirection;

    if ( ! _userLight ) {
        // try to find a light in the scene
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
    }else{
        // take the user light as light source
        lightpos = _userLight->getPosition();
        lightDirection = _userLight->getDirection();
        selectLight = _userLight;
    }    

    if (selectLight)
    {

        // do traversal of shadow receiving scene which does need to be decorated by the shadow map
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


//////////////////////////////////////////////////////////////////////////
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
//////////////////////////////////////////////////////////////////////////


void ParallelSplitShadowMap::calculateFrustumCorners(
    PSSMShadowSplitTexture &pssmShadowSplitTexture,
    osg::Vec3d *frustumCorners
    ) {

        // get user cameras
        double fovy,aspectRatio,camNear,camFar;
        pssmShadowSplitTexture._cameraProj.getPerspective(fovy,aspectRatio,camNear,camFar);


        // force to max far distance to show shadow, for some scene it can be solve performance problems.
        if ( _isSetMaxFarDistance ) {
            if (_setMaxFarDistance < camFar) camFar = _setMaxFarDistance;
        }


        // build camera matrix with some offsets (the user view camera)
        osg::Matrixf viewMat;
        osg::Vec3d camEye,camCenter,camUp;
        pssmShadowSplitTexture._cameraView.getLookAt(camEye,camCenter,camUp);
        osg::Vec3d viewDir = camCenter - camEye;
        viewDir.normalize();
        camEye = camEye  - viewDir * _move_vcam_behind_rcam_factor;
        camFar += _move_vcam_behind_rcam_factor * viewDir.length();
        viewMat.makeLookAt(camEye,camCenter,camUp);



        //////////////////////////////////////////////////////////////////////////
        /// CALCULATE SPLIT
        double maxFar = camFar;
        double minNear = camNear;
        double camNearFar_Dist = maxFar - camNear;
        if ( _linearSplit ) {
            camFar  = camNear + (camNearFar_Dist) * ((double)(pssmShadowSplitTexture._splitID+1))/((double)(_number_of_splits));
            camNear = camNear + (camNearFar_Dist) * ((double)(pssmShadowSplitTexture._splitID))/((double)(_number_of_splits));
        } else {
            // Exponential split scheme:
            //
            // Ci = (n - f)*(i/numsplits)^(bias+1) + n;
            //
            static float fSplitSchemeBias[2]={0.25f,0.66f};
            fSplitSchemeBias[1]=Clamp(fSplitSchemeBias[1],0.0f,3.0f);
            float* pSplitDistances =new float[_number_of_splits+1];

            for(int i=0;i<(int)_number_of_splits;i++) {
                float fIDM=i/(float)_number_of_splits;
                pSplitDistances[i]=camNearFar_Dist*(powf(fIDM,fSplitSchemeBias[1]+1))+camNear;
            }
            // make sure border values are right
            pSplitDistances[0]=camNear;
            pSplitDistances[_number_of_splits]=camFar;

            camNear = pSplitDistances[pssmShadowSplitTexture._splitID];
            camFar  = pSplitDistances[pssmShadowSplitTexture._splitID+1];

            delete[] pSplitDistances;
        }

        //pssmShadowSplitTexture._depth->setRange((camNear - minNear) / camNearFar_Dist,(camFar  - minNear)/ camNearFar_Dist);
        pssmShadowSplitTexture._depth->setRange(0.0,1.0);
        pssmShadowSplitTexture._depth->setFunction(osg::Depth::LEQUAL);

        #ifdef SHADOW_TEXTURE_GLSL
            float fVal = (float)((camFar-minNear)/camNearFar_Dist);
            //std::cout << pssmShadowSplitTexture._farDistanceSplit->getName() << " " << fVal <<  std::endl;
            pssmShadowSplitTexture._farDistanceSplit->set(fVal);
        #endif

        //////////////////////////////////////////////////////////////////////////
        /// TRANSFORM frustum corners (Optimized for Orthogonal)
        osg::Matrixf invProjViewMat;


        osg::Matrixf projMat;
        projMat.makePerspective(fovy,aspectRatio,camNear,camFar);

        osg::Matrixf projViewMat(viewMat*projMat);
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


        //std::cout << "camFar : "<<pssmShadowSplitTexture._splitID << " / " << camNear << "," << camFar << std::endl;
}

//////////////////////////////////////////////////////////////////////////
//
// compute directional light initial position;
void ParallelSplitShadowMap::calculateLightInitalPosition(PSSMShadowSplitTexture &pssmShadowSplitTexture,osg::Vec3d *frustumCorners){
    pssmShadowSplitTexture._frustumSplitCenter = frustumCorners[0];
    for(int i=1;i<8;i++) {
        pssmShadowSplitTexture._frustumSplitCenter +=frustumCorners[i];
    }
    pssmShadowSplitTexture._frustumSplitCenter /= 8.0;

    //
    // To avoid edge problems, scale the frustum so
    // that it's at least a few pixels larger
    //
    for(int i=0;i<8;i++)
    {
        // scale by adding offset from center
        frustumCorners[i]+=(frustumCorners[i]-pssmShadowSplitTexture._frustumSplitCenter)*(0.25);
    }

}

void ParallelSplitShadowMap::calculateLightNearFarFormFrustum(
    PSSMShadowSplitTexture &pssmShadowSplitTexture,
    osg::Vec3d *frustumCorners
    ) {

        //calculate near, far
        double zFar(-DBL_MAX);

        // calculate zFar (as longest distance)
        for(int i=0;i<8;i++) {
            double dist_z_from_light = fabs(pssmShadowSplitTexture._lightDirection*(frustumCorners[i] -  pssmShadowSplitTexture._frustumSplitCenter));
            if ( zFar  < dist_z_from_light ) zFar  = dist_z_from_light;
        } 

        // update camera position and look at center
        pssmShadowSplitTexture._lightCameraSource = pssmShadowSplitTexture._frustumSplitCenter - pssmShadowSplitTexture._lightDirection*(zFar+_split_min_near_dist);
        pssmShadowSplitTexture._lightCameraTarget = pssmShadowSplitTexture._frustumSplitCenter + pssmShadowSplitTexture._lightDirection*(zFar);
        // update near - far plane
        pssmShadowSplitTexture._lightNear = 0.01;
        pssmShadowSplitTexture._lightFar  = zFar*2.0 + _split_min_near_dist;

        // calculate [zNear,zFar] 
        zFar = (-DBL_MAX);
        double zNear(DBL_MAX);
        for(int i=0;i<8;i++) {
            double dist_z_from_light = fabs(pssmShadowSplitTexture._lightDirection*(frustumCorners[i] -  pssmShadowSplitTexture._lightCameraSource));
            if ( zFar  < dist_z_from_light ) zFar  = dist_z_from_light;
            if ( zNear > dist_z_from_light ) zNear  = dist_z_from_light;
        } 
        // update near - far plane
        pssmShadowSplitTexture._lightNear = zNear - _split_min_near_dist + 0.01;
        pssmShadowSplitTexture._lightFar  = zFar;
}




void ParallelSplitShadowMap::calculateLightViewProjectionFormFrustum(PSSMShadowSplitTexture &pssmShadowSplitTexture,osg::Vec3d *frustumCorners) {



    // calculate the camera's coordinate system
    osg::Vec3d camEye,camCenter,camUp;
    pssmShadowSplitTexture._cameraView.getLookAt(camEye,camCenter,camUp);

    osg::Vec3d viewDir(camCenter-camEye);
    viewDir.normalize();

    osg::Vec3d camLeft(camUp ^ viewDir);
    camLeft.normalize();

    osg::Vec3d top(pssmShadowSplitTexture._lightDirection ^ camLeft);
    if(top.length2() < 0.5) top = pssmShadowSplitTexture._lightDirection ^ camUp;

    osg::Vec3d left(top ^ pssmShadowSplitTexture._lightDirection);


    // calculate the camera's frustum left,right,bottom,top parameters
    double maxLeft(-DBL_MAX),maxTop(-DBL_MAX);
    double minLeft(DBL_MAX),minTop(DBL_MAX);

    for(int i(0); i < 8; i++)
    {
        osg::Vec3d diffCorner(pssmShadowSplitTexture._lightCameraSource - frustumCorners[i]);
        double lLeft(diffCorner*left);  
        double lTop(diffCorner*top); 

        if ( lLeft > maxLeft ) maxLeft  =  lLeft;
        if ( lTop  > maxTop  ) maxTop   =  lTop;

        if ( lLeft < minLeft ) minLeft  =  lLeft;
        if ( lTop  < minTop  ) minTop   =  lTop;
    }


    // make the camera view matrix
    pssmShadowSplitTexture._camera->setViewMatrixAsLookAt(pssmShadowSplitTexture._lightCameraSource,pssmShadowSplitTexture._lightCameraTarget,top);

    // use ortho projection for light (directional light only supported)
    pssmShadowSplitTexture._camera->setProjectionMatrixAsOrtho(minLeft,maxLeft,minTop,maxTop,pssmShadowSplitTexture._lightNear,pssmShadowSplitTexture._lightFar);
}



