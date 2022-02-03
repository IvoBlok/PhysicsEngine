#version 450 core
out vec4 fragColor;

in flat int objectInfoIndex2;

struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
};  
struct ObjectInfo_t {
    mat4 geometryMatrix;
    vec4 color;
    vec4 indices;
};

layout (std140, binding=0) buffer allObjectInfo
{ 
  ObjectInfo_t objectInfo[];
};

uniform DirLight dirLight;

in vec3 gNormal;

// function declarations
vec3 calcDirLight(DirLight light, vec3 normal, vec3 baseColor);

void main()
{
    vec3 result = calcDirLight(dirLight, gNormal, objectInfo[objectInfoIndex2].color.xyz);

    fragColor = vec4(result, 1.0);
//    if (objectInfoIndex2 != 0) {
//        fragColor = vec4(1.0);
//    } else {
//        fragColor = vec4(0.0, 0.0, 0.0, 1.0);
//    }

    //vec3 modifiedNormal = vec3(max(gNormal.x, 0.0), max(gNormal.y, 0.0), max(gNormal.z, 0.0));
}

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
