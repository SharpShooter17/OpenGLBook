// Wizualizacja normalnych
// Shader fragment�w
// Graham Sellers
// OpenGL. Ksi�ga eksperta
#version 150

precision highp float;

in vec4 color;

out vec4 output_color;

void main(void)
{
    output_color = color;
}
