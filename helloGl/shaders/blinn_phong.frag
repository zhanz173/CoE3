#version 330 core
bool blinn = true;
vec3 lightColor = vec3(0.5);

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
} fs_in;

uniform sampler2D diffuse1;
uniform sampler2D shadowMap;
uniform vec4 lightPos;
uniform vec3 viewPos;


float Shadow(float bias){

    bias = max(0.05 * (1.0 - bias), 0.005);
    float shadow = 0.0f;
    vec3 pos = fs_in.FragPosLightSpace.xyz * 0.5 + 0.5;
    float depth = texture(shadowMap, pos.xy).r + bias;

    if(pos.z > 1.0)
        shadow = 1.0;
    else 
        shadow = depth < pos.z ? 0.0 : 1.0;

    return shadow;
}


void main(){
    vec3 color = texture(diffuse1, fs_in.TexCoords).rgb;
    vec3 ambient = 0.5*color;
    vec3 lightDir;

    //diffuse light
    if(lightPos.z == 0.0f)
        lightDir = normalize(-lightPos.xyz);
    else 
        lightDir = normalize(lightPos.xyz - fs_in.FragPos);
    vec3 norm = normalize(fs_in.Normal);
    float diff = max(dot(norm, lightDir), 0.0f);
    vec3 diffuse = diff * lightColor;
    
    
    //specular
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = 0.0;    
    if(blinn)
    {
        vec3 halfwayDir = normalize(lightDir + viewDir);  
        spec = pow(max(dot(norm, halfwayDir), 0.0), 32.0);
    }
    else
    {
        spec = pow(max(dot(viewDir, reflectDir), 0.0), 8.0);
    }

    //shadow
    float shadow = Shadow(diff);       
    vec3 lighting = (ambient +  shadow * (diffuse + spec)) * color ;   

    gl_FragColor =  vec4(lighting, 1.0);
}