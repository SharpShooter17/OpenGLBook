#include <stdio.h>
#include <iostream>
#include <ImfRgbaFile.h>            // Nag³ówki OpenEXR
#include <ImfArray.h>

#include <GLTools.h>
#include <GLFrustum.h>
#include <GLBatch.h>
#include <GLMatrixStack.h>
#include <GLGeometryTransform.h>
#include <StopWatch.h>

#include <GL\glu.h>

#ifdef __APPLE__
#include <glut/glut.h>
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>
#endif

#ifdef _WIN32
#pragma comment (lib, "half.lib") 
#pragma comment (lib, "Iex.lib")
#pragma comment (lib, "IlmImf.lib")
#pragma comment (lib, "IlmThread.lib")
#pragma comment (lib, "Imath.lib")
#pragma comment (lib, "zlib.lib")
#endif

#pragma warning( disable : 4244)

static GLfloat vWhite[] = { 1.0f, 1.0f, 1.0f, 1.0f };
static GLfloat vWhiteX2[] = { 2.0f, 2.0f, 2.0f, 2.0f };
static GLfloat vLtGrey[] =  { 0.75f, 0.75f, 0.75f, 1.0f };
static GLfloat vLightPos[] = { -2.0f, 3.0f, -2.0f, 1.0f };
static GLfloat vSkyBlue[] = { 0.260f, 0.476f, 0.925f, 1.0f};

static const GLenum windowBuff[] = { GL_BACK_LEFT };
static const GLenum fboBuffs[] = { GL_COLOR_ATTACHMENT0 };

GLsizei	 screenWidth;			// ¯¹dana szerokoœæ okna lub pulpitu
GLsizei  screenHeight;			// ¯¹dana wysokoœæ okna lub pulpitu

GLboolean bFullScreen;			// ¯¹danie dzia³ania w trybie pe³noekranowym
GLboolean bAnimated;			// ¯¹danie ci¹g³ych aktualizacji

GLMatrixStack		modelViewMatrix;		// Macierz model-widok
GLMatrixStack		projectionMatrix;		// Macierz rzutowania
GLFrustum			viewFrustum;			// Frusta widoku
GLGeometryTransform	transformPipeline;		// Potok przetwarzania geometrii
GLFrame				cameraFrame;			// Uk³ad odniesienia kamery
GLBatch             screenQuad;
M3DMatrix44f        orthoMatrix;  

GLBatch				floorBatch;
GLBatch				windowBatch;
GLBatch				windowBorderBatch;
GLBatch				windowGridBatch;

GLuint				flatColorProg;
GLuint				texReplaceProg;
GLuint				hdrResolve;
GLuint				blurProg;

GLuint              hdrFBO[1];
GLuint              brightPassFBO[4];
GLuint				textures[1];
GLuint				hdrTextures[1];
GLuint              texBOTexture;
GLuint				windowTexture;
GLfloat				exposure;
GLint				sampleCount;
GLint               lastSampleCount;
GLuint				sampleWeightBuf;
GLfloat				sampleWeights[8][8];
GLuint              useWeightedResolve;

void GenerateOrtho2DMat(GLuint imageWidth, GLuint imageHeight);
bool LoadOpenEXRImage(char *fileName, GLint textureName, GLuint &texWidth, GLuint &texHeight);
void SetupTexReplaceProg(GLfloat *vLightPos, GLfloat *vColor);
void SetupFlatColorProg(GLfloat *vLightPos, GLfloat *vColor);
void SetupHDRProg();

///////////////////////////////////////////////////////////////////////////////////////////////////////
// Wczytanie pliku BMP jako tekstury. Umo¿liwia okreœlanie filtrów i trybu zawijania
bool LoadBMPTexture(const char *szFileName, GLenum minFilter, GLenum magFilter, GLenum wrapMode)	
{
    GLbyte *pBits;
    GLint iWidth, iHeight;

    pBits = gltReadBMPBits(szFileName, &iWidth, &iHeight);
    if(pBits == NULL)
        return false;

    // Ustawienia trybów zawijania
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);

    // Czy konieczne jest generowanie mipmap?
    if(minFilter == GL_LINEAR_MIPMAP_LINEAR || minFilter == GL_LINEAR_MIPMAP_NEAREST || minFilter == GL_NEAREST_MIPMAP_LINEAR || minFilter == GL_NEAREST_MIPMAP_NEAREST)
        glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, iWidth, iHeight, 0, GL_BGR, GL_UNSIGNED_BYTE, pBits);
    return true;
}



