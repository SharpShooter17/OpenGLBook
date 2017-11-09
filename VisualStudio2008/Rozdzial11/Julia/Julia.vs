// Korekcja perspektywy
// Shader wierzcho�k�w
// Graham Sellers
// OpenGL. Ksi�ga eksperta
#version 150

precision highp float;

// Dane przychodz�ce wierzcho�k�w � po�o�enie i wsp�rz�dne tekstury
in vec4 vVertex;

// Wsp�czynnik zoomu
uniform float zoom;

// Wektor przesuni�cia
uniform vec2 offset;

out vec2 initial_z;

void main(void)
{
    initial_z = (vVertex.xy * zoom) + offset;
    gl_Position = vVertex;
}
