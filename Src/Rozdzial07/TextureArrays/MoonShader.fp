// Shader ksiê¿yca
// Shader fragmentów
// Richard S. Wright Jr.
// OpenGL. Ksiêga eksperta
#version 130

// Kolejny b³¹d zgodnoœci ze sterownikami NVIDII
// Nie powinno byæ konieczne umieszczanie tego tutaj, jeœli u¿ywany jest sterownik 3.3.
#extension GL_EXT_gpu_shader4: enable

out vec4 vFragColor;

uniform sampler2DArray moonImage;

smooth in vec3 vMoonCoords;

void main(void)
   { 
   vFragColor = texture2DArray(moonImage, vMoonCoords.stp);
   }