///////////////////////////////////////////////////////////////////////////////
// Tu mo¿na bezpiecznie umieœciæ kod innicjuj¹cy OpenGL, ³adowanie tekstur itp.
void SetupRC(void)
{
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        /* Problem: wywo³anie funkcji glewInit nie powiod³o siê */
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
    }

    glEnable(GL_DEPTH_TEST);

    exposure = 1.0f;

    // Jasnoniebieski
    glClearColor(vSkyBlue[0], vSkyBlue[1], vSkyBlue[2], vSkyBlue[3]);

    // Wczytanie geometrii
    GLfloat alpha = 0.25f;
    floorBatch.Begin(GL_TRIANGLE_FAN, 4, 1);
        floorBatch.Color4f(0.0f, 1.0f, 0.0f, alpha);
        floorBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
        floorBatch.Normal3f(0.0, 1.0f, 0.0f);
        floorBatch.Vertex3f(-20.0f, -0.41f, 20.0f);

        floorBatch.Color4f(0.0f, 1.0f, 0.0f, alpha);
        floorBatch.MultiTexCoord2f(0, 1.00f, 0.0f);
        floorBatch.Normal3f(0.0, 1.0f, 0.0f);
        floorBatch.Vertex3f(20.0f, -0.41f, 20.0f);

        floorBatch.Color4f(0.0f, 1.0f, 0.0f, alpha);
        floorBatch.MultiTexCoord2f(0, 1.00f, 1.00f);
        floorBatch.Normal3f(0.0, 1.0f, 0.0f);
        floorBatch.Vertex3f(20.0f, -0.41f, -20.0f);

        floorBatch.Color4f(0.0f, 1.0f, 0.0f, alpha);
        floorBatch.MultiTexCoord2f(0, 0.0f, 1.00f);
        floorBatch.Normal3f(0.0, 1.0f, 0.0f);
        floorBatch.Vertex3f(-20.0f, -0.41f, -20.0f);
    floorBatch.End();

    windowBatch.Begin(GL_TRIANGLE_FAN, 4, 1);
        windowBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
        windowBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
        windowBatch.Normal3f( 0.0f, 1.0f, 0.0f);
        windowBatch.Vertex3f(-1.0f, 0.0f, 0.0f);

        windowBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
        windowBatch.MultiTexCoord2f(0, 1.0f, 0.0f);
        windowBatch.Normal3f(0.0f, 1.0f, 0.0f);
        windowBatch.Vertex3f(1.0f, 0.0f, 0.0f);

        windowBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
        windowBatch.MultiTexCoord2f(0, 1.0f, 1.0f);
        windowBatch.Normal3f(0.0f, 1.0f, 0.0f);
        windowBatch.Vertex3f(1.0f, 2.0f, 0.0f);

        windowBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
        windowBatch.MultiTexCoord2f(0, 0.0f, 1.0f);
        windowBatch.Normal3f( 0.0f, 1.0f, 0.0f);
        windowBatch.Vertex3f(-1.0f, 2.0f, 0.0f);
    windowBatch.End();

    const float width = 0.2f;
    windowBorderBatch.Begin(GL_TRIANGLE_STRIP, 13);
        windowBorderBatch.Normal3f( 0.0f, 0.0f, 1.0f);
        windowBorderBatch.Vertex3f(-1.01f, width, 0.01f);

        windowBorderBatch.Normal3f(0.0f, 0.0f, 1.0f);
        windowBorderBatch.Vertex3f(-1.01f, 0.0f, 0.01f);

        windowBorderBatch.Normal3f(0.0f, 0.0f, 1.0f);
        windowBorderBatch.Vertex3f(1.01f, width, 0.01f);

        windowBorderBatch.Normal3f( 0.0f, 0.0f, 1.0f);
        windowBorderBatch.Vertex3f(1.01f, 0.0f, 0.01f);

        windowBorderBatch.Normal3f( 0.0f, 0.0f, 1.0f);
        windowBorderBatch.Vertex3f(1.01-width, 0.0f, 0.01f);

        windowBorderBatch.Normal3f( 0.0f, 0.0f, 1.0f);
        windowBorderBatch.Vertex3f(1.01f, 2.0f, 0.01f);
            
        windowBorderBatch.Normal3f( 0.0f, 0.0f, 1.0f);
        windowBorderBatch.Vertex3f(1.01-width, 2.0f, 0.01f);
            
        windowBorderBatch.Normal3f( 0.0f, 0.0f, 1.0f);
        windowBorderBatch.Vertex3f(1.01f, 2.0-width, 0.01f);

        windowBorderBatch.Normal3f( 0.0f, 0.0f, 1.0f);
        windowBorderBatch.Vertex3f(-1.01f, 2.f, 0.01f);

        windowBorderBatch.Normal3f( 0.0f, 0.0f, 1.0f);
        windowBorderBatch.Vertex3f(-1.01f, 2.0-width, 0.01f);

        windowBorderBatch.Normal3f( 0.0f, 0.0f, 1.0f);
        windowBorderBatch.Vertex3f(-1.01+width, 2.f, 0.01f);

        windowBorderBatch.Normal3f( 0.0f, 0.0f, 1.0f);
        windowBorderBatch.Vertex3f(-1.01f, 0.0f, 0.01f);

        windowBorderBatch.Normal3f( 0.0f, 0.0f, 1.0f);
        windowBorderBatch.Vertex3f(-1.01+width, 0.0f, 0.01f);
    windowBorderBatch.End();

    const float gridWidth = (float)0.01;
    const int gridLineCount = 24;
    windowGridBatch.Begin(GL_TRIANGLES, gridLineCount*2*6);
        // dó³ poziom
        
        for(int i=0; i<gridLineCount; i++)
        {
            float offset = 2*((float)(i+1)/(float)(gridLineCount+1));
            windowGridBatch.Normal3f( 0.0f, 0.0f, 1.0f);
            windowGridBatch.Vertex3f(-1.0f, offset+gridWidth, 0.01f);

            windowGridBatch.Normal3f(0.0f, 0.0f, 1.0f);
            windowGridBatch.Vertex3f(-1.0f, offset-gridWidth, 0.01f);

            windowGridBatch.Normal3f(0.0f, 0.0f, 1.0f);
            windowGridBatch.Vertex3f(1.0f, offset-gridWidth, 0.01f);

            windowGridBatch.Normal3f(0.0f, 0.0f, 1.0f);
            windowGridBatch.Vertex3f(1.0f,offset-gridWidth, 0.01f);

            windowGridBatch.Normal3f(0.0f, 0.0f, 1.0f);
            windowGridBatch.Vertex3f(1.0f, offset+gridWidth, 0.01f);

            windowGridBatch.Normal3f( 0.0f, 0.0f, 1.0f);
            windowGridBatch.Vertex3f(-1.0f, offset+gridWidth, 0.01f);
        }

        // Pionowe
        for(int i=0; i<gridLineCount; i++)
        {
            float offset = 2*((float)(i+1)/(float)(gridLineCount+1)) - 1.0;
            windowGridBatch.Normal3f( 0.0f, 0.0f, 1.0f);
            windowGridBatch.Vertex3f(offset+gridWidth, 0.0f, 0.01f);

            windowGridBatch.Normal3f(0.0f, 0.0f, 1.0f);
            windowGridBatch.Vertex3f(offset-gridWidth, 0.0f, 0.01f);

            windowGridBatch.Normal3f(0.0f, 0.0f, 1.0f);
            windowGridBatch.Vertex3f(offset-gridWidth, 2.0f, 0.01f);

            windowGridBatch.Normal3f(0.0f, 0.0f, 1.0f);
            windowGridBatch.Vertex3f(offset-gridWidth, 2.0f, 0.01f);

            windowGridBatch.Normal3f(0.0f, 0.0f, 1.0f);
            windowGridBatch.Vertex3f(offset+gridWidth, 2.0, 0.01f);

            windowGridBatch.Normal3f( 0.0f, 0.0f, 1.0f);
            windowGridBatch.Vertex3f(offset+gridWidth, 0.0f, 0.01f);
        }
    
    windowGridBatch.End();

    glGenTextures(1, textures);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    LoadBMPTexture("marble.bmp", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_REPEAT);

	int Sampling = 8;
	bool SamplingFailed = false;
	do{
		if(SamplingFailed){
			Sampling /=2;
		}
		fprintf(stderr, "Liczba próbek: %i\n", Sampling);

		// Konfiguracja tekstur HDR
		glGenTextures(1, hdrTextures);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, hdrTextures[0]);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, Sampling /*8*/, GL_RGB16F, screenWidth, screenHeight, GL_FALSE);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		// Wi¹zanie tekstury HDR do FBO
		// Utworzenie i zwi¹zanie FBO
		glGenFramebuffers(1,hdrFBO);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, hdrFBO[0]);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, hdrTextures[0], 0);
		
		if(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT){
			SamplingFailed = true;
			fprintf(stderr, "Failed\n");
		}
		else{
			fprintf(stderr, "OK\n");
			SamplingFailed = false;
		}
	}
	while(SamplingFailed);
    
    // Utworzenie tekstury okna
    glGenTextures(1, &windowTexture);
    glBindTexture(GL_TEXTURE_2D, windowTexture);
    GLuint texWidth = 0;
    GLuint texHeight = 0;
    // Wczytanie obrazu HDR z pliku EXR
    LoadOpenEXRImage("window.exr", windowTexture, texWidth, texHeight);

    // Za³adowanie p³askiego shadera kolorów
    flatColorProg =  gltLoadShaderPairWithAttributes("basic.vs", "color.fs", 3, 
                            GLT_ATTRIBUTE_VERTEX, "vVertex", 
                            GLT_ATTRIBUTE_NORMAL, "vNormal", 
                            GLT_ATTRIBUTE_TEXTURE0, "vTexCoord0");
    glBindFragDataLocation(flatColorProg, 0, "oColor");
    glBindFragDataLocation(flatColorProg, 1, "oBright");
    glLinkProgram(flatColorProg);

    // Za³adowanie shadera zamiany tekstur
    texReplaceProg =  gltLoadShaderPairWithAttributes("basic.vs", "tex_replace.fs", 3, 
                            GLT_ATTRIBUTE_VERTEX, "vVertex", 
                            GLT_ATTRIBUTE_NORMAL, "vNormal", 
                            GLT_ATTRIBUTE_TEXTURE0, "vTexCoord0");
    glBindFragDataLocation(texReplaceProg, 0, "oColor");
    glBindFragDataLocation(texReplaceProg, 1, "oBright");
    glLinkProgram(texReplaceProg);

    // Za³adowanie shadera poœwiaty
    hdrResolve =  gltLoadShaderPairWithAttributes("basic.vs", "hdr_exposure.fs", 3, 
                            GLT_ATTRIBUTE_VERTEX, "vVertex", 
                            GLT_ATTRIBUTE_NORMAL, "vNormal", 
                            GLT_ATTRIBUTE_TEXTURE0, "vTexCoord0");
    glBindFragDataLocation(hdrResolve, 0, "oColor");
    glLinkProgram(hdrResolve);	

    // Usuñ komentarz z poni¿szego wiersza, aby program dzia³a³ poprawnie
    floorBatch.Draw();

    // Sprawdzenie lokalizacji wszystkich podpikseli wielopróbkowania
    int sampleCount = 0;
    glGetIntegerv(GL_SAMPLES, &sampleCount);

    float positions[64]; // Wystarczy dla przynajmniej 32 próbek
    for(int i =0; i < sampleCount; i++)
    {
        glGetMultisamplefv(GL_SAMPLE_POSITION, i, &positions[i*2]);
    }

    // Ograniczenie liczby próbek do 8x
    //assert(sampleCount >= 8);
    sampleCount = 8;

    float invertedSampleDistances[8];
    // maxDist s³u¿y do odwracania odleg³oœci
    // Mo¿na by by³o u¿yæ rzeczywistej maksymalnej odleg³oœci, ale wówczas
    // próbka mia³aby wagê 0, u¿yj 1.0.
    //float maxDist = sqrt(0.5*0.5*2);
    float maxDist = 1.0f;

    // Zsumowanie odleg³oœci w celu obliczenia sumy ca³kowitej u¿ytej do obliczania wag
    // Odwrócenie, aby próbki bli¿sze œrodka mia³y wiêksz¹ wagê
    for(int i=0; i<8; i++)
    {
        double xDist = positions[i*2  ]-0.5;
        double yDist = positions[i*2+1]-0.5;
        invertedSampleDistances[i] = maxDist - sqrt(xDist*xDist + yDist*yDist);
    }
    
    // Wyzerowanie tablicy wag próbek
    for(int i=0; i<8; i++)
    {
        for(int j=0; j<8; j++)
        {
            sampleWeights[i][j] = 0.0f;
        }
    }

    // Pierwsza tablica jest ³atwa, 1 próbka, a wiêc jej waga wynosi 1
    sampleWeights[0][0] = 1.0f;

    // Sumowanie odleg³oœci 
    for(int i=1; i<8; i++)
    {
        float totalWeight = 0.0f;
        for(int j=0; j<=i; j++)
            totalWeight += invertedSampleDistances[j];

        // Invert to get the factor used for each sample, the sum of all sample weights is always 1.0
        float perSampleFactor = 1 / totalWeight;
        for(int j=0; j<=i; j++)
            sampleWeights[i][j] = invertedSampleDistances[j] * perSampleFactor;
    }
    
    // Ustawienie obiektu bufora tekstury do przechowywania wag próbek
    glGenBuffers(1, &sampleWeightBuf);
    glBindBuffer(GL_TEXTURE_BUFFER_ARB, sampleWeightBuf);
    glBufferData(GL_TEXTURE_BUFFER_ARB, sizeof(float)*8, sampleWeights, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_TEXTURE_BUFFER_ARB, 0);

    // Za³adowanie TBO do tekstury 1
	glActiveTexture(GL_TEXTURE1);
    glGenTextures(1, &texBOTexture);
	glBindTexture(GL_TEXTURE_BUFFER_ARB, texBOTexture);
	glTexBufferARB(GL_TEXTURE_BUFFER_ARB, GL_R32F, sampleWeightBuf); 
	glActiveTexture(GL_TEXTURE0);

    // Reset wi¹zañ bufora obrazu
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

}

