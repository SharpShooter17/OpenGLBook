// Shader odbicia
// Shader fragment�w
// Richard S. Wright Jr.
// OpenGL. Ksi�ga eksperta
#version 130

out vec4 vFragColor;

uniform samplerCube cubeMap;
uniform sampler2D   tarnishMap;

smooth in vec3 vVaryingTexCoord;
smooth in vec2 vTarnishCoords;

void main(void)
    { 
    vFragColor = texture(cubeMap, vVaryingTexCoord.stp);
    vFragColor *= texture(tarnishMap, vTarnishCoords);
    }
    