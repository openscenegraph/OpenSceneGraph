#include <iostream>
#include <osg/Geode>
#include <osg/TexGen>
#include <osg/Texture2D>
#include <osg/MatrixTransform>
#include <osg/BlendFunc>
#include <osgText/Text>
#include <osgDB/ReadFile>

#include "VirtualProgram.h"

using osgCandidate::VirtualProgram;

////////////////////////////////////////////////////////////////////////////////
// Example shaders assume:
// one texture
// one directional light 
// front face lighting
// color material mode not used (its not supported by GLSL anyway)
// diffuse/ambient/emissive/specular factors defined in material structure
// all coords and normal except gl_Position are in view space
////////////////////////////////////////////////////////////////////////////////

char MainVertexShaderSource[] =
"vec4 texture( in vec3 position, in vec3 normal );                          \n" //1
"void lighting( in vec3 position, in vec3 normal );                         \n" //2
"                                                                           \n" //3
"void main ()                                                               \n" //4
"{                                                                          \n" //5
"    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;                \n" //6
"    vec4 position4 = gl_ModelViewMatrix * gl_Vertex;                       \n" //7
"    vec3 position = position4.xyz / position4.w;                           \n" //8
"    vec3 normal = normalize( gl_NormalMatrix * gl_Normal );                \n" //9
"    gl_TexCoord[0] = texture( position, normal );                          \n" //10
"    lighting( position, normal );                                          \n" //11
"}                                                                          \n";//12

char TexCoordTextureVertexShaderSource[] =
"vec4 texture( in vec3 position, in vec3 normal )                           \n" //1
"{                                                                          \n" //2
"    return gl_TextureMatrix[0] * gl_MultiTexCoord0;                        \n" //3
"}                                                                          \n";//4

char SphereMapTextureVertexShaderSource[] =
"vec4 texture( in vec3 position, in vec3 normal )                           \n" //1
"{                                                                          \n" //2
"    vec3 u = normalize( position );                                        \n" //3
"    vec3 r = reflect(u, normal);                                           \n" //4
"    float m = 2.0 * sqrt(r.x * r.x + r.y * r.y + (r.z+1.0) * (r.z+1.0));   \n" //5
"    return vec4(r.x / m + 0.5, r.y / m + 0.5, 1.0, 1.0 );                  \n" //6
"}                                                                          \n";//7

char PerVertexDirectionalLightingVertexShaderSource[] = 
"void lighting( in vec3 position, in vec3 normal )                          \n" //1
"{                                                                          \n" //2
"    float NdotL = dot( normal, normalize(gl_LightSource[0].position.xyz) );\n" //3
"    NdotL = max( 0.0, NdotL );                                             \n" //4
"    float NdotHV = dot( normal, gl_LightSource[0].halfVector.xyz );        \n" //5
"    NdotHV = max( 0.0, NdotHV );                                           \n" //6
"                                                                           \n" //7
"    gl_FrontColor = gl_FrontLightModelProduct.sceneColor +                 \n" //8
"                    gl_FrontLightProduct[0].ambient +                      \n" //9
"                    gl_FrontLightProduct[0].diffuse * NdotL;               \n" //10
"                                                                           \n" //11
"    gl_FrontSecondaryColor = vec4(0.0);                                    \n" //12
"                                                                           \n" //13
"    if ( NdotL * NdotHV > 0.0 )                                            \n" //14
"        gl_FrontSecondaryColor = gl_FrontLightProduct[0].specular *        \n" //15
"                                 pow( NdotHV, gl_FrontMaterial.shininess );\n" //16
"                                                                           \n" //17
"    gl_BackColor = gl_FrontColor;                                          \n" //18
"    gl_BackSecondaryColor = gl_FrontSecondaryColor;                        \n" //19
"}                                                                          \n";//20

char MainFragmentShaderSource[] =
"vec4 texture( void );                                                      \n" //1
"void lighting( inout vec4 color );                                         \n" //2
"                                                                           \n" //3
"void main ()                                                               \n" //4
"{                                                                          \n" //5
"    vec4 color = texture();                                                \n" //6
"    lighting( color );                                                     \n" //7
"    gl_FragColor = color;                                                  \n" //8
"}                                                                          \n";//9

