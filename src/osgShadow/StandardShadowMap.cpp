/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield 
 *
 * This library is open source and may be redistributed and/or modified under  
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or 
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * OpenSceneGraph Public License for more details.
 *
 * ViewDependentShadow codes Copyright (C) 2008 Wojciech Lewandowski
 * Thanks to to my company http://www.ai.com.pl for allowing me free this work.
*/

#include <osgShadow/StandardShadowMap>
#include <osg/PolygonOffset>
#include <osg/ComputeBoundsVisitor>
#include <osgShadow/ShadowedScene>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/CullFace>
#include <osg/AlphaFunc>
#include <osg/Point>

#include <stdio.h>

using namespace osgShadow;

#define DISPLAY_SHADOW_TEXEL_TO_PIXEL_ERROR 0


StandardShadowMap::StandardShadowMap():
    BaseClass(),    
    _polygonOffsetFactor( 1.1f ),
    _polygonOffsetUnits( 4.0f ),
    _textureSize( 1024, 1024 ),
    _baseTextureUnit( 0 ),
    _shadowTextureUnit( 1 ),
    _baseTextureCoordIndex( 0 ),
    _shadowTextureCoordIndex( 1 )

{ 
    _mainFragmentShader = new osg::Shader( osg::Shader::FRAGMENT,
        " // following expressions are auto modified - do not change them:       \n"
        " // gl_TexCoord[0]  0 - can be subsituted with other index              \n"
        "                                                                        \n"
        "float DynamicShadow( );                                                 \n"
        "                                                                        \n"
        "varying vec4 colorAmbientEmissive;                                      \n"
        "                                                                        \n"
        "uniform sampler2D baseTexture;                                          \n"
        "                                                                        \n"
        "void main(void)                                                         \n"
        "{                                                                       \n"
        "  vec4 color = texture2D( baseTexture, gl_TexCoord[0].xy );             \n"
        "  color *= mix( colorAmbientEmissive, gl_Color, DynamicShadow() );      \n"
#if DISPLAY_SHADOW_TEXEL_TO_PIXEL_ERROR
        "  color.xy = abs( dFdy( gl_TexCoord[1].xy / gl_TexCoord[1].w ) )* 1024.0; \n"
        "  color.z = color.y; \n"
        "  color.x = color.z; \n"
        "  color.y = color.z; \n"
        "  color.a = 1.0; \n"
#endif
//      "  float fog = clamp((gl_Fog.end - gl_FogFragCoord)*gl_Fog.scale, 0.,1.);\n"
//      "  color.rgb = mix( gl_Fog.color.rgb, color.rgb, fog );                  \n"
        "  gl_FragColor = color;                                                 \n"
        "} \n" );   
    
    
    _shadowFragmentShader = new osg::Shader( osg::Shader::FRAGMENT,
        " // following expressions are auto modified - do not change them:      \n"
        " // gl_TexCoord[1]  1 - can be subsituted with other index             \n"
        "                                                                       \n"
        "uniform sampler2DShadow shadowTexture;                                 \n"
        "                                                                       \n"
        "float DynamicShadow( )                                                 \n"
        "{                                                                      \n"
        "    return shadow2DProj( shadowTexture, gl_TexCoord[1] ).r;            \n"
        "} \n" );

    
    
    _shadowVertexShader = new osg::Shader( osg::Shader::VERTEX,
        " // following expressions are auto modified - do not change them:      \n"
        " // gl_TexCoord[1]  1 - can be subsituted with other index             \n"
        " // gl_EyePlaneS[1] 1 - can be subsituted with other index             \n"
        " // gl_EyePlaneT[1] 1 - can be subsituted with other index             \n"
        " // gl_EyePlaneR[1] 1 - can be subsituted with other index             \n"
        " // gl_EyePlaneQ[1] 1 - can be subsituted with other index             \n"
        "                                                                       \n"
        "void DynamicShadow( in vec4 ecPosition )                               \n"
        "{                                                                      \n"
        "    // generate coords for shadow mapping                              \n"
        "    gl_TexCoord[1].s = dot( ecPosition, gl_EyePlaneS[1] );             \n"
        "    gl_TexCoord[1].t = dot( ecPosition, gl_EyePlaneT[1] );             \n"
        "    gl_TexCoord[1].p = dot( ecPosition, gl_EyePlaneR[1] );             \n"
        "    gl_TexCoord[1].q = dot( ecPosition, gl_EyePlaneQ[1] );             \n"
        "} \n" );
    
    _mainVertexShader = new osg::Shader( osg::Shader::VERTEX,
        " // following expressions are auto modified - do not change them:      \n"
        " // gl_TexCoord[0]      0 - can be subsituted with other index         \n"
        " // gl_TextureMatrix[0] 0 - can be subsituted with other index         \n"
        " // gl_MultiTexCoord0   0 - can be subsituted with other index         \n"
        "                                                                       \n"    
        "const int NumEnabledLights = 1;                                        \n"
        "                                                                       \n"
        "void DynamicShadow( in vec4 ecPosition );                              \n"
        "                                                                        \n"
        "varying vec4 colorAmbientEmissive;                                     \n"
        "                                                                       \n"
        "void SpotLight(in int i,                                               \n" 
        "               in vec3 eye,                                            \n"
        "               in vec3 ecPosition3,                                    \n"
        "               in vec3 normal,                                            \n"
        "               inout vec4 ambient,                                     \n"
        "               inout vec4 diffuse,                                        \n"
        "               inout vec4 specular)                                    \n"
        "{                                                                        \n"
        "    float nDotVP;          // normal . light direction                 \n"
        "    float nDotHV;          // normal . light half vector                \n"
        "    float pf;              // power factor                                \n"
        "    float spotDot;         // cosine of angle between spotlight        \n"
        "    float spotAttenuation; // spotlight attenuation factor             \n"
        "    float attenuation;     // computed attenuation factor                \n"
        "    float d;               // distance from surface to light source    \n"
        "    vec3 VP;               // direction from surface to light position \n"
        "    vec3 halfVector;       // direction of maximum highlights          \n"
        "                                                                        \n"
        "    // Compute vector from surface to light position                    \n"
        "    VP = vec3(gl_LightSource[i].position) - ecPosition3;                \n"
        "                                                                       \n"
        "    // Compute distance between surface and light position                \n"
        "    d = length(VP);                                                    \n"
        "                                                                        \n"
        "    // Normalize the vector from surface to light position             \n"
        "    VP = normalize(VP);                                                \n"
        "                                                                        \n"
        "    // Compute attenuation                                                \n"
        "    attenuation = 1.0 / (gl_LightSource[i].constantAttenuation +       \n" 
        "                         gl_LightSource[i].linearAttenuation * d +        \n"
        "                         gl_LightSource[i].quadraticAttenuation *d*d); \n"
        "                                                                        \n"
        "    // See if point on surface is inside cone of illumination          \n"
        "    spotDot = dot(-VP, normalize(gl_LightSource[i].spotDirection));    \n"
        "                                                                        \n"
        "    if (spotDot < gl_LightSource[i].spotCosCutoff)                        \n"
        "        spotAttenuation = 0.0; // light adds no contribution            \n"
        "    else                                                                \n"
        "        spotAttenuation = pow(spotDot, gl_LightSource[i].spotExponent);\n"
        "                                                                        \n"
        "    // Combine the spotlight and distance attenuation.                    \n"
        "    attenuation *= spotAttenuation;                                    \n"
        "                                                                        \n"
        "    halfVector = normalize(VP + eye);                                    \n"
        "                                                                        \n"
        "    nDotVP = max(0.0, dot(normal, VP));                                \n"
        "    nDotHV = max(0.0, dot(normal, halfVector));                        \n"
        "                                                                        \n"
        "    if (nDotVP == 0.0)                                                 \n"
        "        pf = 0.0;                                                        \n"
        "    else                                                                \n"
        "        pf = pow(nDotHV, gl_FrontMaterial.shininess);                    \n"
        "                                                                        \n"
        "    ambient  += gl_LightSource[i].ambient * attenuation;                \n"
        "    diffuse  += gl_LightSource[i].diffuse * nDotVP * attenuation;        \n"
        "    specular += gl_LightSource[i].specular * pf * attenuation;            \n"
        "}                                                                        \n"
        "                                                                        \n"
        "void PointLight(in int i,                                                \n"
        "                in vec3 eye,                                            \n"
        "                in vec3 ecPosition3,                                    \n"
        "                in vec3 normal,                                        \n"
        "                inout vec4 ambient,                                    \n"
        "                inout vec4 diffuse,                                    \n"
        "                inout vec4 specular)                                   \n" 
        "{                                                                        \n"
        "    float nDotVP;      // normal . light direction                        \n"
        "    float nDotHV;      // normal . light half vector                    \n"
        "    float pf;          // power factor                                    \n"
        "    float attenuation; // computed attenuation factor                    \n"
        "    float d;           // distance from surface to light source        \n"
        "    vec3  VP;          // direction from surface to light position        \n"
        "    vec3  halfVector;  // direction of maximum highlights                \n"
        "                                                                        \n"
        "    // Compute vector from surface to light position                    \n"
        "    VP = vec3(gl_LightSource[i].position) - ecPosition3;                \n"
        "                                                                        \n"
        "    // Compute distance between surface and light position                \n"
        "    d = length(VP);                                                    \n"
        "                                                                        \n"
        "    // Normalize the vector from surface to light position             \n" 
        "    VP = normalize(VP);                                                \n"
        "                                                                        \n"
        "    // Compute attenuation                                                \n"
        "    attenuation = 1.0 / (gl_LightSource[i].constantAttenuation +        \n"
        "                         gl_LightSource[i].linearAttenuation * d +        \n"
        "                         gl_LightSource[i].quadraticAttenuation * d*d);\n"
        "                                                                        \n"
        "    halfVector = normalize(VP + eye);                                    \n"
        "                                                                        \n"
        "    nDotVP = max(0.0, dot(normal, VP));                                \n"
        "    nDotHV = max(0.0, dot(normal, halfVector));                        \n"
        "                                                                        \n"
        "    if (nDotVP == 0.0)                                                    \n"
        "        pf = 0.0;                                                        \n"
        "    else                                                                \n"
        "        pf = pow(nDotHV, gl_FrontMaterial.shininess);                  \n"
        "                                                                        \n"
        "    ambient += gl_LightSource[i].ambient * attenuation;                \n"
        "    diffuse += gl_LightSource[i].diffuse * nDotVP * attenuation;        \n"
        "    specular += gl_LightSource[i].specular * pf * attenuation;            \n"
        "}                                                                        \n"
        "                                                                        \n"
        "void DirectionalLight(in int i,                                        \n"
        "                      in vec3 normal,                                    \n"
        "                      inout vec4 ambient,                                \n"
        "                      inout vec4 diffuse,                                \n"
        "                      inout vec4 specular)                                \n"
        "{                                                                        \n"
        "     float nDotVP;         // normal . light direction                    \n"
        "     float nDotHV;         // normal . light half vector                \n"
        "     float pf;             // power factor                                \n"
        "                                                                        \n"
        "     nDotVP = max(0.0, dot(normal,                                        \n"
        "                normalize(vec3(gl_LightSource[i].position))));            \n"
        "     nDotHV = max(0.0, dot(normal,                                        \n"
        "                      vec3(gl_LightSource[i].halfVector)));            \n"
        "                                                                        \n"
        "     if (nDotVP == 0.0)                                                \n"
        "         pf = 0.0;                                                        \n"
        "     else                                                                \n"
        "         pf = pow(nDotHV, gl_FrontMaterial.shininess);                    \n"
        "                                                                        \n"
        "     ambient  += gl_LightSource[i].ambient;                            \n"
        "     diffuse  += gl_LightSource[i].diffuse * nDotVP;                    \n"
        "     specular += gl_LightSource[i].specular * pf;                        \n"
        "}                                                                        \n"
        "                                                                        \n"
        "void main( )                                                            \n"
        "{                                                                        \n"
        "    // Transform vertex to clip space                                    \n"
        "    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;            \n"
        "    vec3 normal = normalize( gl_NormalMatrix * gl_Normal );            \n"
        "                                                                        \n"
        "    vec4  ecPos  = gl_ModelViewMatrix * gl_Vertex;                        \n"
        "    float ecLen  = length( ecPos );                                    \n"
        "    vec3  ecPosition3 = ecPos.xyz / ecPos.w;                           \n"
        "                                                                       \n" 
        "    vec3  eye = vec3( 0.0, 0.0, 1.0 );                                 \n"
        "    //vec3  eye = -normalize(ecPosition3);                             \n"
        "                                                                       \n"
        "    DynamicShadow( ecPos );                                            \n"
        "                                                                        \n"
        "     gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;          \n"
        "                                                                        \n"
        "    // Front Face lighting                                                \n"
        "                                                                        \n"
        "    // Clear the light intensity accumulators                            \n"
        "    vec4 amb  = vec4(0.0);                                                \n"
        "    vec4 diff = vec4(0.0);                                                \n"
        "    vec4 spec = vec4(0.0);                                                \n"
        "                                                                        \n"
        "    // Loop through enabled lights, compute contribution from each        \n"
        "    for (int i = 0; i < NumEnabledLights; i++)                          \n"
        "   {                                                                   \n" 
        "       if (gl_LightSource[i].position.w == 0.0)                        \n"
        "           DirectionalLight(i, normal, amb, diff, spec);               \n" 
        "       else if (gl_LightSource[i].spotCutoff == 180.0)                 \n"
        "           PointLight(i, eye, ecPosition3, normal, amb, diff, spec);   \n"   
        "       else                                                            \n"  
        "           SpotLight(i, eye, ecPosition3, normal, amb, diff, spec);    \n" 
        "    }                                                                  \n"
        "                                                                       \n" 
        "    colorAmbientEmissive = gl_FrontLightModelProduct.sceneColor +        \n"
        "                           amb * gl_FrontMaterial.ambient;             \n"       
        "                                                                       \n"
        "    gl_FrontColor = colorAmbientEmissive +                             \n"
        "                    diff * gl_FrontMaterial.diffuse;                   \n"
        "                                                                        \n"
        "     gl_FrontSecondaryColor = vec4(spec*gl_FrontMaterial.specular);     \n"
        "                                                                        \n"
        "    gl_BackColor = gl_FrontColor;                                        \n"
        "    gl_BackSecondaryColor = gl_FrontSecondaryColor;                    \n"
        "                                                                        \n"
        "    gl_FogFragCoord = ecLen;                                            \n"
        "} \n" );                                                                
}                                                                                

