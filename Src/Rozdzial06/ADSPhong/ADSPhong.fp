// Shader o�wietlenia punktowego ADS
// Shader fragment�w
// Richard S. Wright Jr.
// OpenGL. Ksi�ga eksperta
#version 130

out vec4 vFragColor;

uniform vec4    ambientColor;
uniform vec4    diffuseColor;   
uniform vec4    specularColor;

smooth in vec3 vVaryingNormal;
smooth in vec3 vVaryingLightDir;


void main(void)
    { 
    // Obliczenie nat�enia sk�adowe �wiat�a rozproszonego poprzez obliczenie iloczynu skalarnego wektor�w
    float diff = max(0.0, dot(normalize(vVaryingNormal), normalize(vVaryingLightDir)));

    // Mno�enie nat�enia przez kolor rozproszony, alfa ma warto�� 1.0
    vFragColor = diff * diffuseColor;

    // Dodanie sk�adowej �wiat�a otaczaj�cego
    vFragColor += ambientColor;


    // �wiat�o odbite zwierciadlanie
    vec3 vReflection = normalize(reflect(-normalize(vVaryingLightDir), normalize(vVaryingNormal)));
    float spec = max(0.0, dot(normalize(vVaryingNormal), vReflection));
    if(diff != 0) {
        float fSpec = pow(spec, 128.0);
        vFragColor.rgb += vec3(fSpec, fSpec, fSpec);
        }
    }
    