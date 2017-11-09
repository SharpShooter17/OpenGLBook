#version 150 
// hdr.fs
// Prosta wymiana tekstury
// 

in vec2 vTexCoord;

uniform sampler2D textureUnit0;

out vec4 oColor;

void main(void) 
{ 
	// Pobranie z tekstury HDR
	vec4 hdrTexel = texture(textureUnit0, vTexCoord); 
	
	oColor = hdrTexel;
}