char TextureFragmentShaderSource[] =
"uniform sampler2D baseTexture;                                             \n" //1
"vec4 texture( void )                                                       \n" //2
"{                                                                          \n" //3
"    return texture2D( baseTexture, gl_TexCoord[0].xy );                    \n" //4
"}                                                                          \n";//5

char ProceduralBlueTextureFragmentShaderSource[] =
"vec4 texture( void )                                                       \n" //1
"{                                                                          \n" //2
"    return vec4( 0.3, 0.3, 1.0, 1.0 );                                     \n" //3
"}                                                                          \n";//4

char PerVertexLightingFragmentShaderSource[] =
"void lighting( inout vec4 color )                                          \n" //1
"{                                                                          \n" //2
"    color = color * gl_Color + gl_SecondaryColor;                          \n" //3
"}                                                                          \n";//4

char PerFragmentLightingVertexShaderSource[] =
"varying vec3 Normal;                                                       \n" //1
"varying vec3 Position;                                                     \n" //2
"                                                                           \n" //3
"void lighting( in vec3 position, in vec3 normal )                          \n" //4
"{                                                                          \n" //5
"    Normal = normal;                                                       \n" //6
"    Position = position;                                                   \n" //7
"}                                                                          \n";//8

char PerFragmentDirectionalLightingFragmentShaderSource[] =
"varying vec3 Normal;                                                       \n" //1
"varying vec3 Position; // not used for directional lighting                \n" //2
"                                                                           \n" //3
"void lighting( inout vec4 color )                                          \n" //4
"{                                                                          \n" //5
"    vec3 n = normalize( Normal );                                          \n" //5
"    float NdotL = dot( n, normalize(gl_LightSource[0].position.xyz) );     \n" //6
"    NdotL = max( 0.0, NdotL );                                             \n" //7
"    float NdotHV = dot( n, gl_LightSource[0].halfVector.xyz );             \n" //8
"    NdotHV = max( 0.0, NdotHV );                                           \n" //9
"                                                                           \n" //10
"    color *= gl_FrontLightModelProduct.sceneColor +                        \n" //11
"             gl_FrontLightProduct[0].ambient +                             \n" //12
"             gl_FrontLightProduct[0].diffuse * NdotL;                      \n" //13
"                                                                           \n" //14
"    if ( NdotL * NdotHV > 0.0 )                                            \n" //15
"        color += gl_FrontLightProduct[0].specular *                        \n" //16
"                 pow( NdotHV, gl_FrontMaterial.shininess );                \n" //17
"}                                                                          \n";//18

