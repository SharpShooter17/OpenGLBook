// Shader o�wietlenia kom�rki
// Shader fragment�w
// Richard S. Wright Jr.
// OpenGL. Ksi�ga eksperta
#version 130

uniform sampler1D colorTable;
out vec4 vFragColor;

smooth in float textureCoordinate;


void main(void)
   { 
   vFragColor = texture(colorTable, textureCoordinate);
   }