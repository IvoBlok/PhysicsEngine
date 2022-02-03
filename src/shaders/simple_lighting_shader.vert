#version 450 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in float index;
//layout (location = 2) in float objectInfoIndex;

#define MAX_OBJECT_COUNT 1000

struct ObjectInfo_t {
    mat4 geometryMatrix;
    vec4 color;
    vec4 indices;
};

layout (std140, binding=0) buffer allObjectInfo
{ 
  ObjectInfo_t objectInfo[];
};

uniform bool instancing;
out int objectInfoIndex;

void main()
{
    if(instancing){
        gl_Position = objectInfo[gl_InstanceID].geometryMatrix * vec4(aPos, 1.0);
        objectInfoIndex = gl_InstanceID;
        return;
    }
    for(int i=0; i < MAX_OBJECT_COUNT; i++){
        if(gl_VertexID >= objectInfo[i].indices[0] && gl_VertexID <= objectInfo[i].indices[1]) {
            gl_Position = objectInfo[i].geometryMatrix * vec4(aPos, 1.0);
            objectInfoIndex = i;
            return;
        }
    }
}