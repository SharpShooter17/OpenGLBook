// Shader o�wietlenia punktowego ADS
// Shader fragment�w
// Richard S. Wright Jr.
// OpenGL SuperBible
#version 130

out vec4 vFragColor;
smooth in vec4 vVaryingColor;

void main(void)
   { 
   vFragColor = vVaryingColor;
   }