StandardShadowMap::StandardShadowMap(const StandardShadowMap& copy, const osg::CopyOp& copyop) :
    BaseClass(copy,copyop),
    _polygonOffsetFactor( copy._polygonOffsetFactor ),
    _polygonOffsetUnits( copy._polygonOffsetUnits ),
    _textureSize( copy._textureSize ),
    _baseTextureUnit( copy._baseTextureUnit ),
    _shadowTextureUnit( copy._shadowTextureUnit )
{
    if( copy._mainVertexShader.valid() )
        _mainVertexShader = dynamic_cast<osg::Shader*>
            ( copy._mainVertexShader->clone(copyop) );

    if( copy._mainFragmentShader.valid() )
        _mainFragmentShader = dynamic_cast<osg::Shader*>
            ( copy._mainFragmentShader->clone(copyop) );

    if( copy._shadowVertexShader.valid() )
        _shadowVertexShader = dynamic_cast<osg::Shader*>
            ( copy._shadowVertexShader->clone(copyop) );

    if( copy._shadowFragmentShader.valid() )
        _shadowFragmentShader = dynamic_cast<osg::Shader*>
            ( copy._shadowFragmentShader->clone(copyop) );
}

StandardShadowMap::~StandardShadowMap(void)                                        
{                                                                                
                                                                                
}                                    