////////////////////////////////////////////////////////////////////////////////
// Convenience method to simplify code a little ...
void SetVirtualProgramShader( VirtualProgram * virtualProgram,
                              std::string shader_semantics,
                              osg::Shader::Type shader_type,
                              std::string shader_name,
                              std::string shader_source )
{
    osg::Shader * shader = new osg::Shader( shader_type );
    shader->setName( shader_name );
    shader->setShaderSource( shader_source );
    virtualProgram->setShader( shader_semantics, shader );
}
///////////////////////////////////////////////////////////////////////////////
void AddLabel( osg::Group * group, const std::string & label, float offset )
{
    osg::Vec3 center( 0, 0, offset * 0.5 );
    osg::Geode * geode = new osg::Geode;

    // Make sure no program breaks text outputs 
    geode->getOrCreateStateSet()->setAttribute
      ( new osg::Program, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED );

    // Turn off stage 1 texture set in parent transform (otherwise it darkens text)
    geode->getOrCreateStateSet()->setTextureMode( 1, GL_TEXTURE_2D, osg::StateAttribute::OFF );

    group->addChild( geode );

    osgText::Text* text = new osgText::Text;
    geode->addDrawable( text );
    text->setFont("fonts/times.ttf");
    text->setCharacterSize( offset * 0.1 );
    text->setPosition(center);
    text->setAlignment( osgText::TextBase::CENTER_CENTER );
    text->setAxisAlignment(osgText::Text::SCREEN);

    osg::Vec4 characterSizeModeColor(1.0f,0.0f,0.5f,1.0f);
#if 1
    // reproduce outline bounding box compute problem with backdrop on.
    text->setBackdropType(osgText::Text::OUTLINE);
    text->setDrawMode(osgText::Text::TEXT | osgText::Text::BOUNDINGBOX);
#endif

    text->setText( label );
}
////////////////////////////////////////////////////////////////////////////////
osg::Node * CreateAdvancedHierarchy( osg::Node * model )
{
    if( !model ) return NULL;
    float offset = model->getBound().radius() * 1.3; // diameter

    // Create transforms for translated instances of the model
    osg::MatrixTransform * transformCenterMiddle  = new osg::MatrixTransform( );
    transformCenterMiddle->setMatrix( osg::Matrix::translate( 0,0, offset * 0.5 ) );
    transformCenterMiddle->addChild( model );

    osg::MatrixTransform * transformCenterTop  = new osg::MatrixTransform( );
    transformCenterMiddle->addChild( transformCenterTop );
    transformCenterTop->setMatrix( osg::Matrix::translate( 0,0,offset ) );
    transformCenterTop->addChild( model );

    osg::MatrixTransform * transformCenterBottom  = new osg::MatrixTransform( );
    transformCenterMiddle->addChild( transformCenterBottom );
    transformCenterBottom->setMatrix( osg::Matrix::translate( 0,0,-offset ) );
    transformCenterBottom->addChild( model );

    osg::MatrixTransform * transformLeftBottom  = new osg::MatrixTransform( );
    transformCenterBottom->addChild( transformLeftBottom );
    transformLeftBottom->setMatrix( osg::Matrix::translate( -offset * 0.8,0, -offset * 0.8 ) );
    transformLeftBottom->addChild( model );

    osg::MatrixTransform * transformRightBottom  = new osg::MatrixTransform( );
    transformCenterBottom->addChild( transformRightBottom );
    transformRightBottom->setMatrix( osg::Matrix::translate( offset * 0.8,0, -offset * 0.8 ) );
    transformRightBottom->addChild( model );

    // Set default VirtualProgram in root StateSet 
    // With main vertex and main fragment shaders calling 
    // lighting and texture functions defined in aditional shaders
    // Lighting is done per vertex using simple directional light
    // Texture uses stage 0 TexCoords and TexMap

    if( 1 ) 
    {
        // NOTE:
        // duplicating the same semantics name in virtual program 
        // is only possible if its used for shaders of differing types
        // here for VERTEX and FRAGMENT

        VirtualProgram * vp = new VirtualProgram( );
        transformCenterMiddle->getOrCreateStateSet()->setAttribute( vp );
        AddLabel( transformCenterMiddle, "Per Vertex Lighting Virtual Program", offset );

        SetVirtualProgramShader( vp, "main", osg::Shader::VERTEX,
            "Vertex Main", MainVertexShaderSource );

        SetVirtualProgramShader( vp, "main", osg::Shader::FRAGMENT,
            "Fragment Main", MainFragmentShaderSource );

        SetVirtualProgramShader( vp, "texture",osg::Shader::VERTEX,
            "Vertex Texture Coord 0", TexCoordTextureVertexShaderSource );

        SetVirtualProgramShader( vp, "texture",osg::Shader::FRAGMENT,
            "Fragment Texture", TextureFragmentShaderSource );

        SetVirtualProgramShader( vp, "lighting",osg::Shader::VERTEX,
            "Vertex Lighting", PerVertexDirectionalLightingVertexShaderSource );

        SetVirtualProgramShader( vp, "lighting",osg::Shader::FRAGMENT,
            "Fragment Lighting", PerVertexLightingFragmentShaderSource );

        transformCenterMiddle->getOrCreateStateSet()->
            addUniform( new osg::Uniform( "baseTexture", 0 ) );

    }

    // Override default vertex ligting with pixel lighting shaders
    // For three bottom models
    if( 1 ) 
    {
        AddLabel( transformCenterBottom, "Per Pixel Lighting VP", offset );
        VirtualProgram * vp = new VirtualProgram( );
        transformCenterBottom->getOrCreateStateSet()->setAttribute( vp );

        SetVirtualProgramShader( vp, "lighting",osg::Shader::VERTEX,
            "Vertex Shader For Per Pixel Lighting", 
            PerFragmentLightingVertexShaderSource );

        SetVirtualProgramShader( vp, "lighting",osg::Shader::FRAGMENT,
            "Fragment Shader For Per Pixel Lighting", 
            PerFragmentDirectionalLightingFragmentShaderSource );
    }

    // Additionaly set bottom left model texture to procedural blue to 
    // better observe smooth speculars done through per pixel lighting
    if( 1 ) 
    {
        AddLabel( transformLeftBottom, "Blue Tex VP", offset );
        VirtualProgram * vp = new VirtualProgram( );
        transformLeftBottom->getOrCreateStateSet()->setAttribute( vp );

        SetVirtualProgramShader( vp, "texture",osg::Shader::FRAGMENT,
            "Fragment Shader Procedural Blue Tex", 
            ProceduralBlueTextureFragmentShaderSource );
    }

    // Additionaly change texture mapping to SphereMAp in bottom right model 
    if( 1 )
    {
        AddLabel( transformRightBottom, "EnvMap Sphere VP", offset );

        osg::StateSet * ss = transformRightBottom->getOrCreateStateSet();
        VirtualProgram * vp = new VirtualProgram( );
        ss->setAttribute( vp );
        SetVirtualProgramShader( vp, "texture",osg::Shader::VERTEX,
            "Vertex Texture Sphere Map", SphereMapTextureVertexShaderSource );

        osg::Texture2D * texture =  new osg::Texture2D(
//            osgDB::readImageFile("Images/reflect.rgb") 
            osgDB::readImageFile("Images/skymap.jpg") 
        );

        // Texture is set on stage 1 to not interfere with label text
        // The same could be achieved with texture override 
        // but such approach also turns off label texture
        ss->setTextureAttributeAndModes( 1, texture, osg::StateAttribute::ON );
        ss->addUniform( new osg::Uniform( "baseTexture", 1 ) );

#if 0 // Could be useful with Fixed Vertex Pipeline
        osg::TexGen * texGen = new osg::TexGen();
        texGen->setMode( osg::TexGen::SPHERE_MAP );

        // Texture states applied
        ss->setTextureAttributeAndModes( 1, texGen, osg::StateAttribute::ON );
#endif

    }


    // Top center model usues osg::Program overriding VirtualProgram in model
    if( 1 ) 
    {
        AddLabel( transformCenterTop, "Fixed Vertex + Simple Fragment osg::Program", offset );
        osg::Program * program = new osg::Program;
        program->setName( "Trivial Fragment + Fixed Vertex Program" );

        transformCenterTop->getOrCreateStateSet( )->setAttributeAndModes
            ( program, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );

        osg::Shader * shader = new osg::Shader( osg::Shader::FRAGMENT );
        shader->setName( "Trivial Fragment Shader" );
        shader->setShaderSource(
            "uniform sampler2D baseTexture;                                          \n"
            "void main(void)                                                         \n"
            "{                                                                       \n"
            "    gl_FragColor = gl_Color * texture2D( baseTexture,gl_TexCoord[0].xy);\n"
            "}                                                                       \n"
            );

        program->addShader( shader );
    }

    return transformCenterMiddle;
}

