// Korekcja perspektywy
// Shader wierzcho³ków
// Graham Sellers
// OpenGL. Ksiêga eksperta
#version 150

precision highp float;

// Dane przychodz¹ce wierzcho³ków — po³o¿enie i wspó³rzêdne tekstury
in vec4 vVertex;

// Wspó³czynnik zoomu
uniform float zoom;

// Wektor przesuniêcia
uniform vec2 offset;

out vec2 initial_z;

void main(void)
{
    initial_z = (vVertex.xy * zoom) + offset;
    gl_Position = vVertex;
}
