// Shader cieniowania kreskówkowego
// Shader wierzcho³ków
// Richard S. Wright Jr.
// OpenGL. Ksiêga eksperta
#version 150

precision highp float;

// Dane przychodz¹ce wierzcho³ków... po³o¿enie i normalna
in vec4 vVertex;
in vec3 vNormal;

out Vertex
{
    vec3 normal;
    vec4 color;
} vertex;

uniform vec3 vLightPosition;
uniform mat4 mvMatrix;
uniform mat3 normalMatrix;

void main(void)
{
    // Obliczenie normalnej do powierzchni we wspó³rzêdnych oka
    vec3 vEyeNormal = normalMatrix * vNormal;

    // Obliczenie po³o¿enia wierzcho³ka we wspó³rzêdnych oka
    vec4 vPosition4 = mvMatrix * vVertex;
    vec3 vPosition3 = vPosition4.xyz / vPosition4.w;

    // Wektor do Ÿród³a œwiat³a
    vec3 vLightDir = normalize(vLightPosition - vPosition3);

    // Iloczyn skalarny daje intensywnoœæ rozproszenia
    vertex.color = vec4(0.3, 0.3, 0.9, 1.0) * max(0.0, dot(vEyeNormal, vLightDir));

    gl_Position = vVertex;
    vertex.normal = vNormal;
}