////////////////////////////////////////////////////////////////////////////
// Pobranie nazwy/lokalizacji pliku i za³adowanie OpenEXR
// Za³adowanie obrazu do obiektu tekstury i przekazanie z powrotem rozmiarów tekstury
// 
bool LoadOpenEXRImage(char *fileName, GLint textureName, GLuint &texWidth, GLuint &texHeight)
{
    // W OpenEXR b³êdy s¹ obs³ugiwane za pomoc¹ mechanizmu wyj¹tków
    // Wszystkie dzia³ania wykonuj w bloku try, aby móc przechwyciæ wszystkie wyj¹tki.
    try
    {
        Imf::Array2D<Imf::Rgba> pixels;
        Imf::RgbaInputFile file (fileName);
        Imath::Box2i dw = file.dataWindow();

        texWidth  = dw.max.x - dw.min.x + 1;
        texHeight = dw.max.y - dw.min.y + 1;
        
        pixels.resizeErase (texHeight, texWidth); 

        file.setFrameBuffer (&pixels[0][0] - dw.min.x - dw.min.y * texWidth, 1, texWidth);
        file.readPixels (dw.min.y, dw.max.y); 

        GLfloat* texels = (GLfloat*)malloc(texWidth * texHeight * 3 * sizeof(GLfloat));
        GLfloat* pTex = texels;

        // Kopiowanie OpenEXR do lokalnego bufora w celu wczytania do tekstury
        for (unsigned int v = 0; v < texHeight; v++)
        {
            for (unsigned int u = 0; u < texWidth; u++)
            {
                Imf::Rgba texel = pixels[texHeight - v - 1][u];  
                pTex[0] = texel.r;
                pTex[1] = texel.g;
                pTex[2] = texel.b;

                pTex += 3;
            }
        }

        // Wi¹zanie tekstury, wczytanie obrazu, ustawienie stanu tekstury
        glBindTexture(GL_TEXTURE_2D, textureName);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, texWidth, texHeight, 0, GL_RGB, GL_FLOAT, texels);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        
        free(texels);
    }
    catch(Iex::BaseExc & e)  
    {
        std::cerr << e.what() << std::endl;
        //
        // Obs³uga wyj¹tku
        //
    }

    return true;
}




