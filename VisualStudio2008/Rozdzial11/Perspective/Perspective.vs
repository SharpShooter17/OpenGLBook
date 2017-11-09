// Korekcja perspektywy
// Shader wierzcho�k�w
// Graham Sellers
// OpenGL. Ksi�ga eksperta
#version 150

precision highp float;

// Dane przychodz�ce wierzcho�k�w � po�o�enie i wsp�rz�dne tekstury
in vec4 vVertex;
in vec2 vTexCoord;

// Zwyk�a wsp�rz�dna tekstury
out vec2 texcoord;
// Wsp�rz�dna tekstury bez perspektywy
noperspective out vec2 texcoordNoPerspective;

uniform mat4 mvpMatrix;

void main(void)
{
    // Iloczyn skalarny daje intensywno�� rozproszenia
    texcoord = vTexCoord;
    texcoordNoPerspective = vTexCoord;
    gl_Position = mvpMatrix * vVertex;
}