void StandardShadowMap::updateTextureCoordIndices( unsigned int fromTextureCoordIndex, unsigned int toTextureCoordIndex )
{

    if( fromTextureCoordIndex == toTextureCoordIndex ) return;

    const char *expressions[] = {
        "gl_TexCoord[","]",
        "gl_TextureMatrix[","]",
        "gl_MultiTexCoord","",
        "gl_EyePlaneS[","]",
        "gl_EyePlaneT[","]",
        "gl_EyePlaneR[","]",
        "gl_EyePlaneQ[","]"
    };

    for( unsigned int i = 0;
         i < sizeof( expressions ) / sizeof( expressions[0] );
         i+=2 )
    {
        char acFrom[ 32 ], acTo[32];

        // its not elegant to mix stdio & stl strings 
        // but in this context I do an exception for cleaner code

        sprintf( acFrom, "%s%d%s", expressions[i], fromTextureCoordIndex, expressions[i+1]);
        sprintf( acTo, "%s%d%s", expressions[i], toTextureCoordIndex, expressions[i+1]);

        std::string from( acFrom ), to( acTo );

        searchAndReplaceShaderSource( getShadowVertexShader(), from, to );
        searchAndReplaceShaderSource( getShadowFragmentShader(), from, to );
        searchAndReplaceShaderSource( getMainVertexShader(), from, to );
        searchAndReplaceShaderSource( getMainFragmentShader(), from, to );
    }

    dirty();
}

