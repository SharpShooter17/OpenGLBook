// Prosty shader oœwietlenia rozproszonego
// Shader fragmentów
// Richard S. Wright Jr.
// OpenGL. Ksiêga eksperta
#version 130

out vec4 vFragColor;
smooth in vec4 vVaryingColor;

void main(void)
   { 
   vFragColor = vVaryingColor;
   }