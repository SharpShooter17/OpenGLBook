// Shader œwiat³a normalnego mapowania
// Shader fragmentów
// Richard S. Wright Jr.
// OpenGL. Ksiêga eksperta
#version 130

uniform vec4      ambientColor;
uniform vec4      diffuseColor;   

uniform sampler2D colorMap;
uniform sampler2D normalMap;

smooth in vec3 vVaryingLightDir;
smooth in vec2 vTexCoords;

out vec4 vFragColor;

void main(void)
    { 
    vec3 vTextureNormal = texture2D(normalMap, vTexCoords).xyz;
    vTextureNormal = (vTextureNormal - 0.5) * 2.0;
    
    // Iloczyn skalarny da nam intensywnoœæ rozproszenia
    float diff = max(0.0, dot(normalize(vTextureNormal), normalize(vVaryingLightDir)));

    // Mno¿enie intensywnoœci przez kolor rozproszenia, alfa = 1.0
    vFragColor = diff * diffuseColor;

    // Dodanie œwiat³a otaczaj¹cego
    vFragColor += ambientColor;
    vFragColor.rgb = min(vec3(1.0,1.0,1.0), vFragColor.rgb);

    // Dodanie do tekstury
    vFragColor *= texture(colorMap, vTexCoords);
    }
    