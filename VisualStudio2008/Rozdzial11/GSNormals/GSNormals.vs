// Shader cieniowania kreskówkowego
// Shader wierzcho³ków
// Richard S. Wright Jr.
// OpenGL. Ksiêga eksperta
#version 150

precision highp float;

// Dane przychodz¹ce wierzcho³ków... po³o¿enie i normalna
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
