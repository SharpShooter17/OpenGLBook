// Shader tekstury prostok¹tnej
// Shader fragmentów
// Richard S. Wright Jr.
// OpenGL. Ksiêga eksperta
#version 140

out vec4 vFragColor;

uniform sampler2DRect  rectangleImage;

smooth in vec2 vVaryingTexCoord;

void main(void)
    { 
    vFragColor = texture(rectangleImage, vVaryingTexCoord);
    }
    