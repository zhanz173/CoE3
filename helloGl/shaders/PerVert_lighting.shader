#shader vertex
#version 330 core

#define gl_MaxLights 1

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform gl_LightSourceParameters gl_LightSource[gl_MaxLights];

void main() {
	vec3 normal, lightDir;
	vec4 diffuse;
	float NdotL;

	FragPos = vec3(model * vec4(aPos, 1.0))
	normal = normalize(mat3(transpose(inverse(model))) * aNormal);
	lightDir = normalize(vec3(gl_LightSource[0].position));

	//light angle
	NdotL = max(dot(normal, lightDir), 0.0);

	// diffuse
	diffuse = 0.5 * gl_LightSource[0].diffuse;
	gl_FrontColor = NdotL * diffuse;

	// ambient
	ambient = gl_FrontMaterial.ambient * gl_LightSource[0].ambient;
	globalAmbient = gl_LightModel.ambient * gl_FrontMaterial.ambient;
	gl_FrontColor = NdotL * diffuse + globalAmbient + ambient;

	gl_Position = projection * view * vec4(FragPos, 1.0);
}


#shader fragment

#version 330 core
out vec4 FragColor;


void main()
{
	gl_FragColor = gl_Color;
}