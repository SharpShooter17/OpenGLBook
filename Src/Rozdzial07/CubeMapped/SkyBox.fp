// Shader SkyBox
// Shader fragmentów
// Richard S. Wright Jr.
// OpenGL. Ksiêga eksperta
#version 130

out vec4 vFragColor;

uniform samplerCube  cubeMap;

varying vec3 vVaryingTexCoord;

void main(void)
    { 
    vFragColor = texture(cubeMap, vVaryingTexCoord);
    }
    