void StandardShadowMap::searchAndReplaceShaderSource
           ( osg::Shader* shader, std::string fromString, std::string toString )
{
    if( !shader || fromString == toString ) return;

    const std::string & srceString = shader->getShaderSource();
    std::string destString;

    std::string::size_type fromLength = fromString.length();
    std::string::size_type srceLength = srceString.length();   

    for( std::string::size_type pos = 0; pos < srceLength; )
    {
        std::string::size_type end = srceString.find( fromString, pos );

        if( end == std::string::npos )
            end = srceLength;

        destString.append( srceString, pos, end - pos );
        
        if( end == srceLength )
            break;

        destString.append( toString );
        pos = end + fromLength;
    }

    shader->setShaderSource( destString );
}

void StandardShadowMap::ViewData::cull()
{
    // step 1: 
    // cull shadowed scene ie put into render bins and states into stage graphs 
    cullShadowReceivingScene( );

    // step 2:
    // find the light casting our shadows 
    osg::Vec4 lightPos;
    osg::Vec3 lightDir;
    osg::Vec3 lightUp( 0,0,0 ); // force computing most approprate dir
    const osg::Light *light = selectLight( lightPos, lightDir );

    if ( !light )
        return;// bail out - no shadowing needed in darkest night

    // step 3:
    // compute shadow casting matrices and apply them to shadow map camera
    aimShadowCastingCamera( light, lightPos, lightDir, lightUp );

    // step 4:
    // cull scene casting shadow and generate render  
    cullShadowCastingScene( );

    // step 5:
    // setup texgen generating shadow map coords for the shadow receiving scene 
    addShadowReceivingTexGen( );

    BaseClass::ViewData::cull();
}

