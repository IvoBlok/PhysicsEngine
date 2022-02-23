// Geometry Shader
#version 430

// in / out
// -----------
layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in int objectInfoIndexList[];
out flat int objectInfoIndex2;

out vec3 gNormal;

// buffers
// -----------
layout (std140) uniform Matrices
{
    mat4 projection;
    mat4 view;
};

// main
// -----------
void main()
{
    vec3 side1 = (gl_in[2].gl_Position - gl_in[0].gl_Position).xyz;
    vec3 side2 = (gl_in[1].gl_Position - gl_in[0].gl_Position).xyz;
    vec3 diff = side2 - side1;
    vec3 facetNormal = normalize(cross(side1, side2));

    for(int i = 0; i < 3; i++) {
        gNormal = facetNormal;
        objectInfoIndex2 = objectInfoIndexList[i];
        gl_Position = projection * view * gl_in[i].gl_Position;
        EmitVertex();   
    }
    EndPrimitive();
}