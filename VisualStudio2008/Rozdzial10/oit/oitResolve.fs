#version 150 
// oitResolve.fs
// 

in vec2 vTexCoord;

uniform sampler2DMS origImage;
uniform sampler2DMS origDepth;

out vec4 oColor;

void main(void) 
{ 
    const int sampleCount = 8;
    	
    vec4  vColor[sampleCount];
	float vDepth[sampleCount];
	int   vSurfOrder[sampleCount];
	int   i = 0;
	
    // Obliczenie nieznormalizowanych wsp�rz�dnych tekstury
	vec2 tmp = floor(textureSize2DMS(origDepth) * vTexCoord); 
		
    // Pobranie danych pr�bek i zainicjalizowanie kolejno�ci powierzchni
	for (i = 0; i < sampleCount; i++)
	{
		vSurfOrder[i] = i;
        vColor[i] = texelFetch(origImage, ivec2(tmp), i);
	    vDepth[i] = texelFetch(origDepth, ivec2(tmp), i).r;
	}
	
	// Sortowanie warto�ci g��bi, najwi�ksze na prz�d, najmniejsze na ty�
	// Musimy wykona� (size^2-size), przej�� przez tablic� lub sko�czy� wcze�niej, je�li
    // w kt�rym� przebiegu oka�e si�, �e wszystkie pr�bki s� we w�a�ciwej kolejno�ci
	for (int j = 0; j < sampleCount; j++)
    {
		bool bFinished = true;
        for (i = 0; i < (sampleCount-1); i++)
	    {
	        float temp1 = vDepth[vSurfOrder[i]];
	        float temp2 = vDepth[vSurfOrder[i+1]];
    	    
	        if (temp2 < temp1)
	        {
	            // swap values
	            int tempIndex   = vSurfOrder[i];
	            vSurfOrder[i]   = vSurfOrder[i+1];
	            vSurfOrder[i+1] = tempIndex;
	            bFinished = false;
	        }
	    }
	    
	    if (bFinished)
            j = 8;
	}
	
	// Zsumowanie wszystkich kolor�w w kolejno�ci od przodu do ty�u. Zastosowanie warto�ci alfa
	bool bFoundFirstColor = false;
	vec4 summedColor = vec4(0.0, 0.0, 0.0, 0.0);
	for (i = (sampleCount-1); i >= 0; i--)
    {
		int surfIndex = vSurfOrder[i];
		if(vColor[surfIndex].a > 0.001)
		{
			if (bFoundFirstColor == false)
			{
				// Zastosowanie 100% pierwszego koloru
				summedColor = vColor[surfIndex];
				bFoundFirstColor = true;
			}
			else
			{
				// Zastosowanie koloru z warto�ci� alfa 
				summedColor.rgb = (summedColor.rgb * (1 - vColor[surfIndex].a))     +
				                  (vColor[surfIndex].rgb * vColor[surfIndex].a);
			}
		}
    }
   
   oColor = summedColor;
   
    int surfIndex = 2;
    float val = vDepth[vSurfOrder[surfIndex]];
    //oColor = vec4(val, val, val, 1.0);
    //oColor = vec4(vColor[vSurfOrder[surfIndex]].rgb, 1.0);
    //oColor = vec4(vColor[0].rgb, 1.0);
    oColor.a = 1.0f;
}