void StandardShadowMap::ViewData::init( ThisClass *st, osgUtil::CullVisitor *cv )
{
    BaseClass::ViewData::init( st, cv );

    _lightPtr             = &st->_light;
    _shadowTextureUnitPtr = &st->_shadowTextureUnit;
    _baseTextureUnitPtr   = &st->_baseTextureUnit;

    _texture = new osg::Texture2D;
    { // Setup shadow texture
        _texture->setTextureSize( st->_textureSize.x(), st->_textureSize.y());
        _texture->setInternalFormat(GL_DEPTH_COMPONENT);
        _texture->setShadowComparison(true);
        _texture->setShadowTextureMode(osg::Texture2D::LUMINANCE);
        _texture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
        _texture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);

        // the shadow comparison should fail if object is outside the texture
        _texture->setWrap(osg::Texture2D::WRAP_S,osg::Texture2D::CLAMP_TO_BORDER);
        _texture->setWrap(osg::Texture2D::WRAP_T,osg::Texture2D::CLAMP_TO_BORDER);
        _texture->setBorderColor(osg::Vec4(1.0f,1.0f,1.0f,1.0f));
    }
    
    _camera = new osg::Camera;
    { // Setup shadow map camera 
        _camera->setName( "ShadowCamera" );
#if 0  // Absolute reference frame INHERIT_VIEWPOINT works better than this
        _camera->setCullingMode
                ( _camera->getCullingMode() & ~osg::CullSettings::SMALL_FEATURE_CULLING ); 
#endif
        _camera->setReferenceFrame(osg::Camera::ABSOLUTE_RF_INHERIT_VIEWPOINT);
        _camera->setCullCallback(new CameraCullCallback( st ));
        _camera->setClearMask(GL_DEPTH_BUFFER_BIT);

#if 0   // Left in case of some debug testing
        _camera->setClearMask(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        _camera->setClearColor( osg::Vec4(1.0f,1.0f,1.0f,1.0f) );
#endif
        _camera->setComputeNearFarMode(osg::Camera::DO_NOT_COMPUTE_NEAR_FAR);        
        _camera->setViewport(0,0, st->_textureSize.x(), st->_textureSize.y() );
        _camera->setRenderOrder(osg::Camera::PRE_RENDER);        
        _camera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);        
        _camera->attach(osg::Camera::DEPTH_BUFFER, _texture.get());
    }

    _texgen = new osg::TexGen;

    _stateset = new osg::StateSet;
    { // Create and add fake texture for use with nodes without any texture
        osg::Image * image = new osg::Image;
        image->allocateImage( 1, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE );
        *(osg::Vec4ub*)image->data() = osg::Vec4ub( 0xFF, 0xFF, 0xFF, 0xFF );
        
        osg::Texture2D* fakeTex = new osg::Texture2D( image );
        fakeTex->setWrap(osg::Texture2D::WRAP_S,osg::Texture2D::REPEAT);
        fakeTex->setWrap(osg::Texture2D::WRAP_T,osg::Texture2D::REPEAT);
        fakeTex->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::NEAREST);
        fakeTex->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::NEAREST);
        
        _stateset->setTextureAttribute(st->_baseTextureUnit,fakeTex,osg::StateAttribute::ON);
        _stateset->setTextureMode(st->_baseTextureUnit,GL_TEXTURE_1D,osg::StateAttribute::OFF);
        _stateset->setTextureMode(st->_baseTextureUnit,GL_TEXTURE_2D,osg::StateAttribute::ON);
        _stateset->setTextureMode(st->_baseTextureUnit,GL_TEXTURE_3D,osg::StateAttribute::OFF);
    }

    { // Add shadow texture 
        _stateset->setTextureAttributeAndModes(st->_shadowTextureUnit,_texture.get(),osg::StateAttribute::ON);
        _stateset->setTextureMode(st->_shadowTextureUnit,GL_TEXTURE_GEN_S,osg::StateAttribute::ON);
        _stateset->setTextureMode(st->_shadowTextureUnit,GL_TEXTURE_GEN_T,osg::StateAttribute::ON);
        _stateset->setTextureMode(st->_shadowTextureUnit,GL_TEXTURE_GEN_R,osg::StateAttribute::ON);
        _stateset->setTextureMode(st->_shadowTextureUnit,GL_TEXTURE_GEN_Q,osg::StateAttribute::ON);
    }

    {  // Setup shaders used in shadow casting        
        osg::Program * program = new osg::Program();
        _stateset->setAttribute( program );

        if( st->_shadowFragmentShader.valid() )
            program->addShader( st->_shadowFragmentShader.get() );

        if( st->_mainFragmentShader.valid() )
            program->addShader( st->_mainFragmentShader.get() );

        if( st->_shadowVertexShader.valid() )
            program->addShader( st->_shadowVertexShader.get() );

        if( st->_mainVertexShader.valid() )
            program->addShader( st->_mainVertexShader.get() );

        _stateset->addUniform
            ( new osg::Uniform( "baseTexture", int( st->_baseTextureUnit ) ) );
        _stateset->addUniform
            ( new osg::Uniform( "shadowTexture", int( st->_shadowTextureUnit ) ) );
    }

    { // Setup states used for shadow map generation
        osg::StateSet * stateset = _camera->getOrCreateStateSet();
     
        stateset->setAttribute( 
            new osg::PolygonOffset( st->_polygonOffsetFactor, st->_polygonOffsetUnits ),
                osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );

        stateset->setMode( GL_POLYGON_OFFSET_FILL,
              osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );

        // agressive optimization
        stateset->setRenderBinDetails( 0, "RenderBin", 
                            osg::StateSet::OVERRIDE_RENDERBIN_DETAILS );

        // Assure that AlphaTest/AlphaRef works when redirecting all drawables to single bin.
        // If AlphaFunc/AlphaTest is off - transparent objects will cast solid blocky shadows.
        // No override to allow users change this policy in the model if really want solid shadows.
        stateset->setAttributeAndModes
            ( new osg::AlphaFunc( osg::AlphaFunc::GREATER, 0 ), osg::StateAttribute::ON );

        // agressive optimization
        stateset->setAttributeAndModes
            ( new osg::ColorMask( false, false, false, false ),
            osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );

        // note soft (attribute only no mode override) setting. When this works ?
        // 1. for objects prepared for backface culling 
        //    because they usually also set CullFace and CullMode on in their state
        //    For them we override CullFace but CullMode remains set by them
        // 2. For one faced, trees, and similar objects which cannot use 
        //    backface nor front face so they usually use CullMode off set here.
        //    In this case we will draw them in their entirety.

        stateset->setAttribute( new osg::CullFace( osg::CullFace::FRONT ),
              osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );

        // make sure GL_CULL_FACE is off by default 
        // we assume that if object has cull face attribute set to back 
        // it will also set cull face mode ON so no need for override
        stateset->setMode( GL_CULL_FACE, osg::StateAttribute::OFF );

        // optimization attributes 
        osg::Program* program = new osg::Program;
        stateset->setAttribute( program, osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON );
        stateset->setMode
            ( GL_LIGHTING, osg::StateAttribute::OVERRIDE | osg::StateAttribute::OFF );
        stateset->setMode
            ( GL_BLEND, osg::StateAttribute::OVERRIDE | osg::StateAttribute::OFF );

#if 0 // fixed pipeline seems faster (at least on my 7800)
        program->addShader( new osg::Shader( osg::Shader::FRAGMENT,
            "uniform sampler2D texture;                                       \n"
            "void main(void)                                                  \n"
            "{                                                                \n"
            " gl_FragColor = texture2D( texture, gl_TexCoord[0].xy );         \n"
            "}                                                                \n"
        ) ); // program->addShader Fragment

        program->addShader( new osg::Shader( osg::Shader::VERTEX,
            "void main(void)                                                  \n"
            "{                                                                \n"
            "   gl_Position = ftransform();                                   \n"
            "   gl_TexCoord[0] = gl_MultiTexCoord0;                           \n"
            "}                                                                \n"
        ) ); // program->addShader Vertex
#endif

        for( unsigned stage = 1; stage < 4; stage ++ )
        {
            stateset->setTextureMode( stage, GL_TEXTURE_1D, osg::StateAttribute::OVERRIDE | osg::StateAttribute::OFF );
            stateset->setTextureMode( stage, GL_TEXTURE_2D, osg::StateAttribute::OVERRIDE | osg::StateAttribute::OFF );
            stateset->setTextureMode( stage, GL_TEXTURE_3D, osg::StateAttribute::OVERRIDE | osg::StateAttribute::OFF );
        }
    }
}