///////////////////////////////////////////////////////////////////////////////
// Procedury porz¹dkowe, zwalnianie tekstur, obiektów bufora itp.
void ShutdownRC(void)
{
    // Upewnienie siê, ¿e domyœlny FBO jest zwi¹zany
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

    // Czyszczenie tekstur
    for (int i=0; i<5;i++)
    {
        glActiveTexture(GL_TEXTURE0+i);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    glDeleteTextures(1, textures);
    glDeleteTextures(1, hdrTextures);
    glDeleteTextures(1, &texBOTexture);
    glDeleteTextures(1, &windowTexture);

    // Czyszczenie buforów
    glDeleteBuffers(1, &sampleWeightBuf);

    // Czyszczenie FBO
    glDeleteFramebuffers(1, hdrFBO);
}
void SetupFlatColorProg(GLfloat *vLightPos, GLfloat *vColor)
{
    glUseProgram(flatColorProg);

    // Konfiguracja macierzy rzutowania
    glUniformMatrix4fv(glGetUniformLocation(flatColorProg, "pMatrix"), 
        1, GL_FALSE, transformPipeline.GetProjectionMatrix());

    // Macierz rzutowania model-widok
    glUniformMatrix4fv(glGetUniformLocation(flatColorProg, "mvMatrix"), 
        1, GL_FALSE, transformPipeline.GetModelViewMatrix());

    // Ustawienie po³o¿enia Ÿród³a œwiat³a
    glUniform3fv(glGetUniformLocation(flatColorProg, "vLightPos"), 1, vLightPos);

    // Ustawienie koloru
    glUniform4fv(glGetUniformLocation(flatColorProg, "vColor"), 1, vColor);

    gltCheckErrors(flatColorProg);
}

void SetupTexReplaceProg(GLfloat *vLightPos, GLfloat *vColor)
{
    glUseProgram(texReplaceProg);

    // Konfiguracja macierzy rzutowania
    glUniformMatrix4fv(glGetUniformLocation(texReplaceProg, "pMatrix"), 
        1, GL_FALSE, transformPipeline.GetProjectionMatrix());

    // Macierz rzutowania model-widok
    glUniformMatrix4fv(glGetUniformLocation(texReplaceProg, "mvMatrix"), 
        1, GL_FALSE, transformPipeline.GetModelViewMatrix());

    // Ustawienie po³o¿enia Ÿród³a œwiat³a
    glUniform3fv(glGetUniformLocation(texReplaceProg, "vLightPos"), 1, vLightPos);
    
    // Ustawienie koloru
    glUniform4fv(glGetUniformLocation(texReplaceProg, "vColor"), 1, vColor);

    // Ustawienie jednostki teksturuj¹cej
    glUniform1i(glGetUniformLocation(texReplaceProg, "textureUnit0"), 0);

    gltCheckErrors(texReplaceProg);

}

void SetupHDRProg()
{
    glUseProgram(hdrResolve);

    // Konfiguracja macierzy rzutowania
    glUniformMatrix4fv(glGetUniformLocation(hdrResolve, "pMatrix"), 
        1, GL_FALSE, transformPipeline.GetProjectionMatrix());

    // Macierz rzutowania model-widok
    glUniformMatrix4fv(glGetUniformLocation(hdrResolve, "mvMatrix"), 
        1, GL_FALSE, transformPipeline.GetModelViewMatrix());

    // Ustawienie zmiennych uniform kontrolowanych przez u¿ytkownika
    glUniform1fv(glGetUniformLocation(hdrResolve, "exposure"), 1, &exposure);

    // Ustawienie zmiennych uniform tekstury
    glUniform1i(glGetUniformLocation(hdrResolve, "origImage"), 0);
    glUniform1i(glGetUniformLocation(hdrResolve, "sampleWeightSampler"), 1);

    // Setup MS sepcific uniforms
    glUniform1i(glGetUniformLocation(hdrResolve, "sampleCount"), sampleCount);
    glUniform1i(glGetUniformLocation(hdrResolve, "useWeightedResolve"), useWeightedResolve);

    // Konfiguracja odpowiednich tekstur
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, hdrTextures[0]);

    // Sprawdzenie czy bufor wag próbek wymaga aktualizacji
    if (sampleCount != lastSampleCount)
    {
        glBindBuffer(GL_TEXTURE_BUFFER_ARB, sampleWeightBuf);
        void *data = glMapBufferRange(GL_TEXTURE_BUFFER_ARB, 0, sizeof(float)*8,
                         (GL_MAP_WRITE_BIT |GL_MAP_INVALIDATE_RANGE_BIT));
        memcpy(data, (void*)sampleWeights[sampleCount],sizeof(float)*8);
        glUnmapBuffer(GL_TEXTURE_BUFFER_ARB);
        lastSampleCount = sampleCount;
    }

    gltCheckErrors(hdrResolve);
} 

