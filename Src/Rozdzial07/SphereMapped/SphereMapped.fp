// Shader o�wietlenia punktowego ADS
// Shader fragment�w
// Richard S. Wright Jr.
// OpenGL. Ksi�ga eksperta
#version 130

out vec4 vFragColor;

uniform sampler2D  sphereMap;

smooth in vec2 vVaryingTexCoord;

void main(void)
    { 
    vFragColor = texture(sphereMap, vVaryingTexCoord);
    }
    