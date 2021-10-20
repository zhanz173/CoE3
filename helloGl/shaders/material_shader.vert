#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;
out float attenuation;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

struct Light {
    vec4 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;

    float cutOff;
    float outerCutOff;
};

uniform Light light;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));;
    Normal = mat3(transpose(inverse(model))) * aNormal;

    vec4 light_eye = view * light.direction;
    vec4 vertex_eye = view * vec4(FragPos, 1.0);

    gl_Position = projection * vertex_eye;
    TexCoords = aTexCoords;

    float distance = length(light_eye.xyz - vertex_eye.xyz);
    attenuation = 1.0 / (light.constant + light.linear * distance + 
    		    light.quadratic * (distance * distance));   
}
