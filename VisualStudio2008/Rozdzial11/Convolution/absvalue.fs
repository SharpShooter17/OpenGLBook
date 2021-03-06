#version 150
precision highp float;

// Blok interfejsu wejściowego dla danych z shadera wierzchołków
in Fragment
{
    vec2 tex_coord;
} fragment;

//
uniform sampler2D tex_input_image;

out vec4 output_color;

void main(void)
{
    output_color = abs(texture(tex_input_image, fragment.tex_coord));
}
