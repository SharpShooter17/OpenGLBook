// Shader cieniowania kreskówkowego
// Shader wierzchołków
// Richard S. Wright Jr.
// OpenGL. Księga eksperta
#version 150

precision highp float;

// Dane przychodzące wierzchołków... położenie i normalna
in vec4 vVertex;
in vec3 vNormal;

out Vertex
{
    vec3 normal;
} vertex;

void main(void)
{
    gl_Position = vVertex;
    vertex.normal = vNormal;
}
