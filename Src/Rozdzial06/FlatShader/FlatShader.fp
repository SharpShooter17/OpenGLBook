// Shader p�aski
// Shader fragment�w
// Richard S. Wright Jr.
// OpenGL. Ksi�ga eksperta
#version 130

// Geometria ma by� jednolita
uniform vec4 vColorValue;

// Kolor fragmentu wyj�ciowego
out vec4 vFragColor;


void main(void)
   { 
   vFragColor = vColorValue;
   }