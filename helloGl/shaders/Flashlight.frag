#version 330 core
#define precision 0.0001f

out vec4 FragColor;
in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;
in float attenuation;

struct Material {
    sampler2D diffuse;  //diffuse + ambient
    sampler2D specular;
    float     shininess;
};

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

uniform vec3 viewPos;
uniform Material material;
uniform Light light;

void main()
{
    // ambient
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));

    vec3 lightDir = normalize(viewPos - FragPos);
      
    vec3 norm = normalize(Normal);
    float diff = max(dot(norm, lightDir), 0.0f);
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
      
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0f), material.shininess);
    vec3 specular_tex = vec3(texture(material.specular, TexCoords));
    vec3 specular = light.specular * spec * specular_tex;   

    //intensity
    float theta     = dot(lightDir, normalize(-light.direction.xyz));
    float epsilon   = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);   

    FragColor = vec4(ambient + (diffuse + specular) * attenuation * intensity, 1.0);

}