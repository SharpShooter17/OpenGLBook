#version 150 
// hdr_exposure.fs
// Skaluje teksturê zmiennoprzecinkow¹ do zakresu 0.0 - 1.0
// w okreœlonym naœwietleniu
// Rozwi¹zuje bufor wielopróbkowania na podstawie liczby próbek wejœciowych
// 

in vec2 vTexCoord;

uniform sampler2DMS origImage;
uniform samplerBuffer sampleWeightSampler; 
uniform int sampleCount; // Liczymy od zera, tzn. 0 = 1 próbka, 1 = 2 próbki itd.
uniform int useWeightedResolve; // 0-false, 1-true
uniform float exposure;

out vec4 oColor;

// Odwzorowywanie tonów wykonujemy w oddzielnej funkcji
vec4 toneMap(vec4 vHdrColor)
{
	vec4 vLdrColor;
	vLdrColor = 1.0 - exp2(-vHdrColor * exposure);
	vLdrColor.a = 1.0f;
	return vLdrColor;
}

void main(void) 
{ 
    // Obliczenie nieznormalizowanych wspó³rzêdnych tekstury
	vec2 tmp = floor(textureSize2DMS(origImage) * vTexCoord); 

    // Znalezienie kolorów wa¿onych i niewa¿onych
	vec4 vColor = vec4(0.0, 0.0, 0.0, 1.0);
	vec4 vWeightedColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	for (int i = 0; i <= sampleCount; i++)
	{
	    // Pobranie wagi próbki z TBO, zmienia siê
	    // w zale¿noœci od liczby próbek
	    float weight = texelFetch(sampleWeightSampler, i).r;
	    
	    // Odwzorowanie tonów teksela HDR przed jego wa¿eniem
	    vec4 sample = toneMap(texelFetch(origImage, ivec2(tmp), i));
	    
		vWeightedColor += sample * weight;
		vColor += sample;
	}

    // Wybór rodzaju rozwi¹zywania
    oColor = vWeightedColor;
	
	// Jeœli u¿ytkownik wybierze rozwi¹zywanie niewa¿one, zwracamy
	// œredni¹ zwyk³¹
	if (useWeightedResolve != 0)
	{
	    oColor = vColor / (sampleCount+1);    
	}

    oColor.a = 1.0f;
    
}