const osg::Light* StandardShadowMap::ViewData::selectLight
                                ( osg::Vec4 & lightPos, osg::Vec3 & lightDir )
{
    const osg::Light* light = 0;

    //MR testing giving a specific light
    osgUtil::RenderStage * rs = _cv->getRenderStage();

    osgUtil::PositionalStateContainer::AttrMatrixList& aml = 
        rs->getPositionalStateContainer()->getAttrMatrixList();

    osg::RefMatrix* matrix = 0;

    for(osgUtil::PositionalStateContainer::AttrMatrixList::iterator itr = aml.begin();
        itr != aml.end();
        ++itr)
    {
        const osg::Light* found = dynamic_cast<const osg::Light*>(itr->first.get());
        if( found )
        {
            if( _lightPtr->valid() && _lightPtr->get() != found )
                continue; // continue search for the right one

            light = found;
            matrix = itr->second.get();
        }
    }

    if( light ) { // transform light to world space

        osg::Matrix localToWorld = osg::Matrix::inverse( *_cv->getModelViewMatrix() );
        if( matrix ) localToWorld.preMult( *matrix );

        lightPos = light->getPosition();

        if( lightPos[3] == 0 )
            lightDir.set( -lightPos[0], -lightPos[1], -lightPos[2] );
        else 
            lightDir = light->getDirection();            

        lightPos = lightPos * localToWorld;
        lightDir = osg::Matrix::transform3x3( lightDir, localToWorld );
        lightDir.normalize();
    }

    return light;
}

