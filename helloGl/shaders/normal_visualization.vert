#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

layout (std140) uniform Matrices
{
    mat4 projection; //0
    mat4 model_view;  //64
    mat4 inv_model_view;  //128
};

out VS_OUT {
    vec4 normal;
} vs_out;


void main()
{
    gl_Position = model_view * vec4(aPos, 1.0);
    vs_out.normal = inv_model_view * vec4(aNormal, 0.0);
}