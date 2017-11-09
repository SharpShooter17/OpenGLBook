#version 150 
// hdr.fs
// Zwraca bezpoœrednio wartoœæ tekstury
// 

in vec2 vTexCoord;

uniform sampler2D textureUnit0;

void main(void) 
{ 
	// Pobranie z tekstury HDR
	vec4 hdrTexel = texture(textureUnit0, vTexCoord); 
	
	gl_FragColor = hdrTexel;
}