void StandardShadowMap::ViewData::aimShadowCastingCamera( const osg::Light *light,
                                                  const osg::Vec4 &lightPos,
                                                  const osg::Vec3 &lightDir,                                           
                                                  const osg::Vec3 &lightUp
                                        /* by default = osg::Vec3( 0, 1 0 )*/ )
{
#if 0 // less precise but faster 
    osg::BoundingSphere bs =_st->getShadowedScene()->getBound();
#else
    // get the bounds of the model.
    osg::ComputeBoundsVisitor cbbv(osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN);
    cbbv.setTraversalMask(_st->getShadowedScene()->getCastsShadowTraversalMask());
    _st->getShadowedScene()->osg::Group::traverse(cbbv);
    osg::BoundingSphere bs( cbbv.getBoundingBox() );
#endif

    aimShadowCastingCamera
        ( bs, light, lightPos, lightDir, lightUp );
}

void StandardShadowMap::ViewData::aimShadowCastingCamera( 
                                        const osg::BoundingSphere &bs,
                                        const osg::Light *light,
                                        const osg::Vec4 &lightPos,
                                        const osg::Vec3 &lightDir,                                           
                                        const osg::Vec3 &lightUpVector 
                                        /* by default = osg::Vec3( 0, 1 0 )*/ )
{
    osg::Matrixd & view = _camera->getViewMatrix();
    osg::Matrixd & projection = _camera->getProjectionMatrix();

    osg::Vec3 up = lightUpVector;
    if( up.length2() <= 0 )  up.set( 0,1,0 );

    if( light->getSpotCutoff() < 180.0f) // spotlight, no need for bounding box
    {
        osg::Vec3 position(lightPos.x(), lightPos.y(), lightPos.z());
        float spotAngle = light->getSpotCutoff();

        projection.makePerspective( spotAngle, 1.0, 0.1, 1000.0);        
        view.makeLookAt(position,position+lightDir,up);
    }
    else
    {
        if (lightPos[3]!=0.0)   // point light
        {
            osg::Vec3 position(lightPos.x(), lightPos.y(), lightPos.z());

            float centerDistance = (position-bs.center()).length();

            float znear = centerDistance-bs.radius();
            float zfar  = centerDistance+bs.radius();
            float zNearRatio = 0.001f;
            if (znear<zfar*zNearRatio) znear = zfar*zNearRatio;

            float top   = (bs.radius()/centerDistance)*znear;
            float right = top;

            projection.makeFrustum(-right,right,-top,top,znear,zfar);
            view.makeLookAt(position,bs.center(),up );
        }
        else    // directional light
        {
            // make an orthographic projection

            // set the position far away along the light direction
            float radius = bs.radius();
            osg::Vec3 position = bs.center() - lightDir * radius * 2;

            float centerDistance = (position-bs.center()).length();

            float znear = centerDistance-radius;
            float zfar  = centerDistance+radius;
            float zNearRatio = 0.001f;
            if (znear<zfar*zNearRatio) 
                znear = zfar*zNearRatio;

            float top   = radius;
            float right = top;

            projection.makeOrtho(-right, right, -top, top, znear, zfar);
            view.makeLookAt(position,bs.center(),up);
        }
    }
}

