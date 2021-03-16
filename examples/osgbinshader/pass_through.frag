#version 430
struct LightSource
{
	vec3 position;
	vec3 color;
};

layout(location = 0) in vec4 fragPos;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec4 color;

layout(location = 0) uniform mat4 osg_ModelViewProjectionMatrix;
layout(location = 1) uniform mat3 osg_NormalMatrix;
layout(location = 2) uniform mat4 NormalMatrix;
layout(location = 3) uniform LightSource lightsource; // consumes 2 locations

layout(location = 0) out vec4 FragColor;
void main(void)
{
	FragColor = color;
}