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
static GLfloat vGrey[] =  { 0.5f, 0.5f, 0.5f, 1.0f };
static GLfloat vLightPos[] = { -2.0f, 3.0f, -2.0f, 1.0f };
static GLfloat vSkyBlue[] = { 0.160f, 0.376f, 0.925f, 1.0f};

static const GLenum windowBuff[] = { GL_BACK_LEFT };
static const GLenum fboBuffs[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };

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
GLuint				hdrBloomProg;
GLuint				blurProg;

GLuint              hdrFBO[1];
GLuint              brightPassFBO[4];
GLuint				textures[1];
GLuint				hdrTextures[1];
GLuint				brightBlurTextures[5];
GLuint				windowTexture;
GLfloat				exposure;
GLfloat				bloomLevel;
GLfloat				texCoordOffsets[5*5*2];
void UpdateMode(void);
void GenerateOrtho2DMat(GLuint imageWidth, GLuint imageHeight);
bool LoadBMPTexture(const char *szFileName, GLenum minFilter, GLenum magFilter, GLenum wrapMode);
bool LoadOpenEXRImage(char *fileName, GLint textureName, GLuint &texWidth, GLuint &texHeight);
void GenTexCoordOffsets(GLuint width, GLuint height);
void SetupTexReplaceProg(GLfloat *vLightPos, GLfloat *vColor);
void SetupFlatColorProg(GLfloat *vLightPos, GLfloat *vColor);
void SetupHDRProg();
void SetupBlurProg();


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
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, iWidth, iHeight, 0, GL_BGR, GL_UNSIGNED_BYTE, pBits);

    // Czy konieczne jest generowanie mipmap?
	if(minFilter == GL_LINEAR_MIPMAP_LINEAR || minFilter == GL_LINEAR_MIPMAP_NEAREST || minFilter == GL_NEAREST_MIPMAP_LINEAR || minFilter == GL_NEAREST_MIPMAP_NEAREST)
		glGenerateMipmap(GL_TEXTURE_2D);    

	return true;
}



