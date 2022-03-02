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
const int MAX_OBJECTS = 100;
uniform int[MAX_OBJECTS] vertexLimits;

// buffers
// -----------
layout (std140, binding=0) buffer allObjectInfo
{ 
  ObjectInfo_t objectInfo[];
};

// main
// -----------
void main()
{
    int objectInfoIndex;
    for (int i = 0; i < MAX_OBJECTS - 1; i++) {
        if (gl_VertexID < vertexLimits[i + 1] || vertexLimits[i + 1] == -1) {
            objectInfoIndex = i;
            break;
        }
    }

    gl_Position = objectInfo[objectInfoIndex].geometryMatrix * vec4(aPos, 1.0);
    objectInfoIndexList = objectInfoIndex;
}