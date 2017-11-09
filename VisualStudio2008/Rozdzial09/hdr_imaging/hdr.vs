#version 150 
// hdr.vs
// Zwraca po�o�enie i wsp�rz�dn� tekstury 0
// 

in vec3 vVertex;
in vec2 vTexCoord0;

uniform mat4 mvpMatrix;

out vec2 vTexCoord;

void main(void) 
{ 
	vTexCoord = vTexCoord0;
	gl_Position = mvpMatrix * vec4(vVertex, 1.0); 
}