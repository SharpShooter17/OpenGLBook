// Shader p³aski
// Shader fragmentów
// Richard S. Wright Jr.
// OpenGL. Ksiêga eksperta
#version 130

// Tworzenie jednolitej geometrii
uniform vec4 vColorValue;

// Wyjœcie koloru fragmentu
out vec4 vFragColor;


void main(void)
   { 
   vFragColor = vColorValue;
   }