////////////////////////////////////////////////////////////////////////////////
// Shders not used in the example but left for fun if anyone wants to play 
char LightingVertexShaderSource[] = 
"// Forward declarations                                                    \n" //1
"                                                                           \n" //2
"void SpotLight( in int i, in vec3 eye, in vec3 position, in vec3 normal,   \n" //3
"            inout vec4 ambient, inout vec4 diffuse, inout vec4 specular ); \n" //4
"                                                                           \n" //5
"void PointLight( in int i, in vec3 eye, in vec3 position, in vec3 normal,  \n" //6
"            inout vec4 ambient, inout vec4 diffuse, inout vec4 specular ); \n" //7
"                                                                           \n" //8
"void DirectionalLight( in int i, in vec3 normal,                           \n" //9
"            inout vec4 ambient, inout vec4 diffuse, inout vec4 specular ); \n" //10
"                                                                           \n" //11
"const int NumEnabledLights = 1;                                            \n" //12
"                                                                           \n" //13
"void lighting( in vec3 position, in vec3 normal )                          \n" //14
"{                                                                          \n" //15
"    vec3  eye = vec3( 0.0, 0.0, 1.0 );                                     \n" //16
"    //vec3  eye = -normalize(position);                                    \n" //17
"                                                                           \n" //18
"    // Clear the light intensity accumulators                              \n" //19
"    vec4 amb  = vec4(0.0);                                                 \n" //20
"    vec4 diff = vec4(0.0);                                                 \n" //21
"    vec4 spec = vec4(0.0);                                                 \n" //22
"                                                                           \n" //23
"    // Loop through enabled lights, compute contribution from each         \n" //24
"    for (int i = 0; i < NumEnabledLights; i++)                             \n" //25
"   {                                                                       \n" //26
"       if (gl_LightSource[i].position.w == 0.0)                            \n" //27
"           DirectionalLight(i, normal, amb, diff, spec);                   \n" //28
"       else if (gl_LightSource[i].spotCutoff == 180.0)                     \n" //29
"           PointLight(i, eye, position, normal, amb, diff, spec);          \n" //30
"       else                                                                \n" //31
"           SpotLight(i, eye, position, normal, amb, diff, spec);           \n" //32
"    }                                                                      \n" //33
"                                                                           \n" //34
"    gl_FrontColor = gl_FrontLightModelProduct.sceneColor +                 \n" //35
"                    amb * gl_FrontMaterial.ambient +                       \n" //36
"                    diff * gl_FrontMaterial.diffuse;                       \n" //37
"                                                                           \n" //38
"    gl_FrontSecondaryColor = vec4(spec*gl_FrontMaterial.specular);         \n" //39
"                                                                           \n" //40
"    gl_BackColor = gl_FrontColor;                                          \n" //41
"    gl_BackSecondaryColor = gl_FrontSecondaryColor;                        \n" //42
"}                                                                          \n";//43

