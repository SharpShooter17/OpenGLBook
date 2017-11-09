// Shader p³aski
// Shader fragmentów
// Richard S. Wright Jr.
// OpenGL. Ksiêga eksperta
#version 130

// Geometria ma byæ jednolita
uniform vec4 vColorValue;

// Kolor fragmentu wyjœciowego
out vec4 vFragColor;


void main(void)
   { 
   vFragColor = vColorValue;
   }