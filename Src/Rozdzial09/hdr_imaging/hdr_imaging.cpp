#include <stdio.h>
#include <iostream>
#include <ImfRgbaFile.h>            // Nag³ówki OpenEXR
#include <ImfArray.h>

#include <GLTools.h>
#include <GLShaderManager.h>
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

#pragma warning (disable : 4305)

GLsizei	 screenWidth;			// ¯¹dana szerokoœæ okna lub pulpitu
GLsizei  screenHeight;			// ¯¹dana wysokoœæ okna lub pulpitu

GLboolean bFullScreen;			// ¯¹danie dzia³ania w trybie pe³noekranowym
GLboolean bAnimated;			// ¯¹danie ci¹g³ych aktualizacji

GLMatrixStack		modelViewMatrix;		// Macierz model-widok
GLMatrixStack		projectionMatrix;		// Macierz rzutowania
GLGeometryTransform	transformPipeline;		// Potok przetwarzania geometrii
GLBatch             screenQuad;
GLBatch             fboQuad;
M3DMatrix44f        orthoMatrix;  
M3DMatrix44f        fboOrthoMatrix; 

GLuint				hdrTextures[1];
GLuint				lutTxtures[1];
GLuint				fboTextures[1];
GLuint				hdrTexturesWidth[1];
GLuint				hdrTexturesHeight[1];
GLuint				curHDRTex;
GLuint				fboName;
GLuint              mapTexProg;
GLuint              varExposureProg;
GLuint              adaptiveProg;
GLuint              curProg;
GLfloat				exposure;

