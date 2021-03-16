#version 430

struct LightSource
{
	vec3 position;
	vec3 color;
};

layout(location = 0) in vec4 osg_Vertex;
layout(location = 1) in vec3 osg_Normal;
layout(location = 2) in vec3 osg_Color;
layout(location = 3) in vec4 osg_MultiTexCoord0;
layout(location = 4) in vec4 osg_MultiTexCoord1;
layout(location = 5) in vec4 osg_MultiTexCoord2;
layout(location = 6) in vec4 osg_MultiTexCoord3;
layout(location = 7) in vec4 osg_MultiTexCoord4;
layout(location = 8) in vec4 osg_MultiTexCoord5;
layout(location = 9) in vec4 osg_MultiTexCoord6;
layout(location = 10) in vec4 osg_MultiTexCoord7;

layout(location = 0) out vec4 fragPos;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec4 color;
layout(location = 3) out vec2 fragTexCoords;

uniform sampler2D baseTexture;
layout(location = 0) uniform mat4 osg_ModelViewProjectionMatrix;
layout(location = 1) uniform mat3 osg_NormalMatrix;
layout(location = 3) uniform mat4 NormalMatrix;
layout(location = 4) uniform int numOfLightSources;
layout(location = 5) uniform mat4 osg_ViewMatrix;
layout(location = 6) uniform mat4 ModelMatrix;
layout(location = 7) uniform LightSource[2] lightSource; // consumes 2 * 2 locations
layout(location = 11) uniform mat4 osg_ViewMatrixInverse;

void main(void)
{
	fragNormal = osg_Normal;
	fragPos = vec4(vec3(ModelMatrix * osg_Vertex), 1.0);
	color = vec4(osg_Color, 1.0f);
	fragTexCoords = osg_MultiTexCoord0.xy;

	gl_Position = osg_ModelViewProjectionMatrix * osg_Vertex;
}