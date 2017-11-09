// MoonShader
// Shader fragment�w
// Richard S. Wright Jr.
// OpenGL. Ksi�ga eksperta
#version 130

// Kolejny problem ze zgodno�ci� sterownik�w NVIDII
// Kod ten nie powinien by� konieczny dla sterownik�w w wersji 3.3
#extension GL_EXT_gpu_shader4: enable

out vec4 vFragColor;

uniform sampler2DArray moonImage;

smooth in vec3 vMoonCoords;

void main(void)
   { 
   vFragColor = texture2DArray(moonImage, vMoonCoords.stp);
   }