// Shader oœwietlenia punktowego ADS
// Shader fragmentów
// Richard S. Wright Jr.
// OpenGL. Ksiêga eksperta
#version 130

out vec4 vFragColor;

uniform vec4    ambientColor;
uniform vec4    diffuseColor;   
uniform vec4    specularColor;

smooth in vec3 vVaryingNormal;
smooth in vec3 vVaryingLightDir;


void main(void)
    { 
    // Obliczenie natê¿enia sk³adowe œwiat³a rozproszonego poprzez obliczenie iloczynu skalarnego wektorów
    float diff = max(0.0, dot(normalize(vVaryingNormal), normalize(vVaryingLightDir)));

    // Mno¿enie natê¿enia przez kolor rozproszony, alfa ma wartoœæ 1.0
    vFragColor = diff * diffuseColor;

    // Dodanie sk³adowej œwiat³a otaczaj¹cego
    vFragColor += ambientColor;


    // Œwiat³o odbite zwierciadlanie
    vec3 vReflection = normalize(reflect(-normalize(vVaryingLightDir), normalize(vVaryingNormal)));
    float spec = max(0.0, dot(normalize(vVaryingNormal), vReflection));
    if(diff != 0) {
        float fSpec = pow(spec, 128.0);
        vFragColor.rgb += vec3(fSpec, fSpec, fSpec);
        }
    }
    