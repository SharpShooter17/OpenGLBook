#version 150 
// multibuffer.fs
// Wysy³a dane do trzech buforów: normalnego, koloru i skali szaroœci 
// oraz kolor z dostosowan¹ luminancj¹

in vec4 vFragColor; 
in vec2 vTexCoord; 

uniform sampler2D textureUnit0; 
uniform int bUseTexture;
uniform samplerBuffer lumCurveSampler; 

void main(void) { 
    vec4 vColor; 
    vec4 lumFactor; 
    
    if (bUseTexture != 0) 
	     vColor =  texture(textureUnit0, vTexCoord);
    else 
	     vColor = vFragColor;
	     
	// Nieruszane dane wyjœciowe do pierwszego bufora
	gl_FragData[0] = vColor;

	 // Czarny-bia³y do drugiego bufora
    float grey = dot(vColor.rgb, vec3(0.3, 0.59, 0.11));
	gl_FragData[1] = vec4(grey, grey, grey, 1.0f);
	
	// Obciêcie koloru wejœciowego do zakresu 0.0 - 1.0
	vColor = clamp(vColor, 0.0f, 1.0f);
	 
    int offset = int(vColor.r * (1024 - 1)); 
    lumFactor.r = texelFetch(lumCurveSampler, offset ).r;
    
    offset = int(vColor.g * (1024 - 1)); 
    lumFactor.g = texelFetch(lumCurveSampler, offset ).r;
    
    offset = int(vColor.b * (1024 - 1)); 
    lumFactor.b = texelFetch(lumCurveSampler, offset ).r;
    
    lumFactor.a = 1.0f;
    gl_FragData[2] = lumFactor; 
}