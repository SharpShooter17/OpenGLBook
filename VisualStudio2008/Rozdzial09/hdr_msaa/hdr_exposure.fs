#version 150 
// hdr_exposure.fs
// Skaluje tekstur� zmiennoprzecinkow� do zakresu 0.0 - 1.0
// w okre�lonym na�wietleniu
// Rozwi�zuje bufor wielopr�bkowania na podstawie liczby pr�bek wej�ciowych
// 

in vec2 vTexCoord;

uniform sampler2DMS origImage;
uniform samplerBuffer sampleWeightSampler; 
uniform int sampleCount; // Liczymy od zera, tzn. 0 = 1 pr�bka, 1 = 2 pr�bki itd.
uniform int useWeightedResolve; // 0-false, 1-true
uniform float exposure;

out vec4 oColor;

// Odwzorowywanie ton�w wykonujemy w oddzielnej funkcji
vec4 toneMap(vec4 vHdrColor)
{
	vec4 vLdrColor;
	vLdrColor = 1.0 - exp2(-vHdrColor * exposure);
	vLdrColor.a = 1.0f;
	return vLdrColor;
}

void main(void) 
{ 
    // Obliczenie nieznormalizowanych wsp�rz�dnych tekstury
	vec2 tmp = floor(textureSize2DMS(origImage) * vTexCoord); 

    // Znalezienie kolor�w wa�onych i niewa�onych
	vec4 vColor = vec4(0.0, 0.0, 0.0, 1.0);
	vec4 vWeightedColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	for (int i = 0; i <= sampleCount; i++)
	{
	    // Pobranie wagi pr�bki z TBO, zmienia si�
	    // w zale�no�ci od liczby pr�bek
	    float weight = texelFetch(sampleWeightSampler, i).r;
	    
	    // Odwzorowanie ton�w teksela HDR przed jego wa�eniem
	    vec4 sample = toneMap(texelFetch(origImage, ivec2(tmp), i));
	    
		vWeightedColor += sample * weight;
		vColor += sample;
	}

    // Wyb�r rodzaju rozwi�zywania
    oColor = vWeightedColor;
	
	// Je�li u�ytkownik wybierze rozwi�zywanie niewa�one, zwracamy
	// �redni� zwyk��
	if (useWeightedResolve != 0)
	{
	    oColor = vColor / (sampleCount+1);    
	}

    oColor.a = 1.0f;
    
}