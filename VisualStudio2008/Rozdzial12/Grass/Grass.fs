// Korekcja perspektywy
// Shader fragmentów
// Graham Sellers
// OpenGL. Ksiêga eksperta
#version 150

precision highp float;

in vec4 color;

out vec4 output_color;

void main(void)
{
    vec2 coord;

    output_color = color;
}
