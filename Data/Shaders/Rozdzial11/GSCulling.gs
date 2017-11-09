// Simple culling Geometry Shader Example - Vertex Shader
// Vertex Shader
// Graham Sellers.
// OpenGL SuperBible 5th Ed.
#version 150

precision highp float;

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in Vertex
{
    vec3 normal;
    vec4 color;
} vertex[];

out vec4 color;

uniform mat4 mvpMatrix;
uniform mat4 mvMatrix;
uniform mat3 normalMatrix;

uniform vec3 viewpoint;
// const vec3 viewpoint = vec3(0.0, 40.0, -10.0);

void main(void)
{
    int n;
    
    vec3 ab = gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz;
    vec3 ac = gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz;
    vec3 normal = normalize(cross(ab, ac));
    vec3 transformed_normal = (normalMatrix * normal);
    vec4 worldspace = /* mvMatrix * */ gl_in[0].gl_Position;
    vec3 vt = normalize(viewpoint - worldspace.xyz);

    if (dot(normal, vt) > 0.0) {
        for (n = 0; n < 3; n++) {
            gl_Position = mvpMatrix * gl_in[n].gl_Position;
            color = vertex[n].color;
            EmitVertex();
        }
        EndPrimitive();
    }
}