///////////////////////////////////////////////////////////////////////////////
// Ta funkcja jest wywo³ywana przynajmniej raz i przed ka¿d¹ operacj¹ renderowania. Jeœli ekran
// jest mog¹cym zmieniaæ rozmiar oknem, to funkcja ta zostanie wywo³ana dla ka¿dej zmiany
// rozmiaru okna.
void ChangeSize(int nWidth, int nHeight)
{
    glViewport(0, 0, nWidth, nHeight);
    transformPipeline.SetMatrixStacks(modelViewMatrix, projectionMatrix);
 
    viewFrustum.SetPerspective(35.0f, float(nWidth)/float(nHeight), 1.0f, 100.0f);
    projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
    modelViewMatrix.LoadIdentity();

    // Aktualizacja wymiarów ekranu
    screenWidth = nWidth;
    screenHeight = nHeight;

    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, hdrTextures[0]);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 8, GL_RGB16F, screenWidth, screenHeight, GL_FALSE);

    GenerateOrtho2DMat(nWidth, nHeight);
    glUseProgram(0);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////
// Utworzenie macierzy odwzorowuj¹c¹ geometriê na ekran. Jedna jednostka w kierunku x równa siê jednemu pikselowi
// szerokoœci. To samo dotyczy kierunku y.
//
void GenerateOrtho2DMat(GLuint imageWidth, GLuint imageHeight)
{
    float right = (float)imageWidth;
    float quadWidth = right;
    float left  = 0.0f;
    float top = (float)imageHeight;
    float quadHeight = top;
    float bottom = 0.0f;

    // Macierz rzutowania ortonormalnego
    orthoMatrix[0] = (float)(2 / (right));
    orthoMatrix[1] = 0.0;
    orthoMatrix[2] = 0.0;
    orthoMatrix[3] = 0.0;

    orthoMatrix[4] = 0.0;
    orthoMatrix[5] = (float)(2 / (top));
    orthoMatrix[6] = 0.0;
    orthoMatrix[7] = 0.0;

    orthoMatrix[8] = 0.0;
    orthoMatrix[9] = 0.0;
    orthoMatrix[10] = (float)(-2 / (1.0 - 0.0));
    orthoMatrix[11] = 0.0;

    orthoMatrix[12] = -1.0f;
    orthoMatrix[13] = -1.0f;
    orthoMatrix[14] = -1.0f;
    orthoMatrix[15] =  1.0;

    
    screenQuad.Reset();
    screenQuad.Begin(GL_TRIANGLE_STRIP, 4, 1);
        screenQuad.Color4f(0.0f, 1.0f, 0.0f, 1.0f);
        screenQuad.MultiTexCoord2f(0, 0.0f, 0.0f); 
        screenQuad.Vertex3f(0.0f, 0.0f, 0.0f);

        screenQuad.Color4f(0.0f, 1.0f, 0.0f, 1.0f);
        screenQuad.MultiTexCoord2f(0, 1.0f, 0.0f);
        screenQuad.Vertex3f(right, 0.0f, 0.0f);

        screenQuad.Color4f(0.0f, 1.0f, 0.0f, 1.0f);
        screenQuad.MultiTexCoord2f(0, 0.0f, 1.0f);
        screenQuad.Vertex3f(0.0f, top, 0.0f);

        screenQuad.Color4f(0.0f, 1.0f, 0.0f, 1.0f);
        screenQuad.MultiTexCoord2f(0, 1.0f, 1.0f);
        screenQuad.Vertex3f(right, top, 0.0f);
    screenQuad.End();
}

