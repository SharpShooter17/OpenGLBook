// Shader cieniowania kresk�wkowego
// Shader wierzcho�k�w
// Richard S. Wright Jr.
// OpenGL. Ksi�ga eksperta
#version 150

precision highp float;

// Dane przychodz�ce wierzcho�k�w... po�o�enie i normalna
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