void GenTexCoordOffsets(GLuint width, GLuint height)
{
	float xInc = 1.0f / (GLfloat)(width);
	float yInc = 1.0f / (GLfloat)(height);

	for (int i = 0; i < 5; i++)
	{
		for (int j = 0; j < 5; j++)
		{
			texCoordOffsets[(((i*5)+j)*2)+0] = (-2.0f * xInc) + ((GLfloat)i * xInc);
			texCoordOffsets[(((i*5)+j)*2)+1] = (-2.0f * yInc) + ((GLfloat)j * yInc);
		}
	}
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
	bloomLevel = 0.5;

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
		floorBatch.MultiTexCoord2f(0, 10.0f, 0.0f);
		floorBatch.Normal3f(0.0, 1.0f, 0.0f);
		floorBatch.Vertex3f(20.0f, -0.41f, 20.0f);

		floorBatch.Color4f(0.0f, 1.0f, 0.0f, alpha);
		floorBatch.MultiTexCoord2f(0, 10.0f, 10.0f);
		floorBatch.Normal3f(0.0, 1.0f, 0.0f);
		floorBatch.Vertex3f(20.0f, -0.41f, -20.0f);

		floorBatch.Color4f(0.0f, 1.0f, 0.0f, alpha);
		floorBatch.MultiTexCoord2f(0, 0.0f, 10.0f);
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
	const float gridWidth = (float)0.05;
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

	windowGridBatch.Begin(GL_TRIANGLES, 24);
		// dó³ poziom
		windowGridBatch.Normal3f( 0.0f, 0.0f, 1.0f);
		windowGridBatch.Vertex3f(-1.0f, 0.7+gridWidth, 0.01f);

		windowGridBatch.Normal3f(0.0f, 0.0f, 1.0f);
		windowGridBatch.Vertex3f(-1.0f, 0.7-gridWidth, 0.01f);

		windowGridBatch.Normal3f(0.0f, 0.0f, 1.0f);
		windowGridBatch.Vertex3f(1.0f, 0.7-gridWidth, 0.01f);

		windowGridBatch.Normal3f(0.0f, 0.0f, 1.0f);
		windowGridBatch.Vertex3f(1.0f, 0.7-gridWidth, 0.01f);

		windowGridBatch.Normal3f(0.0f, 0.0f, 1.0f);
		windowGridBatch.Vertex3f(1.0f, 0.7+gridWidth, 0.01f);

		windowGridBatch.Normal3f( 0.0f, 0.0f, 1.0f);
		windowGridBatch.Vertex3f(-1.0f, 0.7+gridWidth, 0.01f);
		
		// góra poziom
		windowGridBatch.Normal3f( 0.0f, 0.0f, 1.0f);
		windowGridBatch.Vertex3f(-1.0f, 1.3+gridWidth, 0.01f);

		windowGridBatch.Normal3f(0.0f, 0.0f, 1.0f);
		windowGridBatch.Vertex3f(-1.0f, 1.3-gridWidth, 0.01f);

		windowGridBatch.Normal3f(0.0f, 0.0f, 1.0f);
		windowGridBatch.Vertex3f(1.0f, 1.3-gridWidth, 0.01f);

		windowGridBatch.Normal3f(0.0f, 0.0f, 1.0f);
		windowGridBatch.Vertex3f(1.0f, 1.3-gridWidth, 0.01f);

		windowGridBatch.Normal3f(0.0f, 0.0f, 1.0f);
		windowGridBatch.Vertex3f(1.0f, 1.3+gridWidth, 0.01f);

		windowGridBatch.Normal3f( 0.0f, 0.0f, 1.0f);
		windowGridBatch.Vertex3f(-1.0f, 1.3+gridWidth, 0.01f);
		
		// lewa pion
		windowGridBatch.Normal3f( 0.0f, 0.0f, 1.0f);
		windowGridBatch.Vertex3f(-0.3+gridWidth, 0.0f, 0.01f);

		windowGridBatch.Normal3f(0.0f, 0.0f, 1.0f);
		windowGridBatch.Vertex3f(-0.3-gridWidth, 0.0f, 0.01f);

		windowGridBatch.Normal3f(0.0f, 0.0f, 1.0f);
		windowGridBatch.Vertex3f(-0.3-gridWidth, 2.0f, 0.01f);

		windowGridBatch.Normal3f(0.0f, 0.0f, 1.0f);
		windowGridBatch.Vertex3f(-0.3-gridWidth, 2.0f, 0.01f);

		windowGridBatch.Normal3f(0.0f, 0.0f, 1.0f);
		windowGridBatch.Vertex3f(-0.3+gridWidth, 2.0, 0.01f);

		windowGridBatch.Normal3f( 0.0f, 0.0f, 1.0f);
		windowGridBatch.Vertex3f(-0.3+gridWidth, 0.0f, 0.01f);

		// prawa pion
		windowGridBatch.Normal3f( 0.0f, 0.0f, 1.0f);
		windowGridBatch.Vertex3f(0.3+gridWidth, 0.0f, 0.01f);

		windowGridBatch.Normal3f(0.0f, 0.0f, 1.0f);
		windowGridBatch.Vertex3f(0.3-gridWidth, 0.0f, 0.01f);

		windowGridBatch.Normal3f(0.0f, 0.0f, 1.0f);
		windowGridBatch.Vertex3f(0.3-gridWidth, 2.0f, 0.01f);

		windowGridBatch.Normal3f(0.0f, 0.0f, 1.0f);
		windowGridBatch.Vertex3f(0.3-gridWidth, 2.0f, 0.01f);

		windowGridBatch.Normal3f(0.0f, 0.0f, 1.0f);
		windowGridBatch.Vertex3f(0.3+gridWidth, 2.0, 0.01f);

		windowGridBatch.Normal3f( 0.0f, 0.0f, 1.0f);
		windowGridBatch.Vertex3f(0.3+gridWidth, 0.0f, 0.01f);
	windowGridBatch.End();

	glGenTextures(1, textures);
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	LoadBMPTexture("marble.bmp", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_REPEAT);

	// Konfiguracja tekstur HDR
	glGenTextures(1, hdrTextures);
	glBindTexture(GL_TEXTURE_2D, hdrTextures[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, screenWidth, screenHeight, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		
	// Utworzenie tekstur do efektu poœwiaty
	glGenTextures(5, brightBlurTextures);
	int i = 0;
	for (i=0; i<5; i++)
	{
		glBindTexture(GL_TEXTURE_2D, brightBlurTextures[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, screenWidth, screenHeight, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	}

	// Wi¹zanie tekstury HDR do FBO
	// Utworzenie i zwi¹zanie FBO
	glGenFramebuffers(1,hdrFBO);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, hdrFBO[0]);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, hdrTextures[0], 0);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, brightBlurTextures[0], 0);

	// Utworzenie FBO do efektu poœwiaty
	glGenFramebuffers(4,brightPassFBO);
	for (i=0; i<4; i++)
	{
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, brightPassFBO[i]);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brightBlurTextures[i+1], 0);
	}

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
	hdrBloomProg =  gltLoadShaderPairWithAttributes("basic.vs", "hdr_exposure.fs", 3, 
							GLT_ATTRIBUTE_VERTEX, "vVertex", 
							GLT_ATTRIBUTE_NORMAL, "vNormal", 
							GLT_ATTRIBUTE_TEXTURE0, "vTexCoord0");
	glBindFragDataLocation(hdrBloomProg, 0, "oColor");
	glLinkProgram(hdrBloomProg);

	// Za³adowanie shadera rozmazania
	blurProg =  gltLoadShaderPairWithAttributes("basic.vs", "blur.fs", 2,
							GLT_ATTRIBUTE_VERTEX, "vVertex", 
							GLT_ATTRIBUTE_TEXTURE0, "vTexCoord0");
	glBindFragDataLocation(blurProg, 0, "oColor");
	glLinkProgram(blurProg);
	glUseProgram(blurProg);
	
	// Ustawienie wspó³rzêdnych teksturowych, które zostan¹ u¿yte do pobierania danych j¹dra HDR
	GenTexCoordOffsets(screenWidth, screenHeight);
	glUniform2fv(glGetUniformLocation(blurProg, "tc_offset"), 25, &texCoordOffsets[0]);

	// Upewnienie siê, czy wszystko posz³o dobrze
	gltCheckErrors(flatColorProg);
	gltCheckErrors(texReplaceProg);
	gltCheckErrors(hdrBloomProg);
	gltCheckErrors(blurProg);
	
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
	glDeleteTextures(5, brightBlurTextures);
	glDeleteTextures(1, &windowTexture);

	// Czyszczenie FBO
	glDeleteFramebuffers(1, hdrFBO);
	glDeleteFramebuffers(1, brightPassFBO);

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
	glUseProgram(hdrBloomProg);

	// Konfiguracja macierzy rzutowania
	glUniformMatrix4fv(glGetUniformLocation(hdrBloomProg, "pMatrix"), 
		1, GL_FALSE, transformPipeline.GetProjectionMatrix());

	// Macierz rzutowania model-widok
	glUniformMatrix4fv(glGetUniformLocation(hdrBloomProg, "mvMatrix"), 
		1, GL_FALSE, transformPipeline.GetModelViewMatrix());

	// Ustawienie zmiennych uniform kontrolowanych przez u¿ytkownika
	glUniform1fv(glGetUniformLocation(hdrBloomProg, "exposure"), 1, &exposure);
	glUniform1fv(glGetUniformLocation(hdrBloomProg, "bloomLevel"), 1, &bloomLevel);

	// Ustawienie zmiennych uniform tekstury
	glUniform1i(glGetUniformLocation(hdrBloomProg, "origImage"), 0);
	glUniform1i(glGetUniformLocation(hdrBloomProg, "brightImage"), 1);
	glUniform1i(glGetUniformLocation(hdrBloomProg, "blur1"), 2);
	glUniform1i(glGetUniformLocation(hdrBloomProg, "blur2"), 3);
	glUniform1i(glGetUniformLocation(hdrBloomProg, "blur3"), 4);
	glUniform1i(glGetUniformLocation(hdrBloomProg, "blur4"), 5);

	// Konfiguracja odpowiednich tekstur
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, hdrTextures[0]);
	for (int i=0; i<5; i++)
	{
		glActiveTexture(GL_TEXTURE1+i);
		glBindTexture(GL_TEXTURE_2D, brightBlurTextures[i]);
	}
	gltCheckErrors(hdrBloomProg);
} 