///////////////////////////////////////////////////////////////////////////////
// Aktualizacja kamery na podstawie danych od u¿ytkownika, prze³¹czanie trybów wyœwietlania
// 
void SpecialKeys(int key, int x, int y)
{ 
    static CStopWatch timer;
    float fTime = timer.GetElapsedSeconds();
    float linear = fTime / 100;
    float smallLinear = fTime / 1000;

    // Zwiêkszenie ekspozycji sceny
    if(key == GLUT_KEY_UP)
    {
        if((exposure + smallLinear) < 20.0f)
            exposure += smallLinear;
    }
    // Zmniejszenie ekspozycji sceny
    if(key == GLUT_KEY_DOWN)
    {
        if((exposure - smallLinear) > 0.01f)
            exposure -= smallLinear;
    }
}


void ProcessKeys(unsigned char key, int x, int y)
{
    if (key == 'q' || key ==  'Q')
    {
        useWeightedResolve = 0;
    }
    if (key == 'w' || key == 'W')
    {
        useWeightedResolve = 1;
    }

    if (key == '1')
        sampleCount = 0;
    else if (key == '2')
        sampleCount = 1;
    else if (key == '3')
        sampleCount = 2;
    else if (key == '4')
        sampleCount = 3;
    else if (key == '5')
        sampleCount = 4;
    else if (key == '6')
        sampleCount = 5;
    else if (key == '7')
        sampleCount = 6;
    else if (key == '8')
        sampleCount = 7;
}



