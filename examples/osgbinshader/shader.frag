#version 430
struct LightSource
{
	vec3 position;
	vec3 color;
};

layout(location = 0) in vec4 fragPos;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec4 color;
layout(location = 3) in vec2 fragTexCoords;

uniform sampler2D baseTexture;
layout(location = 0) uniform mat4 osg_ModelViewProjectionMatrix;
layout(location = 1) uniform mat3 osg_NormalMatrix;
layout(location = 3) uniform mat4 NormalMatrix;
layout(location = 4) uniform int numOfLightSources;
layout(location = 5) uniform mat4 osg_ViewMatrix;
layout(location = 6) uniform mat4 ModelMatrix;
layout(location = 7) uniform LightSource[2] lightSource; // consumes 2 * 2 locations
layout(location = 11) uniform mat4 osg_ViewMatrixInverse;

layout(location = 0) out vec4 FragColor;


void main(void)
{
	
	float ambientStrength = 0.001;
	float diffuseStrength = 0.9f;
	float specularStrength = 3.2;

	// specular					
	// Convert camera position from Camera (eye) space (its always at 0, 0, 0, 1
	// in there) to World space. Don't forget to use mat4 and vec4!
	vec4 viewPos = osg_ViewMatrixInverse * vec4(0, 0, 0, 1);
	vec3 viewDir  = normalize(viewPos.xyz - fragPos.xyz);

	for (int i = 0; i <= numOfLightSources - 1; i++)
	{
		// ambient		
		vec3 ambient = ambientStrength * texture(baseTexture, fragTexCoords).rgb;
		
		// diffuse					
		vec4 norm = normalize(NormalMatrix * vec4(fragNormal, 1.0f));
		vec3 lightDir = normalize(lightSource[i].position - fragPos.xyz);
		float diff = max(dot(norm.xyz, lightDir), 0.0);
		vec3 diffuse = diffuseStrength * diff * texture(baseTexture, fragTexCoords).rgb;

		
		// specular
        vec3 halfwayDir = normalize(lightDir + viewDir);  
        float spec = pow(max(dot(norm.xyz, halfwayDir), 0.0), 8.0);
        vec3 specular = specularStrength * lightSource[i].color * spec;

		vec3 finalColor = (ambient + diffuse + specular) * lightSource[i].color;
		FragColor += vec4(finalColor, 1.0f);

	}
}