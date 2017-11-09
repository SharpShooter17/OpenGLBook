#version 150
precision highp float;

// Blok interfejsu wej�ciowego dla danych z shadera wierzcho�k�w
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
