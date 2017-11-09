// Shader oœwietlenia punktowego ADS
// Shader fragmentów
// Richard S. Wright Jr.
// OpenGL. Ksiêga eksperta
#version 130

out vec4 vFragColor;

uniform sampler2D  sphereMap;

smooth in vec2 vVaryingTexCoord;

void main(void)
    { 
    vFragColor = texture(sphereMap, vVaryingTexCoord);
    }
    