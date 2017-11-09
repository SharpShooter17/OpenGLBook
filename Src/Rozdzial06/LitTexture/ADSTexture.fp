// Shader o�wietlenia punktowego ADS
// Shader fragment�w
// Richard S. Wright Jr.
// OpenGL. Ksi�ga eksperta
#version 130

out vec4 vFragColor;

uniform vec4      ambientColor;
uniform vec4      diffuseColor;   
uniform vec4      specularColor;
uniform sampler2D colorMap;

smooth in vec3 vVaryingNormal;
smooth in vec3 vVaryingLightDir;
smooth in vec2 vTexCoords;

void main(void)
    { 
    // Iloczyn skalarny da nam intensywno�� rozproszenia
    float diff = max(0.0, dot(normalize(vVaryingNormal), normalize(vVaryingLightDir)));

    // Mno�enie intensywno�ci przez kolor rozproszenia, alfa = 1.0
    vFragColor = diff * diffuseColor;

    // Dodanie �wiat�a otaczaj�cego
    vFragColor += ambientColor;

    // Dodanie do tekstury
    vFragColor *= texture(colorMap, vTexCoords);

    // �wiat�o odbite
    vec3 vReflection = normalize(reflect(-normalize(vVaryingLightDir), normalize(vVaryingNormal)));
    float spec = max(0.0, dot(normalize(vVaryingNormal), vReflection));
    if(diff != 0) {
        float fSpec = pow(spec, 128.0);
        vFragColor.rgb += vec3(fSpec, fSpec, fSpec);
        }
    }
    