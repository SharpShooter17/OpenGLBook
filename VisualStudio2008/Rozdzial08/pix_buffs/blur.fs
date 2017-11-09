#version 150 
// blur.fs
// Zwraca wa¿ony wynik zmieszania czterech tekstur
// 

in vec2 vTexCoord;
uniform sampler2D textureUnit0;
uniform sampler2D textureUnit1;
uniform sampler2D textureUnit2;
uniform sampler2D textureUnit3;
uniform sampler2D textureUnit4;
uniform sampler2D textureUnit5;
void main(void) 
{ 
	// 0 oznacza najnowszy obraz, a 5 — najstarszy
	vec4 blur0 = texture(textureUnit0, vTexCoord); 
	vec4 blur1 = texture(textureUnit1, vTexCoord); 
	vec4 blur2 = texture(textureUnit2, vTexCoord); 
	vec4 blur3 = texture(textureUnit3, vTexCoord); 
	vec4 blur4 = texture(textureUnit4, vTexCoord); 
	vec4 blur5 = texture(textureUnit5, vTexCoord); 
	
	vec4 summedBlur = blur0	+ blur1 + blur2 +blur3 + blur4 + blur5;
	gl_FragColor = summedBlur/6;
}
