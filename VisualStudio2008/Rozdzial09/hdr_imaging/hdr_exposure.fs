#version 150 
// hdr_exposure.fs
// Skaluje teksturê zmiennoprzecinkow¹ do zakresu 0.0 - 1.0 
// w okreœlonym naœwietleniu
// 

in vec2 vTexCoord;

uniform sampler2D textureUnit0;
uniform float exposure;

out vec4 oColor;

void main(void) 
{ 
	// Pobranie z tekstury HDR
	vec4 vColor = texture(textureUnit0, vTexCoord); 
	
	// Zastosowanie ekspozycji do tego teksela
	oColor = 1.0 - exp2(-vColor * exposure);
    oColor.a = 1.0f;
}