void GenerateOrtho2DMat(GLuint windowWidth, GLuint windowHeight, GLuint imageWidth, GLuint imageHeight);
void GenerateFBOOrtho2DMat(GLuint imageWidth, GLuint imageHeight);
void SetupHDRProg();
void SetupStraightTexProg();
bool LoadOpenEXRImage(char *fileName, GLint textureName, GLuint &texWidth, GLuint &texHeight);

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
// Tu mo¿na bezpiecznie umieœciæ kod innicjuj¹cy OpenGL, ³adowanie tekstur itp.
void SetupRC(void)
{
	GLfloat texCoordOffsets[4][5*5*2];
	GLfloat exposureLUT[20]   = { 11.0, 6.0, 3.2, 2.8, 2.2, 1.90, 1.80, 1.80, 1.70, 1.70,  1.60, 1.60, 1.50, 1.50, 1.40, 1.40, 1.30, 1.20, 1.10, 1.00 };
	
	// Upewnienie siê, czy punkty wejœciowe OpenGL s¹ ustawione
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		/* Problem: wywo³anie funkcji glewInit nie powiod³o siê */
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
	}

	// Nie u¿ywa bufora g³êbi
	glDisable(GL_DEPTH_TEST);
	curHDRTex = 0;
	
	// Inicjalizacja macierzy model-widok i pozostawienie jej 
	modelViewMatrix.LoadIdentity();

	// Czarny
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	// Ustawienie tekstury LUT do u¿ytku z adaptywnym filtrem ekspozycji
	glGenTextures(1, lutTxtures);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_1D, lutTxtures[1]);
	glTexImage1D(GL_TEXTURE_1D, 0, GL_R16F, 20,  0, GL_RED, GL_FLOAT, exposureLUT);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// Ustawienie tekstur HDR
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, hdrTextures);
	glBindTexture(GL_TEXTURE_2D, hdrTextures[curHDRTex]);

	// Wczytanie obrazu HDR z pliku EXR
    LoadOpenEXRImage("Tree.exr", hdrTextures[curHDRTex], hdrTexturesWidth[curHDRTex], hdrTexturesHeight[curHDRTex]);

	// Utworzenie macierzy rzutowania ortonormalnego i prostok¹ta o rozmiarze ekranu odpowiadaj¹cego wspó³czynnikowi proporcji obrazów
    GenerateOrtho2DMat(screenWidth, screenHeight, hdrTexturesWidth[curHDRTex], hdrTexturesHeight[curHDRTex]);
	//GenerateFBOOrtho2DMat(hdrTexturesWidth[curHDRTex], hdrTexturesHeight[curHDRTex]);
    gltGenerateOrtho2DMat(hdrTexturesWidth[curHDRTex], hdrTexturesHeight[curHDRTex], fboOrthoMatrix, fboQuad);

	// Ustawienie wspó³rzêdnych teksturowych, które zostan¹ u¿yte do pobierania danych j¹dra HDR
	for (int k = 0; k < 4; k++)
	{
		float xInc = 1.0f / (GLfloat)(hdrTexturesWidth[curHDRTex] >> k);
		float yInc = 1.0f / (GLfloat)(hdrTexturesHeight[curHDRTex] >> k);

		for (int i = 0; i < 5; i++)
		{
			for (int j = 0; j < 5; j++)
			{
				texCoordOffsets[k][(((i*5)+j)*2)+0] = (-1.0f * xInc) + ((GLfloat)i * xInc);
				texCoordOffsets[k][(((i*5)+j)*2)+1] = (-1.0f * yInc) + ((GLfloat)j * yInc);
			}
		}
	}

	// Za³adowanie shaderów
    mapTexProg =  gltLoadShaderPairWithAttributes("hdr.vs", "hdr_simple.fs", 2, GLT_ATTRIBUTE_VERTEX, "vVertex", GLT_ATTRIBUTE_TEXTURE0, "vTexCoord0");
	glBindFragDataLocation(mapTexProg, 0, "oColor");
	glLinkProgram(mapTexProg);

	varExposureProg =  gltLoadShaderPairWithAttributes("hdr.vs", "hdr_exposure.fs", 2, GLT_ATTRIBUTE_VERTEX, "vVertex", GLT_ATTRIBUTE_TEXTURE0, "vTexCoord0");
	glBindFragDataLocation(varExposureProg, 0, "oColor");
	glLinkProgram(varExposureProg);
	glUseProgram(varExposureProg);
	glUniform1i(glGetUniformLocation(varExposureProg, "textureUnit0"), 0);

	adaptiveProg =  gltLoadShaderPairWithAttributes("hdr.vs", "hdr_adaptive.fs", 2, GLT_ATTRIBUTE_VERTEX, "vVertex", GLT_ATTRIBUTE_TEXTURE0, "vTexCoord0");
	glBindFragDataLocation(adaptiveProg, 0, "oColor");
	glLinkProgram(adaptiveProg);
	glUseProgram(adaptiveProg);
	glUniform1i(glGetUniformLocation(adaptiveProg, "textureUnit0"), 0);
	glUniform1i(glGetUniformLocation(adaptiveProg, "textureUnit1"), 1);
	glUniform2fv(glGetUniformLocation(adaptiveProg, "tc_offset"), 25, texCoordOffsets[0]);

	glUseProgram(0);

	// Utworzenie i zwi¹zanie FBO
	glGenFramebuffers(1,&fboName);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboName);

	// Utworzenie tekstury FBO
	glGenTextures(1, fboTextures);
	glBindTexture(GL_TEXTURE_2D, fboTextures[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, hdrTexturesWidth[curHDRTex], hdrTexturesHeight[curHDRTex], 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTextures[0], 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
	// Upewnienie siê, czy wszystko posz³o dobrze
	gltCheckErrors();
	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	// Ustawienie pierwszego trybu dzia³ania
	curProg = adaptiveProg;
}


///////////////////////////////////////////////////////////////////////////////
// Procedury porz¹dkowe, zwalnianie tekstur, obiektów bufora itp.
void ShutdownRC(void)
{
	// Upewnienie siê, ¿e domyœlny FBO jest zwi¹zany
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

	glDeleteFramebuffers(1, &fboName);

	// Czyszczenie tekstur
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_1D, 0);
	
	glDeleteTextures(1, hdrTextures);
	glDeleteTextures(1, lutTxtures);
	glDeleteTextures(1, fboTextures);

}

