// Shader p�aski ProvokingVertex
// Shader fragment�w
// Richard S. Wright Jr.
// OpenGL. Ksi�ga eksperta
#version 130

out vec4 vFragColor;
flat in vec4 vVaryingColor;

void main(void)
   { 
   vFragColor = vVaryingColor;
   }