#include <stdio.h>
#include <iostream>

#include <GLTools.h>
#include <GLShaderManager.h>
#include <GLFrustum.h>
#include <GLBatch.h>
#include <GLMatrixStack.h>
#include <GLGeometryTransform.h>
#include <StopWatch.h>

#include <GL/glu.h>

#ifdef __APPLE__
#include <glut/glut.h>
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>
#endif

static GLfloat vGreen[] = { 0.0f, 1.0f, 0.0f, 1.0f };
static GLfloat vWhite[] = { 1.0f, 1.0f, 1.0f, 1.0f };
static GLfloat vLightPos[] = { 0.0f, 3.0f, 0.0f, 1.0f };

GLsizei	 screenWidth;			// Planowana szeroko�� okna lub pulpitu
GLsizei  screenHeight;			// Planowana wysoko�� okna lub pulpitu

GLboolean bFullScreen;			// ��danie trybu pe�noekranowego
GLboolean bAnimated;			// ��danie ci�g�ych aktualizacji


GLShaderManager		shaderManager;			// Manager shader�w
GLMatrixStack		modelViewMatrix;		// Macierz model-widok
GLMatrixStack		projectionMatrix;		// Macierz rzutowania
M3DMatrix44f        orthoMatrix;     
GLFrustum			viewFrustum;			// Obszar widoku
GLGeometryTransform	transformPipeline;		// Potok przekszta�cania geometrii
GLFrame				cameraFrame;			// Uk�ad odniesienia kamery

GLTriangleBatch		torusBatch;
GLBatch				floorBatch;
GLBatch             screenQuad;

GLuint				textures[1];
GLuint				blurTextures[6];
GLuint				pixBuffObjs[1];
GLuint				curBlurTarget;
bool				bUsePBOPath;
GLfloat				speedFactor;
GLuint				blurProg;
void				*pixelData;
GLuint				pixelDataSize;

void MoveCamera(void);
void DrawWorld(GLfloat yRot, GLfloat xPos);
bool LoadBMPTexture(const char *szFileName, GLenum minFilter, GLenum magFilter, GLenum wrapMode);

void SetupBlurProg(void);

// Zwraca 1 - 6 dla tekstur efektu zamazania
// // curPixBuf ma zawsze warto�� z przedzia�u 1-5
void AdvanceBlurTaget() { curBlurTarget = ((curBlurTarget+ 1) %6); }
GLuint GetBlurTarget0(){ return (1 + ((curBlurTarget + 5) %6)); }
GLuint GetBlurTarget1(){ return (1 + ((curBlurTarget + 4) %6)); }
GLuint GetBlurTarget2(){ return (1 + ((curBlurTarget + 3) %6)); }
GLuint GetBlurTarget3(){ return (1 + ((curBlurTarget + 2) %6)); }
GLuint GetBlurTarget4(){ return (1 + ((curBlurTarget + 1) %6)); }
GLuint GetBlurTarget5(){ return (1 + ((curBlurTarget) %6)); }

void UpdateFrameCount()
{
	static int iFrames = 0;           // Licznik klatek
	static CStopWatch frameTimer;     // Czas renderowania
 
    // Zresetowanie sekundnika przy pierwszej warto�ci czasowej
    if(iFrames == 0)
    {
        frameTimer.Reset();
        iFrames++;
    }
    // Zwi�kszenie warto�ci licznika klatek
    iFrames++;

    // Okresowe obliczenia szybko�ci klatek
    if (iFrames == 101)
    {
        float fps;

        fps = 100.0f / frameTimer.GetElapsedSeconds();
		if (bUsePBOPath)
			printf("Pix_buffs - u�ycie bufor�w pikseli  %.1f fps\n", fps);
		else
			printf("Pix_buffs - zastosowanie kopiowania do pami�ci klienckiej %.1f fps\n", fps);

        frameTimer.Reset();
        iFrames = 1;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
// Za�adowanie pliku BMP jako tekstury. To umo�liwia okre�lenie filtr�w i trybu zawijania
bool LoadBMPTexture(const char *szFileName, GLenum minFilter, GLenum magFilter, GLenum wrapMode)	
{
	GLbyte *pBits;
	GLint iWidth, iHeight;

	pBits = gltReadBMPBits(szFileName, &iWidth, &iHeight);
	if(pBits == NULL)
		return false;

	// Ustawienie tryb�w zawijania
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);

	// Czy trzeba generowa� mipmapy?
	if(minFilter == GL_LINEAR_MIPMAP_LINEAR || minFilter == GL_LINEAR_MIPMAP_NEAREST || minFilter == GL_NEAREST_MIPMAP_LINEAR || minFilter == GL_NEAREST_MIPMAP_NEAREST)
		glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, iWidth, iHeight, 0, GL_BGR, GL_UNSIGNED_BYTE, pBits);
	return true;
}