char SpotLightShaderSource[] = 
"void SpotLight(in int i,                                                   \n" //1
"               in vec3 eye,                                                \n" //2
"               in vec3 position,                                           \n" //3
"               in vec3 normal,                                             \n" //4
"               inout vec4 ambient,                                         \n" //5
"               inout vec4 diffuse,                                         \n" //6
"               inout vec4 specular)                                        \n" //7
"{                                                                          \n" //8
"    float nDotVP;          // normal . light direction                     \n" //9
"    float nDotHV;          // normal . light half vector                   \n" //10
"    float pf;              // power factor                                 \n" //11
"    float spotDot;         // cosine of angle between spotlight            \n" //12
"    float spotAttenuation; // spotlight attenuation factor                 \n" //13
"    float attenuation;     // computed attenuation factor                  \n" //14
"    float d;               // distance from surface to light source        \n" //15
"    vec3 VP;               // direction from surface to light position     \n" //16
"    vec3 halfVector;       // direction of maximum highlights              \n" //17
"                                                                           \n" //18
"    // Compute vector from surface to light position                       \n" //19
"    VP = vec3(gl_LightSource[i].position) - position;                      \n" //20
"                                                                           \n" //21
"    // Compute distance between surface and light position                 \n" //22
"    d = length(VP);                                                        \n" //23
"                                                                           \n" //24
"    // Normalize the vector from surface to light position                 \n" //25
"    VP = normalize(VP);                                                    \n" //26
"                                                                           \n" //27
"    // Compute attenuation                                                 \n" //28
"    attenuation = 1.0 / (gl_LightSource[i].constantAttenuation +           \n" //29
"                         gl_LightSource[i].linearAttenuation * d +         \n" //30
"                         gl_LightSource[i].quadraticAttenuation *d*d);     \n" //31
"                                                                           \n" //32
"    // See if point on surface is inside cone of illumination              \n" //33
"    spotDot = dot(-VP, normalize(gl_LightSource[i].spotDirection));        \n" //34
"                                                                           \n" //35
"    if (spotDot < gl_LightSource[i].spotCosCutoff)                         \n" //36
"        spotAttenuation = 0.0; // light adds no contribution               \n" //37
"    else                                                                   \n" //38
"        spotAttenuation = pow(spotDot, gl_LightSource[i].spotExponent);    \n" //39
"                                                                           \n" //40
"    // Combine the spotlight and distance attenuation.                     \n" //41
"    attenuation *= spotAttenuation;                                        \n" //42
"                                                                           \n" //43
"    halfVector = normalize(VP + eye);                                      \n" //44
"                                                                           \n" //45
"    nDotVP = max(0.0, dot(normal, VP));                                    \n" //46
"    nDotHV = max(0.0, dot(normal, halfVector));                            \n" //47
"                                                                           \n" //48
"    if (nDotVP == 0.0)                                                     \n" //49
"        pf = 0.0;                                                          \n" //50
"    else                                                                   \n" //51
"        pf = pow(nDotHV, gl_FrontMaterial.shininess);                      \n" //52
"                                                                           \n" //53
"    ambient  += gl_LightSource[i].ambient * attenuation;                   \n" //54
"    diffuse  += gl_LightSource[i].diffuse * nDotVP * attenuation;          \n" //55
"    specular += gl_LightSource[i].specular * pf * attenuation;             \n" //56
"}                                                                          \n";//57

