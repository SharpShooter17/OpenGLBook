// Shader identyczno�ciowy
// Shader fragment�w
// Richard S. Wright Jr.
// OpenGL. Ksi�ga eksperta
#version 130

out vec4 vFragColor;

uniform vec4 vColor;

void main(void)
   { 
   vFragColor = vColor;
   }