///////////////////////////////////////////////////////////////////////////////
// Tu mo�na bezpiecznie umie�ci� kod rozruchowy OpenGL. �adowanie tekstur itd.
void SetupRC(void)
{
    GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		/* Problem: wykonanie funkcji glewInit nie powiod�o si�. Co� jest nie w porz�dku. */
		fprintf(stderr, "B��d: %s\n", glewGetErrorString(err));
	}

	// Inicjalizacja managera shader�w
	shaderManager.InitializeStockShaders();
	glEnable(GL_DEPTH_TEST);

	// Czarny
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	gltMakeTorus(torusBatch, 0.4f, 0.15f, 35, 35);

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
	
	glGenTextures(1, textures);
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	LoadBMPTexture("marble.bmp", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_REPEAT);

	// Tworzenie programu rozmywaj�cego
	blurProg =  gltLoadShaderPairWithAttributes("blur.vs", "blur.fs", 2,
												GLT_ATTRIBUTE_VERTEX, "vVertex", GLT_ATTRIBUTE_TEXTURE0, "texCoord0");

	// Tworzenie tekstur rozmycia
	glGenTextures(6, blurTextures);

    // Nie wydaje mi si� to konieczne. Dane tekstury trzeba ustawi� na NULL
	// Alokowanie bufora pikseli w celu zainicjalizowania tekstur i PBO
	pixelDataSize = screenWidth*screenHeight*3*sizeof(unsigned int); // To powinien by� bajt bez znaku
	void* data = (void*)malloc(pixelDataSize);
	memset(data, 0x00, pixelDataSize);

	// Sze�� jednostek teksturuj�cych do efektu rozmycia
	// Inicjalizacja danych tekstur
	for (int i=0; i<6;i++)
	{
		glActiveTexture(GL_TEXTURE1+i);
		glBindTexture(GL_TEXTURE_2D, blurTextures[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenWidth, screenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	}

	// Alokowanie pami�ci do kopiowania pikseli, aby nie musie� wywo�ywa� funkcji malloc przy ka�dym rysowaniu
	glGenBuffers(1, pixBuffObjs);
	glBindBuffer(GL_PIXEL_PACK_BUFFER, pixBuffObjs[0]);
	glBufferData(GL_PIXEL_PACK_BUFFER, pixelDataSize, pixelData, GL_DYNAMIC_COPY);
	glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

	// Utworzenie geometrii i macierzy do rysowania na r�wni z ekranem
	gltGenerateOrtho2DMat(screenWidth, screenHeight, orthoMatrix, screenQuad);

	// Sprawdzenie czy wszystko posz�o zgodnie z planem
	gltCheckErrors();
}


///////////////////////////////////////////////////////////////////////////////
// Procedury czyszcz�ce. Zwolnienie tekstur, obiekt�w bufor�w itd.
void ShutdownRC(void)
{
	// Zwi�zanie domy�lnego FBO
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

	// Wyczyszczenie tekstur
	for (int i=0; i<7;i++)
	{
		glActiveTexture(GL_TEXTURE0+i);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	
	// Usuni�cie odwi�zanych tekstur
	glDeleteTextures(1, textures);
	glDeleteTextures(6, blurTextures);

	// Usuni�cie PBO
	glDeleteBuffers(1, pixBuffObjs);
}


///////////////////////////////////////////////////////////////////////////////
// Ta funkcja jest wywo�ywana przynajmniej raz, przed renderowaniem. Je�li ekran
// jest oknem o zmiennym rozmiarze, funkcja ta zostanie wywo�ana przy ka�dej
// zmianie rozmiaru tego okna.
void ChangeSize(int nWidth, int nHeight)
{
	glViewport(0, 0, nWidth, nHeight);
	transformPipeline.SetMatrixStacks(modelViewMatrix, projectionMatrix);
 	viewFrustum.SetPerspective(35.0f, float(nWidth)/float(nHeight), 1.0f, 100.0f);
	projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
	modelViewMatrix.LoadIdentity();

	// Aktualizacja wymiar�w ekranu
	screenWidth = nWidth;
	screenHeight = nHeight;

	
	gltGenerateOrtho2DMat(screenWidth, screenHeight, orthoMatrix, screenQuad);

	free(pixelData);
	pixelDataSize = screenWidth*screenHeight*3*sizeof(unsigned int);
	pixelData = (void*)malloc(pixelDataSize);

	// Zmiana rozmiaru bufor�w PBO
	glBindBuffer(GL_PIXEL_PACK_BUFFER, pixBuffObjs[0]);
	glBufferData(GL_PIXEL_PACK_BUFFER, pixelDataSize, pixelData, GL_DYNAMIC_COPY);
	glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

	gltCheckErrors();
}


///////////////////////////////////////////////////////////////////////////////
// Zmienianie kamery w odpowiedzi na dane od u�ytkownika, prze��czanie tryb�w wy�wietlania
// 
void ProccessKeys(unsigned char key, int x, int y)
{ 
	static CStopWatch cameraTimer;
	float fTime = cameraTimer.GetElapsedSeconds();
	float linear = fTime * 12.0f;
	cameraTimer.Reset(); 

	// Zmiana mi�dzy PBO i pami�ci� lokaln� w odpowiedzi na naciskanie klawisza P
	if(key == 'P' || key == 'p') 
		bUsePBOPath = (bUsePBOPath)? GL_FALSE : GL_TRUE;

	// Przyspieszenie ruchu
	if(key == '+')
	{
		speedFactor += linear/2;
		if(speedFactor > 6)
			speedFactor = 6;
	}

	// Spowolnienie ruchu
	if(key == '-')
	{
		speedFactor -= linear/2;
		if(speedFactor < 0.5)
			speedFactor = 0.5;
	}
}


///////////////////////////////////////////////////////////////////////////////
// Za�adowanie i skonfigurowanie programu efektu rozmycia
// 

void SetupBlurProg(void)
{
	// Ustawienie programu efektu rozmycia jako bie��cego
	glUseProgram(blurProg);

	// Macierz rzutowania model-widok
	glUniformMatrix4fv(glGetUniformLocation(blurProg, "mvpMatrix"), 1, GL_FALSE, transformPipeline.GetModelViewProjectionMatrix());

	// Jednostki teksturuj�ce do efektu rozmycia, zmieniaj� si� w ka�dej klatce
	glUniform1i(glGetUniformLocation(blurProg, "textureUnit0"), GetBlurTarget0());
	glUniform1i(glGetUniformLocation(blurProg, "textureUnit1"), GetBlurTarget1());
	glUniform1i(glGetUniformLocation(blurProg, "textureUnit2"), GetBlurTarget2());
	glUniform1i(glGetUniformLocation(blurProg, "textureUnit3"), GetBlurTarget3());
	glUniform1i(glGetUniformLocation(blurProg, "textureUnit4"), GetBlurTarget4());
	glUniform1i(glGetUniformLocation(blurProg, "textureUnit5"), GetBlurTarget5());
}


///////////////////////////////////////////////////////////////////////////////
// Rysowanie sceny
// 
void DrawWorld(GLfloat yRot, GLfloat xPos)
{
	M3DMatrix44f mCamera;
	modelViewMatrix.GetMatrix(mCamera);
	
	// Potrzebne po�o�enie �wiat�a wzgl�dem kamery
	M3DVector4f vLightTransformed;
	m3dTransformVector4(vLightTransformed, vLightPos, mCamera);

	// Rysowanie obiekt�w wzgl�dem kamery
	modelViewMatrix.PushMatrix();
		modelViewMatrix.Translate(0.0f, 0.2f, -2.5f);
		modelViewMatrix.Translate(xPos, 0.0f, 0.0f);
		modelViewMatrix.Rotate(yRot, 0.0f, 1.0f, 0.0f);
	
		shaderManager.UseStockShader(GLT_SHADER_POINT_LIGHT_DIFF, 
				modelViewMatrix.GetMatrix(), 
				transformPipeline.GetProjectionMatrix(), 
				vLightTransformed, vGreen, 0);
		torusBatch.Draw();
	modelViewMatrix.PopMatrix();
}


///////////////////////////////////////////////////////////////////////////////
// Renderowanie klatki. System nadrz�dny odpowiada za wymian� bufor�w,
// opr�nienia bufor�w itp.
void RenderScene(void)
{
	static CStopWatch animationTimer;
	static float totalTime = 6; 
	static float halfTotalTime = totalTime/2;
	float seconds = animationTimer.GetElapsedSeconds() * speedFactor;
	float xPos = 0;

	// Obliczenie nast�pnego po�o�enia poruszaj�cego si� obiektu
	// Najpierw wykonujemy operacj� typu dzielenie modulo na czasie jako warto�ci zmiennoprzecinkowej
	while(seconds > totalTime)
		seconds -= totalTime;

	// Przesuni�cie obiektu. Gdy b�dzie w po�owie drogi,
	// zaczynamy go cofa�
	if(seconds < halfTotalTime)
		xPos = seconds -halfTotalTime*0.5f;
	else
		xPos = totalTime - seconds -halfTotalTime*0.5f;

	// Najpierw rysujemy �wiat na ekranie
	modelViewMatrix.PushMatrix();	
		M3DMatrix44f mCamera;
		cameraFrame.GetCameraMatrix(mCamera);
		modelViewMatrix.MultMatrix(mCamera);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textures[0]); // Marmur
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shaderManager.UseStockShader(GLT_SHADER_TEXTURE_MODULATE, transformPipeline.GetModelViewProjectionMatrix(), vWhite, 0);

		floorBatch.Draw();
		DrawWorld(0.0f, xPos);
	modelViewMatrix.PopMatrix();
	
	if(bUsePBOPath)
	{
		// Najpierw wi��emy PBO jako bufor pikseli spakowanych, a nast�pnie wczytujemy te piksele bezpo�rednio do PBO
		glBindBuffer(GL_PIXEL_PACK_BUFFER, pixBuffObjs[0]);
		glReadPixels(0, 0, screenWidth, screenHeight, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

		// P�niej wi��emy PBO jako bufor pikseli odpakowanych i przesy�amy je bezpo�rednio do tekstury
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pixBuffObjs[0]);
        
        // Jednostka teksturuj�ca dla nowego rozmycia, ta warto�� jest zwi�kszana dla ka�dej klatki
		glActiveTexture(GL_TEXTURE0+GetBlurTarget0() ); 
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, screenWidth, screenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	}
	else
	{
		// Pobranie pikseli z ekranu i skopiowanie ich do pami�ci lokalnej
		glReadPixels(0, 0, screenWidth, screenHeight, GL_RGB, GL_UNSIGNED_BYTE, pixelData);
		
		// Wys�anie pikseli z pami�ci klienckiej do tekstury
        // Nowa jednostka teksturuj�ca dla nowego rozmycia, warto�� ta jest inkrementowana dla ka�dej klatki
		glActiveTexture(GL_TEXTURE0+GetBlurTarget0() );
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, screenWidth, screenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, pixelData);
	}

	// Rysowanie pe�noekranowego czworok�ta przy u�yciu shadera rozmycia i wszystkich tekstur rozmycia
	projectionMatrix.PushMatrix(); 
		projectionMatrix.LoadIdentity();
		projectionMatrix.LoadMatrix(orthoMatrix);
		modelViewMatrix.PushMatrix();	
			modelViewMatrix.LoadIdentity();
			glDisable(GL_DEPTH_TEST); 
			SetupBlurProg();
			screenQuad.Draw();
			glEnable(GL_DEPTH_TEST); 
		modelViewMatrix.PopMatrix(); 
	projectionMatrix.PopMatrix();

	// Przej�cie do nast�pnej tekstury rozmycia dla nast�pnej klatki
	AdvanceBlurTaget();
    
    // Zamiana bufor�w
    glutSwapBuffers();
        
    // Jeszcze raz
    glutPostRedisplay();

    UpdateFrameCount();
}


int main(int argc, char* argv[])
{
    screenWidth  = 800;
    screenHeight = 600;
    bFullScreen = false; 
    bAnimated   = true;
    bUsePBOPath = false;
    blurProg    = 0;
    speedFactor = 1.0f;

	gltSetWorkingDirectory(argv[0]);
		
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(screenWidth,screenHeight);
  
    glutCreateWindow("pix_buffs");
 
    glutReshapeFunc(ChangeSize);
    glutDisplayFunc(RenderScene);
    glutKeyboardFunc(ProccessKeys);

    SetupRC();
    glutMainLoop();    
    ShutdownRC();
    return 0;
}
