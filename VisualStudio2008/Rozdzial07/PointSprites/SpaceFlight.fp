// SpaceFlight Shader
// Shader fragment�w
// Richard S. Wright Jr.
// OpenGL. Ksi�ga eksperta
#version 130

out vec4 vFragColor;

in vec4 vStarColor;

uniform sampler2D  starImage;

void main(void)
    { 
    vFragColor = texture(starImage, gl_PointCoord) * vStarColor;
    }
    