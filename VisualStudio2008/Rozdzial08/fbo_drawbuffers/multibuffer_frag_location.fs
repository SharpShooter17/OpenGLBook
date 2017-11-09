#version 150 
// multibuffer_frag_location.fs
/// Wysy�a dane do trzech bufor�w: normalnego, koloru i skali szaro�ci 
// oraz kolor z dostosowan� luminancj�

in vec4 vFragColor; 
in vec2 vTexCoord; 

uniform int bUseTexture;
uniform sampler2D textureUnit0; 
uniform samplerBuffer lumCurveSampler; 

out vec4 oStraightColor;
out vec4 oGreyscale;
out vec4 oLumAdjColor;

void main(void) { 
    vec4 vColor; 
    vec4 lumFactor; 
    
    if (bUseTexture != 0) 
         vColor =  texture(textureUnit0, vTexCoord);
    else 
         vColor = vFragColor;
         
    // Nieruszane dane wyj�ciowe do pierwszego bufora
    oStraightColor = vColor;

     // Czarny-bia�y do drugiego bufora
    float grey = dot(vColor.rgb, vec3(0.3, 0.59, 0.11));
    oGreyscale = vec4(grey, grey, grey, 1.0f);
    
    // Obci�cie koloru wej�ciowego do zakresu 0.0 - 1.0
    vColor = clamp(vColor, 0.0f, 1.0f);
     
    int offset = int(vColor.r * (1024 - 1)); 
    oLumAdjColor.r = texelFetch(lumCurveSampler, offset ).r;
    
    offset = int(vColor.g * (1024 - 1)); 
    oLumAdjColor.g = texelFetch(lumCurveSampler, offset ).r;
    
    offset = int(vColor.b * (1024 - 1)); 
    oLumAdjColor.b = texelFetch(lumCurveSampler, offset ).r;
    
    oLumAdjColor.a = 1.0f;
}