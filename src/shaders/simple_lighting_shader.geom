// Geometry Shader
#version 430

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in int objectInfoIndex[];
out flat int objectInfoIndex2;

out vec3 gNormal;

layout (std140) uniform Matrices
{
    mat4 projection;
    mat4 view;
};

void main()
{
    vec3 side1 = (gl_in[2].gl_Position - gl_in[0].gl_Position).xyz;
    vec3 side2 = (gl_in[1].gl_Position - gl_in[0].gl_Position).xyz;
    vec3 diff = side2 - side1;
    vec3 facetNormal = normalize(cross(side1, side2));

    gNormal = facetNormal;
    objectInfoIndex2 = objectInfoIndex[0];
    gl_Position = projection * view * gl_in[0].gl_Position;
    EmitVertex();

    gNormal = facetNormal;
    objectInfoIndex2 = objectInfoIndex[0];
    gl_Position = projection * view * gl_in[1].gl_Position;
    EmitVertex();

    gNormal = facetNormal;
    objectInfoIndex2 = objectInfoIndex[0];
    gl_Position = projection * view * gl_in[2].gl_Position;
    EmitVertex();

    EndPrimitive();
}