///////////////////////////////////////////////////////////////////////////////
// Rednerowanie klatki. Szkielet odpowiada za zamiany buforów,
// opró¿niania buforów itp.
void RenderScene(void)
{
    // Najpierw renderowanie sceny w HDR do FBO
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, hdrFBO[0]);
    glDrawBuffers(1, &fboBuffs[0]);
    glClearColor(vSkyBlue[0], vSkyBlue[1], vSkyBlue[2], vSkyBlue[3]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Rysowanie do dwóch tekstur, pierwsza zawiera dane sceny
    // druga zawiera tylko jasne obszary
    modelViewMatrix.PushMatrix();	
        M3DMatrix44f mCamera;
        cameraFrame.GetCameraMatrix(mCamera);
        modelViewMatrix.MultMatrix(mCamera);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textures[0]); // Marmur

        // Rysowanie pod³ogi
        SetupTexReplaceProg(vLightPos, vWhite);
        floorBatch.Draw();

        // Rysowanie okna
        modelViewMatrix.PushMatrix();
            modelViewMatrix.Translate(0.0f, -0.4f, -4.0f);
            modelViewMatrix.Rotate(10.0, 0.0, 1.0, 0.0);
            glBindTexture(GL_TEXTURE_2D, windowTexture); 
            
            // Najpierw rysowanie zawartoœci okna z tekstury
            SetupTexReplaceProg(vLightPos, vWhiteX2);
            windowBatch.Draw();

            // Rysowanie obramowania i siatki
            SetupFlatColorProg(vLightPos, vLtGrey);
            windowGridBatch.Draw();
            windowBorderBatch.Draw();
            
        modelViewMatrix.PopMatrix();
    modelViewMatrix.PopMatrix();

    projectionMatrix.PushMatrix();
        projectionMatrix.LoadMatrix(orthoMatrix);
        modelViewMatrix.PushMatrix();
            modelViewMatrix.LoadIdentity();
            
            // Tworzy kombinacjê oryginalnej sceny z rozmazanymi jasnymi teksturami
            // w celu uzyskania efektu poœwiaty
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
            glDrawBuffers(1,windowBuff);
            glViewport(0, 0, screenWidth, screenHeight);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            SetupHDRProg();
            screenQuad.Draw();
        modelViewMatrix.PopMatrix();
    projectionMatrix.PopMatrix();
    
    // Przywrócenie jednostek teksturowych do dawnego stanu
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
    
    // Zamiana buforów
    glutSwapBuffers();
        
    // Jeszcze raz
    glutPostRedisplay();
}

int main(int argc, char* argv[])
{
    screenWidth = 800;
    screenHeight = 600; 
    sampleCount = 0;
    useWeightedResolve = 1;

	gltSetWorkingDirectory(argv[0]);
		
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(screenWidth,screenHeight);
  
    glutCreateWindow("HDR MSAA");
 
    glutReshapeFunc(ChangeSize);
    glutDisplayFunc(RenderScene);
    glutSpecialFunc(SpecialKeys);
    glutKeyboardFunc(ProcessKeys);

    SetupRC();
    glutMainLoop();    
    ShutdownRC();
    return 0;
}