void SetupBlurProg()
{
    // Ustawienie programu na bie¿¹cy
	glUseProgram(blurProg);

	// Konfiguracja macierzy rzutowania
	glUniformMatrix4fv(glGetUniformLocation(blurProg, "pMatrix"), 
		1, GL_FALSE, transformPipeline.GetProjectionMatrix());

	// Macierz rzutowania model-widok
	glUniformMatrix4fv(glGetUniformLocation(blurProg, "mvMatrix"), 
		1, GL_FALSE, transformPipeline.GetModelViewMatrix());

	glUniform1i(glGetUniformLocation(blurProg, "textureUnit0"), 0);
 
	gltCheckErrors(blurProg);
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

	glBindTexture(GL_TEXTURE_2D, hdrTextures[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, screenWidth, screenHeight, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	glBindTexture(GL_TEXTURE_2D, brightBlurTextures[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, screenWidth, screenHeight, 0, GL_RGBA, GL_FLOAT, NULL);

	for(int i=1; i<5; i++)
		{		
		glBindTexture(GL_TEXTURE_2D, brightBlurTextures[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, screenWidth/(i*3.0), screenHeight/(i*3.0), 0, GL_RGBA, GL_FLOAT, NULL);
		}

	// Ustawienie wspó³rzêdnych teksturowych, które zostan¹ u¿yte do pobierania danych j¹dra HDR
	glUseProgram(blurProg);
	GenTexCoordOffsets(screenWidth, screenHeight);
	glUniform2fv(glGetUniformLocation(blurProg, "tc_offset"), 25, texCoordOffsets);

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
		if((exposure + linear) < 20.0f)
			exposure += linear;
	}
	// Zmniejszenie ekspozycji sceny
	if(key == GLUT_KEY_DOWN)
	{
		if((exposure - linear) > 0.01f)
			exposure -= linear;
	}
	
	// Zmniejszenie efektu poœwiaty
	if(key == GLUT_KEY_LEFT)
	{
		if((bloomLevel - smallLinear) > 0.00f)
			bloomLevel -= smallLinear;
		
	}
	// Zwiêkszenie efektu poœwiaty
	if(key == GLUT_KEY_RIGHT)
	{
		if((bloomLevel + smallLinear) < 1.5f)
			bloomLevel += smallLinear;
	}
}


///////////////////////////////////////////////////////////////////////////////
// Rednerowanie klatki. Szkielet odpowiada za zamiany buforów,
// opró¿niania buforów itp.
void RenderScene(void)
{
	int i =0;

	// Najpierw renderowanie sceny w HDR do FBO
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, hdrFBO[0]);
	glDrawBuffers(1, &fboBuffs[0]);
	glClearColor(vSkyBlue[0], vSkyBlue[1], vSkyBlue[2], vSkyBlue[3]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glDrawBuffers(1, &fboBuffs[1]);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Rysowanie do dwóch tekstur, pierwsza zawiera dane sceny
	// druga zawiera tylko jasne obszary
	glDrawBuffers(2, fboBuffs);
	modelViewMatrix.PushMatrix();	
		M3DMatrix44f mCamera;
		cameraFrame.GetCameraMatrix(mCamera);
		modelViewMatrix.MultMatrix(mCamera);
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
			SetupFlatColorProg(vLightPos, vGrey);
			windowGridBatch.Draw();
			windowBorderBatch.Draw();
		modelViewMatrix.PopMatrix();
	modelViewMatrix.PopMatrix();

	projectionMatrix.PushMatrix();
		projectionMatrix.LoadMatrix(orthoMatrix);
		modelViewMatrix.PushMatrix();
			modelViewMatrix.LoadIdentity();
			
			// Pobranie danych z tekstur jasnoœci
			// rozmazanie ich w 4 kolejnych przekazaniach do tekstur
			// o coraz mniejszym rozmiarze
			SetupBlurProg();
			for (i =0; i<4; i++)
			{
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, brightPassFBO[i]); // Rysuje do brightBlurTextures[i+1]
				glDrawBuffers(1, &fboBuffs[0]);
				glViewport(0, 0, screenWidth/((i+1)*3.0), screenHeight/((i+1)*3.0));
				glBindTexture(GL_TEXTURE_2D, brightBlurTextures[i]);
				screenQuad.Draw();
			}

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
	for (i=5; i>-1; i--)
	{
		glActiveTexture(GL_TEXTURE0+i);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
    
    // Zamiana buforów
    glutSwapBuffers();
        
    // Jeszcze raz
    glutPostRedisplay();
}


int main(int argc, char* argv[])
{
    screenWidth = 800;
    screenHeight = 600;
    bFullScreen = false; 
    bAnimated = true;
    bloomLevel = 0.0;

    cameraFrame.MoveUp(0.50);

	gltSetWorkingDirectory(argv[0]);
		
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(screenWidth,screenHeight);
  
    glutCreateWindow("HDR Bloom");
 
    glutReshapeFunc(ChangeSize);
    glutDisplayFunc(RenderScene);
    glutSpecialFunc(SpecialKeys);

    SetupRC();
    glutMainLoop();    
    ShutdownRC();
    return 0;
}