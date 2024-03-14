#version 410 core

in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoords;

out vec4 fColor;

//matrices
uniform mat4 model;
uniform mat4 view;
uniform mat3 normalMatrix;
//lighting
uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 PointPosition;
uniform vec3 lightPointColor;
// textures
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform float fogDensity;

//components
vec3 ambient;
float ambientStrength = 0.2f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.5f;

 float constant = 1.0f;
    float linear = 0.09f;
    float quadratic = 0.032f;

vec3 specularS;
vec3 diffuseS;
vec3 ambientS;

void computeDirLight()
{
    //compute eye space coordinates
    vec4 fPosEye = view * model * vec4(fPosition, 1.0f);
    vec3 normalEye = normalize(normalMatrix * fNormal);

    //normalize light direction
    vec3 lightDirN = vec3(normalize(view * vec4(lightDir, 0.0f)));

    //compute view direction in eye coordinates, the viewer is situated at the origin
    vec3 viewDir = normalize(- fPosEye.xyz);

    //compute ambient light
    ambient = ambientStrength * lightColor;

    //compute diffuse light
    diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;

    //compute specular light
    vec3 reflectDir = reflect(-lightDirN, normalEye);
    float specCoeff = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
    specular = specularStrength * specCoeff * lightColor;

    ambientS += ambient;
    diffuseS += diffuse;
    specularS += specular;  
}

void computePointLight()
{
    //compute eye space coordinates
    vec4 fPosEye = view * model * vec4(fPosition, 1.0f);
    vec3 normalEye = normalize(normalMatrix * fNormal);

    //normalize light direction
    vec3 lightDirN = vec3(normalize(view * vec4(lightDir, 0.0f)));

    //compute view direction in eye coordinates, the viewer is situated at the origin
    vec3 viewDir = normalize(- fPosEye.xyz);

    //compute ambient light
    ambient = ambientStrength * lightPointColor;

    //compute diffuse light
    diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightPointColor;

    //compute specular light
    vec3 reflectDir = reflect(-lightDirN, normalEye);
    float specCoeff = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
    specular = specularStrength * specCoeff * lightPointColor;

    float distance    = length(PointPosition - fPosition);
    float attenuation = 1.0 / (constant + linear * distance + 
    		    quadratic * (distance * distance));  

    ambient  *= attenuation; 
    diffuse  *= attenuation;
    specular *= attenuation;

    ambientS += ambient;
    diffuseS += diffuse;
    specularS += specular;     
}

float computeFog(){
    vec4 fPosEye = view * model * vec4(fPosition, 1.0f);
	float fragmentDistance = length(fPosEye);
	float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));

	return clamp(fogFactor, 0.0f, 1.0f);
}

void main() 
{
    computeDirLight();
    computePointLight();
    //compute final vertex color
    vec3 color = min((ambientS + diffuseS) * texture(diffuseTexture, fTexCoords).rgb + specularS * texture(specularTexture, fTexCoords).rgb, 1.0f);
    
    float fogFactor = computeFog(); 
    vec4 fogColor = vec4(0.5f, 0.5f, 0.5f, 1.0f); 

    fColor = mix(fogColor, vec4(color, 1.0f), fogFactor);
}