///////////////////////////////////////////////////////////////////////////////////////////////////////
// Create a matrix that maps geometry to the screen. 1 unit in the x direction equals one pixel 
// szerokoœci. To samo dotyczy kierunku y.
// To zale¿y równie¿ od rozmiaru wyœwietlanej tekstury
void GenerateOrtho2DMat(GLuint windowWidth, GLuint windowHeight, GLuint imageWidth, GLuint imageHeight)
{
    float right = (float)windowWidth;
	float quadWidth = right;
	float left  = 0.0f;
	float top = (float)windowHeight;
	float quadHeight = top;
	float bottom = 0.0f;
	float screenAspect = (float)windowWidth/windowHeight;
	float imageAspect = (float)imageWidth/imageHeight;

	if (screenAspect > imageAspect)
		quadWidth = windowHeight*imageAspect;
	else
		quadHeight = windowWidth*imageAspect;

    // Macierz rzutowania ortonormalnego
	orthoMatrix[0] = (float)(2 / (right - left));
	orthoMatrix[1] = 0.0;
	orthoMatrix[2] = 0.0;
	orthoMatrix[3] = 0.0;

	orthoMatrix[4] = 0.0;
	orthoMatrix[5] = (float)(2 / (top - bottom));
	orthoMatrix[6] = 0.0;
	orthoMatrix[7] = 0.0;

	orthoMatrix[8] = 0.0;
	orthoMatrix[9] = 0.0;
	orthoMatrix[10] = (float)(-2 / (1.0 - 0.0));
	orthoMatrix[11] = 0.0;

	orthoMatrix[12] = -1*(right + left) / (right - left);
	orthoMatrix[13] = -1*(top + bottom) / (top - bottom);
	orthoMatrix[14] = -1.0f;
	orthoMatrix[15] =  1.0;

    
	screenQuad.Reset();
	screenQuad.Begin(GL_TRIANGLE_STRIP, 4, 1);
		screenQuad.Color4f(0.0f, 1.0f, 0.0f, 1.0f);
		screenQuad.MultiTexCoord2f(0, 0.0f, 0.0f); 
		screenQuad.Vertex3f(0.0f, 0.0f, 0.0f);

		screenQuad.Color4f(0.0f, 1.0f, 0.0f, 1.0f);
		screenQuad.MultiTexCoord2f(0, 1.0f, 0.0f);
		screenQuad.Vertex3f(quadWidth, 0.0f, 0.0f);

		screenQuad.Color4f(0.0f, 1.0f, 0.0f, 1.0f);
		screenQuad.MultiTexCoord2f(0, 0.0f, 1.0f);
		screenQuad.Vertex3f(0.0f, quadHeight, 0.0f);

		screenQuad.Color4f(0.0f, 1.0f, 0.0f, 1.0f);
		screenQuad.MultiTexCoord2f(0, 1.0f, 1.0f);
		screenQuad.Vertex3f(quadWidth, quadHeight, 0.0f);
	screenQuad.End();

}



///////////////////////////////////////////////////////////////////////////////
// Ta funkcja jest wywo³ywana przynajmniej raz i przed ka¿d¹ operacj¹ renderowania. Jeœli ekran
// jest mog¹cym zmieniaæ rozmiar oknem, to funkcja ta zostanie wywo³ana dla ka¿dej zmiany
// rozmiaru okna.
void ChangeSize(int nWidth, int nHeight)
{
	glViewport(0, 0, nWidth, nHeight);
	transformPipeline.SetMatrixStacks(modelViewMatrix, projectionMatrix);
 
	modelViewMatrix.LoadIdentity();

	// Aktualizacja wymiarów ekranu
	screenWidth = nWidth;
	screenHeight = nHeight;

    GenerateOrtho2DMat(nWidth, nHeight, hdrTexturesWidth[curHDRTex], hdrTexturesHeight[curHDRTex]);
}


