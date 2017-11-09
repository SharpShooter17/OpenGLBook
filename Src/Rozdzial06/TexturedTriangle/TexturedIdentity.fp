// Shader TexturedIdentity
// Shader fragment�w
// Richard S. Wright Jr.
// OpenGL. Ksi�ga eksperta
#version 130


uniform sampler2D colorMap;

out vec4 vFragColor;
smooth in vec2 vVaryingTexCoords;


void main(void)
   { 
   vFragColor = texture(colorMap, vVaryingTexCoords.st);
   }