// Shader cieniowania kresk�wkowego
// Shader wierzcho�k�w
// Richard S. Wright Jr.
// OpenGL. Ksi�ga eksperta
#version 150

precision highp float;

// Dane przychodz�ce wierzcho�k�w... po�o�enie i normalna
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
    // Obliczenie normalnej do powierzchni we wsp�rz�dnych oka
    vec3 vEyeNormal = normalMatrix * vNormal;

    // Obliczenie po�o�enia wierzcho�ka we wsp�rz�dnych oka
    vec4 vPosition4 = mvMatrix * vVertex;
    vec3 vPosition3 = vPosition4.xyz / vPosition4.w;

    // Wektor do �r�d�a �wiat�a
    vec3 vLightDir = normalize(vLightPosition - vPosition3);

    // Iloczyn skalarny daje intensywno�� rozproszenia
    vertex.color = vec4(0.3, 0.3, 0.9, 1.0) * max(0.0, dot(vEyeNormal, vLightDir));

    gl_Position = vVertex;
    vertex.normal = vNormal;
}
