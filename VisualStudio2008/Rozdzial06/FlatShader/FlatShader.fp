// Shader p�aski
// Shader fragment�w
// Richard S. Wright Jr.
// OpenGL. Ksi�ga eksperta
#version 130

// Tworzenie jednolitej geometrii
uniform vec4 vColorValue;

// Wyj�cie koloru fragmentu
out vec4 vFragColor;


void main(void)
   { 
   vFragColor = vColorValue;
   }