void StandardShadowMap::ViewData::cullShadowReceivingScene( )
{
    _cv->pushStateSet( _stateset.get() );

    _st->getShadowedScene()->osg::Group::traverse( *_cv );        

    _cv->popStateSet();
}

void StandardShadowMap::ViewData::cullShadowCastingScene( )
{    
    // record the traversal mask on entry so we can reapply it later.
    unsigned int traversalMask = _cv->getTraversalMask();

    _cv->setTraversalMask( traversalMask & 
         _st->getShadowedScene()->getCastsShadowTraversalMask() );

    // do RTT camera traversal
    _camera->accept(*_cv);

    // reapply the original traversal mask
    _cv->setTraversalMask( traversalMask );
}

void StandardShadowMap::ViewData::addShadowReceivingTexGen( )
{     
     _texgen->setMode(osg::TexGen::EYE_LINEAR);

     // compute the matrix which takes a vertex from view coords into tex coords
     _texgen->setPlanesFromMatrix(
            _camera->getProjectionMatrix() *
            osg::Matrix::translate(1.0,1.0,1.0) *
            osg::Matrix::scale(0.5f,0.5f,0.5f) );

     osg::RefMatrix * refMatrix = new osg::RefMatrix
            ( _camera->getInverseViewMatrix() * *_cv->getModelViewMatrix() );

     _cv->getRenderStage()->getPositionalStateContainer()->
         addPositionedTextureAttribute
            ( *_shadowTextureUnitPtr, refMatrix, _texgen.get() );
}

