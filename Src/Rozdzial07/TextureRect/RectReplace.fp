// Shader tekstury prostok�tnej
// Shader fragment�w
// Richard S. Wright Jr.
// OpenGL. Ksi�ga eksperta
#version 140

out vec4 vFragColor;

uniform sampler2DRect  rectangleImage;

smooth in vec2 vVaryingTexCoord;

void main(void)
    { 
    vFragColor = texture(rectangleImage, vVaryingTexCoord);
    }
    