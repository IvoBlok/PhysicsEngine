#version 430 core
// structs
// -----------
struct ObjectInfo_t {
    mat4 geometryMatrix;
    vec4 color;
};

// vertex input
// -----------
layout (location = 0) in vec3 aPos;

// in / out
// -----------
out int objectInfoIndexList; //1 value, but it goes to the geometry shader, so it's called a list because in the geom shader it will be

// buffers
// -----------
layout (std140, binding=1) buffer allObjectInfo
{ 
  ObjectInfo_t objectInfo[];
};

// main
// -----------
void main()
{
    gl_Position = objectInfo[gl_InstanceID].geometryMatrix * vec4(aPos, 1.0);
    objectInfoIndexList = gl_InstanceID;
    return;
}