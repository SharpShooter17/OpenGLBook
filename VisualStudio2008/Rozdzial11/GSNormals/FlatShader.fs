// Wizualizacja normalnych
// Shader fragmentów
// Graham Sellers
// OpenGL. Ksiêga eksperta
#version 150

precision highp float;

in Fragment
{
    vec4 color;
} fragment;

out vec4 output_color;

void main(void)
{
    output_color = fragment.color;
}
