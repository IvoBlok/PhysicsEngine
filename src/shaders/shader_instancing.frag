#version 430 core

// structs
// -----------
struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
};  
struct ObjectInfo_t {
    mat4 geometryMatrix;
    vec4 color;
};

// in / out
// -----------
out vec4 fragColor;

in flat int objectInfoIndex2;
in vec3 gNormal;

// buffers
// -----------
layout (std140, binding=0) buffer allObjectInfo
{ 
  ObjectInfo_t objectInfo[];
};

uniform DirLight dirLight;

// function declarations
// -----------
vec3 calcDirLight(DirLight light, vec3 normal, vec3 baseColor)
{
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // combine results
    vec3 ambient  = light.ambient  * baseColor;
    vec3 diffuse  = light.diffuse  * diff * baseColor;
    return (ambient + diffuse);
}  

// main
// -----------
void main()
{
    vec3 result = calcDirLight(dirLight, gNormal, objectInfo[objectInfoIndex2].color.xyz);

    fragColor = vec4(result, 1.0);
}