char PointLightShaderSource[] = 
"void PointLight(in int i,                                                  \n" //1
"                in vec3 eye,                                               \n" //2
"                in vec3 position,                                          \n" //3
"                in vec3 normal,                                            \n" //4
"                inout vec4 ambient,                                        \n" //5
"                inout vec4 diffuse,                                        \n" //6
"                inout vec4 specular)                                       \n" //7
"{                                                                          \n" //8
"    float nDotVP;      // normal . light direction                         \n" //9
"    float nDotHV;      // normal . light half vector                       \n" //10
"    float pf;          // power factor                                     \n" //11
"    float attenuation; // computed attenuation factor                      \n" //12
"    float d;           // distance from surface to light source            \n" //13
"    vec3  VP;          // direction from surface to light position         \n" //14
"    vec3  halfVector;  // direction of maximum highlights                  \n" //15
"                                                                           \n" //16
"    // Compute vector from surface to light position                       \n" //17
"    VP = vec3(gl_LightSource[i].position) - position;                      \n" //18
"                                                                           \n" //19
"    // Compute distance between surface and light position                 \n" //20
"    d = length(VP);                                                        \n" //21
"                                                                           \n" //22
"    // Normalize the vector from surface to light position                 \n" //23
"    VP = normalize(VP);                                                    \n" //24
"                                                                           \n" //25
"    // Compute attenuation                                                 \n" //26
"    attenuation = 1.0 / (gl_LightSource[i].constantAttenuation +           \n" //27
"                         gl_LightSource[i].linearAttenuation * d +         \n" //28
"                         gl_LightSource[i].quadraticAttenuation * d*d);    \n" //29
"                                                                           \n" //30
"    halfVector = normalize(VP + eye);                                      \n" //31
"                                                                           \n" //32
"    nDotVP = max(0.0, dot(normal, VP));                                    \n" //33
"    nDotHV = max(0.0, dot(normal, halfVector));                            \n" //34
"                                                                           \n" //35
"    if (nDotVP == 0.0)                                                     \n" //36
"        pf = 0.0;                                                          \n" //37
"    else                                                                   \n" //38
"        pf = pow(nDotHV, gl_FrontMaterial.shininess);                      \n" //39
"                                                                           \n" //40
"    ambient += gl_LightSource[i].ambient * attenuation;                    \n" //41
"    diffuse += gl_LightSource[i].diffuse * nDotVP * attenuation;           \n" //42
"    specular += gl_LightSource[i].specular * pf * attenuation;             \n" //43
"}                                                                          \n";//44

char DirectionalLightShaderSource[] =
"void DirectionalLight(in int i,                                            \n" //1
"                      in vec3 normal,                                      \n" //2
"                      inout vec4 ambient,                                  \n" //3
"                      inout vec4 diffuse,                                  \n" //4
"                      inout vec4 specular)                                 \n" //5
"{                                                                          \n" //6
"     float nDotVP;         // normal . light direction                     \n" //7
"     float nDotHV;         // normal . light half vector                   \n" //8
"     float pf;             // power factor                                 \n" //9
"                                                                           \n" //10
"     nDotVP = max(0.0, dot(normal,                                         \n" //11
"                normalize(vec3(gl_LightSource[i].position))));             \n" //12
"     nDotHV = max(0.0, dot(normal,                                         \n" //13
"                      vec3(gl_LightSource[i].halfVector)));                \n" //14
"                                                                           \n" //15
"     if (nDotVP == 0.0)                                                    \n" //16
"         pf = 0.0;                                                         \n" //17
"     else                                                                  \n" //18
"         pf = pow(nDotHV, gl_FrontMaterial.shininess);                     \n" //19
"                                                                           \n" //20
"     ambient  += gl_LightSource[i].ambient;                                \n" //21
"     diffuse  += gl_LightSource[i].diffuse * nDotVP;                       \n" //22
"     specular += gl_LightSource[i].specular * pf;                          \n" //23
"}                                                                          \n";//24

