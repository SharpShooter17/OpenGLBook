// Shader ksi�yca
// Shader fragment�w
// Richard S. Wright Jr.
// OpenGL. Ksi�ga eksperta
#version 130

// Kolejny b��d zgodno�ci ze sterownikami NVIDII
// Nie powinno by� konieczne umieszczanie tego tutaj, je�li u�ywany jest sterownik 3.3.
#extension GL_EXT_gpu_shader4: enable

out vec4 vFragColor;

uniform sampler2DArray moonImage;

smooth in vec3 vMoonCoords;

void main(void)
   { 
   vFragColor = texture2DArray(moonImage, vMoonCoords.stp);
   }