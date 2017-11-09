// Shader identycznoœciowy
// Shader fragmentów
// Richard S. Wright Jr.
// OpenGL. Ksiêga eksperta
#version 130

out vec4 vFragColor;

uniform vec4 vColor;

void main(void)
   { 
   vFragColor = vColor;
   }