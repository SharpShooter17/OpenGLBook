// MoonShader
// Shader fragmentów
// Richard S. Wright Jr.
// OpenGL. Ksiêga eksperta
#version 130

// Kolejny problem ze zgodnoœci¹ sterowników NVIDII
// Kod ten nie powinien byæ konieczny dla sterowników w wersji 3.3
#extension GL_EXT_gpu_shader4: enable

out vec4 vFragColor;

uniform sampler2DArray moonImage;

smooth in vec3 vMoonCoords;

void main(void)
   { 
   vFragColor = texture2DArray(moonImage, vMoonCoords.stp);
   }