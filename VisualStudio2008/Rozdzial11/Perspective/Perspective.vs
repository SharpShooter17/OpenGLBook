// Korekcja perspektywy
// Shader wierzcho³ków
// Graham Sellers
// OpenGL. Ksiêga eksperta
#version 150

precision highp float;

// Dane przychodz¹ce wierzcho³ków — po³o¿enie i wspó³rzêdne tekstury
in vec4 vVertex;
in vec2 vTexCoord;

// Zwyk³a wspó³rzêdna tekstury
out vec2 texcoord;
// Wspó³rzêdna tekstury bez perspektywy
noperspective out vec2 texcoordNoPerspective;

uniform mat4 mvpMatrix;

void main(void)
{
    // Iloczyn skalarny daje intensywnoœæ rozproszenia
    texcoord = vTexCoord;
    texcoordNoPerspective = vTexCoord;
    gl_Position = mvpMatrix * vVertex;
}