///////////////////////////////////////////////////////////////////////////////
// Aktualizacja kamery na podstawie danych od u¿ytkownika, prze³¹czanie trybów wyœwietlania
// 
void SpecialKeys(int key, int x, int y)
{ 
	static CStopWatch timer;
	float fTime = timer.GetElapsedSeconds();
	float linear = fTime / 100;
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
}

void ProcessKeys(unsigned char key, int x, int y)
{
	if(key == '1')
	{
		curProg = mapTexProg;
	}
	if(key == '2')
	{
		curProg = varExposureProg;
	}
	if(key == '3')
	{
		curProg = adaptiveProg;
	}
}

void SetupStraightTexProg()
{
	// Ustawienie programu do zamiany tekstur
	glUseProgram(mapTexProg);

	// Macierz rzutowania model-widok
	glUniformMatrix4fv(glGetUniformLocation(mapTexProg, "mvpMatrix"), 1, GL_FALSE, transformPipeline.GetModelViewProjectionMatrix());
}

void SetupHDRProg()
{
    // Ustawienie programu na bie¿¹cy
	glUseProgram(curProg);

	// Macierz rzutowania model-widok
	glUniformMatrix4fv(glGetUniformLocation(curProg, "mvpMatrix"), 1, GL_FALSE, transformPipeline.GetModelViewProjectionMatrix());

	if (curProg == varExposureProg)
	{
		// Ustawienie ekspozycji
		glUniform1f(glGetUniformLocation(curProg, "exposure"), exposure);
	}	
}

///////////////////////////////////////////////////////////////////////////////
// Rednerowanie klatki. Szkielet odpowiada za zamiany buforów,
// opró¿niania buforów itp.
void RenderScene(void)
{
	static CStopWatch animationTimer;
	float yRot = animationTimer.GetElapsedSeconds() * 60.0f;

	// Najpierw rysowanie do FBO w pe³nej rozdzielczoœci FBO

	// Wi¹zanie FBO z oœmiobitowym punktem wi¹zania
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboName);
	glViewport(0, 0, hdrTexturesWidth[curHDRTex], hdrTexturesHeight[curHDRTex]);
	glClear(GL_COLOR_BUFFER_BIT);

	// Wi¹zanie tekstury z obrazem HDR
	glBindTexture(GL_TEXTURE_2D, hdrTextures[curHDRTex]);

	// Renderowanie, redukcja próbkowania do 8 bitów przy u¿yciu wybranego programu
	projectionMatrix.LoadMatrix(fboOrthoMatrix);
	SetupHDRProg();
	fboQuad.Draw();

	// Narysowanie obrazu na ekranie przy zachowaniu proporcji
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glViewport(0, 0, screenWidth, screenHeight);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	// Zwi¹zanie 8-bitowej tekstury z obrazem HDR
	glBindTexture(GL_TEXTURE_2D, fboTextures[0]);
	
	// Narysowanie proporcjonalnego prostok¹ta o rozmiarze ekranu z 8-bitow¹ tekstur¹
	projectionMatrix.LoadMatrix(orthoMatrix);
	SetupStraightTexProg();
	screenQuad.Draw();
                
    // Zamiana buforów
    glutSwapBuffers();
        
    // Jeszcze raz
    glutPostRedisplay();
}

int main(int argc, char* argv[])
{
    screenWidth = 614;
    screenHeight = 655; 
    mapTexProg = 0;
    varExposureProg = 0;
    adaptiveProg = 0;
    curHDRTex = 0;
	fboName = 0;
    curProg = 0;
    exposure = 0.1f;

	gltSetWorkingDirectory(argv[0]);
		
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(screenWidth,screenHeight);
  
    glutCreateWindow("HDR Imaging");
 
    glutReshapeFunc(ChangeSize);
    glutDisplayFunc(RenderScene);
    glutSpecialFunc(SpecialKeys);
    glutKeyboardFunc(ProcessKeys);

    SetupRC();
    glutMainLoop();    
    ShutdownRC();
    return 0;
}