// Shader ShadedIdentity
// Shader fragmentów
// Richard S. Wright Jr.
// OpenGL. Ksiêga eksperta
#version 130

out vec4 vFragColor;
in vec4 vVaryingColor;

void main(void)
   { 
   vFragColor = vVaryingColor;
   }