#include <stdio.h>
#include <iostream>

#include <GLTools.h>
#include <GLShaderManager.h>
#include <GLFrustum.h>
#include <GLBatch.h>
#include <GLMatrixStack.h>
#include <GLGeometryTransform.h>
#include <StopWatch.h>
#include "sbm.h"

#ifdef __APPLE__
#include <glut/glut.h>
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>
#endif

static GLfloat vGreen[] = { 0.0f, 1.0f, 0.0f, 1.0f };
static GLfloat vWhite[] = { 1.0f, 1.0f, 1.0f, 1.0f };
static GLfloat vLightPos[] = { 0.0f, 3.0f, 0.0f, 1.0f };
static const GLenum windowBuff[] = { GL_BACK_LEFT };
static const GLenum fboBuffs[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };

GLsizei	 screenWidth;			// ��dana szeroko�� okna lub pulpitu
GLsizei  screenHeight;			// ��dana wysoko�� okna lub pulpitu

GLboolean bFullScreen;			// ��danie dzia�ania w trybie pe�noekranowym
GLboolean bAnimated;			// ��danie ci�g�ych aktualizacji


GLShaderManager		shaderManager;			// Manager wierzcho�k�w
GLMatrixStack		modelViewMatrix;		// Macierz model-widok
GLMatrixStack		projectionMatrix;		// Macierz rzutowania
GLFrustum			viewFrustum;			// Frusta widoku
GLGeometryTransform	transformPipeline;		// Potok przetwarzania geometrii
GLFrame				cameraFrame;			// Uk�ad odniesienia kamery

GLTriangleBatch		torusBatch;
GLTriangleBatch		sphereBatch;
GLBatch				floorBatch;
GLBatch             screenQuad;

GLuint				textures[3];
GLuint				processProg;
GLuint				texBO[3];
GLuint				texBOTexture;
bool                bUseFBO;
GLuint              fboName;
GLuint              depthBufferName; 
GLuint				renderBufferNames[3];

SBObject            ninja;
GLuint              ninjaTex[1];

void MoveCamera(void);
void DrawWorld(GLfloat yRot);
bool LoadBMPTexture(const char *szFileName, GLenum minFilter, GLenum magFilter, GLenum wrapMode);

static float* LoadFloatData(const char *szFile, int *count)
{
	GLint lineCount = 0;
	FILE *fp;
	float* data = 0;
	
    // Otwarcie pliku shadera
    fp = fopen(szFile, "r");
    if(fp != NULL)
	{
		char szFloat[1024];
        while ( fgets ( szFloat, sizeof szFloat, fp ) != NULL )
            lineCount++;
		
        // Powr�t na pocz�tek pliku
        rewind(fp);
		
        // Przydzia� pami�ci dla wszystkich danych
		data = (float*)malloc((lineCount)*sizeof(float));
		if (data != NULL)
		{	
			int index = 0;
			while ( fgets ( szFloat, sizeof szFloat, fp ) != NULL )
			{
				data[index] = (float)atof(szFloat);
				index++;
			}
			count[0] = index;
		}
        fclose(fp);
	}
    else
        return 0;    
    
    return data;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////
// Wczytanie pliku BMP jako tekstury
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
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, iWidth, iHeight, 0, GL_BGR, GL_UNSIGNED_BYTE, pBits);

    // Czy trzeba generowa� mipmapy?
	if(minFilter == GL_LINEAR_MIPMAP_LINEAR || minFilter == GL_LINEAR_MIPMAP_NEAREST || minFilter == GL_NEAREST_MIPMAP_LINEAR || minFilter == GL_NEAREST_MIPMAP_NEAREST)
		glGenerateMipmap(GL_TEXTURE_2D);    

	return true;
}


///////////////////////////////////////////////////////////////////////////////
// Tu mo�na bezpiecznie umie�ci� kod pocz�tkowy OpenGL. Wczytanie tekstur itp.
void SetupRC()
{
    GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		/* Problem: wywo�anie funkcji glewInit nie powiod�o si�. */
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
	}

	// Inicjalizacja managera shader�w
	shaderManager.InitializeStockShaders();

	glEnable(GL_DEPTH_TEST);

	// Czarny
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

#ifdef __APPLE__
    ninja.LoadFromSBM("ninja.sbm",
                      GLT_ATTRIBUTE_VERTEX,
                      GLT_ATTRIBUTE_NORMAL,
                      GLT_ATTRIBUTE_TEXTURE0);    
#else
    ninja.LoadFromSBM("../../../Src/Models/Ninja/ninja.sbm",
        GLT_ATTRIBUTE_VERTEX,
        GLT_ATTRIBUTE_NORMAL,
        GLT_ATTRIBUTE_TEXTURE0);
#endif
	gltMakeTorus(torusBatch, 0.4f, 0.15f, 35, 35);
	gltMakeSphere(sphereBatch, 0.1f, 26, 13);

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

    glGenTextures(1, ninjaTex);
	glBindTexture(GL_TEXTURE_2D, ninjaTex[0]);
#ifdef __APPLE__
	LoadBMPTexture("NinjaComp.bmp", GL_LINEAR, GL_LINEAR, GL_CLAMP);
#else
	LoadBMPTexture("../../../Src/Models/Ninja/NinjaComp.bmp", GL_LINEAR, GL_LINEAR, GL_CLAMP);
#endif

	glGenFramebuffers(1,&fboName);

	// Utworzenie bufora renderowania g��bi
	glGenRenderbuffers(1, &depthBufferName);
	glBindRenderbuffer(GL_RENDERBUFFER, depthBufferName);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, screenWidth, screenHeight);
	
	// Utworzenie trzech bufor�w renderowania kolor�w
	glGenRenderbuffers(3, renderBufferNames);
	glBindRenderbuffer(GL_RENDERBUFFER, renderBufferNames[0]);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, screenWidth, screenHeight);
	glBindRenderbuffer(GL_RENDERBUFFER, renderBufferNames[1]);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, screenWidth, screenHeight);
	glBindRenderbuffer(GL_RENDERBUFFER, renderBufferNames[2]);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, screenWidth, screenHeight);

	// Powi�zanie wszystkich czterech bufor�w renderowania z FBO
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboName);
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBufferName);
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, renderBufferNames[0]);
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_RENDERBUFFER, renderBufferNames[1]);
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_RENDERBUFFER, renderBufferNames[2]);

	processProg =  gltLoadShaderPairWithAttributes("multibuffer.vs", "multibuffer_frag_location.fs", 3,
								GLT_ATTRIBUTE_VERTEX, "vVertex", 
								GLT_ATTRIBUTE_NORMAL, "vNormal", 
								GLT_ATTRIBUTE_TEXTURE0, "texCoord0");
	glBindFragDataLocation(processProg, 0, "oStraightColor");
	glBindFragDataLocation(processProg, 1, "oGreyscale");
	glBindFragDataLocation(processProg, 2, "oLumAdjColor"); 
	glLinkProgram(processProg);


	// Utworzenie trzech nowych obiekt�w buforowych
	glGenBuffers(3,texBO);
	glGenTextures(1, &texBOTexture);
	
	int count = 0;
	float* fileData = 0;

	// Wczytanie pierwszego bufora TBO z krzyw� podobn� do tangensoidy � 1024 warto�ci
	fileData = LoadFloatData("LumTan.data", &count);
	if (count > 0)
	{
		glBindBuffer(GL_TEXTURE_BUFFER_ARB, texBO[0]);
		glBufferData(GL_TEXTURE_BUFFER_ARB, sizeof(float)*count, fileData, GL_STATIC_DRAW);
		delete fileData;
	}

	// Wczytanie drugiego bufora TBO z krzyw� podobn� do sinusoidy � 1024 warto�ci
	fileData = LoadFloatData("LumSin.data", &count);
	if (count > 0)
	{
		glBindBuffer(GL_TEXTURE_BUFFER_ARB, texBO[1]);
		glBufferData(GL_TEXTURE_BUFFER_ARB, sizeof(float)*count, fileData, GL_STATIC_DRAW);
		delete fileData;
	}

	// Wczytanie trzeciego bufora TBO z lini� prost� � 1024 warto�ci
	fileData = LoadFloatData("LumLinear.data", &count);
	if (count > 0)
	{
		glBindBuffer(GL_TEXTURE_BUFFER_ARB, texBO[2]);
		glBufferData(GL_TEXTURE_BUFFER_ARB, sizeof(float)*count, fileData, GL_STATIC_DRAW);
		delete fileData;
	}

	// Najpierw wczytujemy tangensoid� 
	glBindBuffer(GL_TEXTURE_BUFFER_ARB, 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_BUFFER_ARB, texBOTexture);
	glTexBufferARB(GL_TEXTURE_BUFFER_ARB, GL_R32F, texBO[0]); 
	glActiveTexture(GL_TEXTURE0);

	// Reset wi�za� bufora obrazu
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	// Upewnienie si�, czy wszystko posz�o dobrze
	gltCheckErrors();
}

///////////////////////////////////////////////////////////////////////////////
// Aktualizacja kamery na podstawie danych wprowadzanych przez u�ytkownika, prze��czanie tryb�w wy�wietlania
// 
void SpecialKeys(int key, int x, int y)
{
	static CStopWatch cameraTimer;
	float fTime = cameraTimer.GetElapsedSeconds();
	cameraTimer.Reset(); 

	float linear = fTime * 3.0f;
	float angular = fTime * float(m3dDegToRad(60.0f));

	if(key == GLUT_KEY_UP)
		cameraFrame.MoveForward(linear);

	if(key == GLUT_KEY_DOWN)
		cameraFrame.MoveForward(-linear);

	if(key == GLUT_KEY_LEFT)
		cameraFrame.RotateWorld(angular, 0.0f, 1.0f, 0.0f);

	if(key == GLUT_KEY_RIGHT)
		cameraFrame.RotateWorld(-angular, 0.0f, 1.0f, 0.0f);

	static bool bF2IsDown = false;
	if(key == GLUT_KEY_F2)
	{
		if(bF2IsDown == false)
		{
			bF2IsDown = true;
			bUseFBO = !bUseFBO;
		}
	}
	else
	{
		bF2IsDown = false; 
	}

	if(key == GLUT_KEY_F3)
	{
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_BUFFER_ARB, texBOTexture);
		glTexBufferARB(GL_TEXTURE_BUFFER_ARB, GL_R32F, texBO[0]); // USTAW TO W GLEE
		glActiveTexture(GL_TEXTURE0);
	}
	else if(key == GLUT_KEY_F4)
	{
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_BUFFER_ARB, texBOTexture);
		glTexBufferARB(GL_TEXTURE_BUFFER_ARB, GL_R32F, texBO[1]); // USTAW TO W GLEE
		glActiveTexture(GL_TEXTURE0);
	}
	else if(key == GLUT_KEY_F5)
	{
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_BUFFER_ARB, texBOTexture);
		glTexBufferARB(GL_TEXTURE_BUFFER_ARB, GL_R32F, texBO[2]); // USTAW TO W GLEE
		glActiveTexture(GL_TEXTURE0);
	}
                        
	 // Od�wie�enie okna
	 glutPostRedisplay();
}

///////////////////////////////////////////////////////////////////////////////
// Procedury porz�dkowe. Zwalnianie tekstur, obiekt�w bufor�w itp.
void ShutdownRC(void)
{
	// Upewnienie si�, �e zwi�zany jest domy�lny FBO
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

	// Porz�dkowanie tekstur
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_BUFFER_ARB, 0);
	glActiveTexture(GL_TEXTURE0);
	
	glDeleteTextures(1, &texBOTexture);
	glDeleteTextures(1, textures);
    glDeleteTextures(1, ninjaTex);

	// Czyszczenie RBO
	glDeleteRenderbuffers(3, renderBufferNames);
	glDeleteRenderbuffers(1, &depthBufferName);

	// Czyszczenie FBO
	glDeleteFramebuffers(1, &fboName);

	// Czyszczenie obiekt�w bufora
	glDeleteBuffers(3, texBO);

	// Czyszczenie program�w
	glUseProgram(0);
	glDeleteProgram(processProg);

    ninja.Free();
}


///////////////////////////////////////////////////////////////////////////////
// Ta funkcja jest wywo�ywana przynajmniej raz i przed ka�d� operacj� renderowania. Je�li ekran
// jest mog�cym zmienia� rozmiar oknem, to funkcja ta zostanie wywo�ana dla ka�dej zmiany
// rozmiaru okna.
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

	glBindRenderbuffer(GL_RENDERBUFFER, depthBufferName);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, screenWidth, screenHeight);
	glBindRenderbuffer(GL_RENDERBUFFER, renderBufferNames[0]);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, screenWidth, screenHeight);
	glBindRenderbuffer(GL_RENDERBUFFER, renderBufferNames[1]);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, screenWidth, screenHeight);
	glBindRenderbuffer(GL_RENDERBUFFER, renderBufferNames[2]);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, screenWidth, screenHeight);
}



///////////////////////////////////////////////////////////////////////////////
// W��czenie i konfiguracja programu GLSL s�u��cego do 
// opr�niania bufor�w itp.
void UseProcessProgram(M3DVector4f vLightPos, M3DVector4f vColor, int textureUnit)
{
	glUseProgram(processProg);

	// Macierze dla shadera wierzcho�k�w
	glUniformMatrix4fv(glGetUniformLocation(processProg, "mvMatrix"), 
				1, GL_FALSE, transformPipeline.GetModelViewMatrix());
	glUniformMatrix4fv(glGetUniformLocation(processProg, "pMatrix"), 
				1, GL_FALSE, transformPipeline.GetProjectionMatrix());

	// Ustawienie po�o�enia o�wietlenia
	glUniform3fv(glGetUniformLocation(processProg, "vLightPos"), 1, vLightPos);

	// Ustawienie koloru wierzcho�k�w dla renderowanych pikseli
	glUniform4fv(glGetUniformLocation(processProg, "vColor"), 1, vColor);

	// Jednostka teksturuj�ca do pobrania TBO
	glUniform1i(glGetUniformLocation(processProg, "lumCurveSampler"), 1);

	// Ustawienie jednostki teksturuj�cej, je�li geometria jest oteksturowana
	if(textureUnit != -1)
	{
		glUniform1i(glGetUniformLocation(processProg, "bUseTexture"), 1);
		glUniform1i(glGetUniformLocation(processProg, "textureUnit0"), textureUnit);
	}
	else
	{
		glUniform1i(glGetUniformLocation(processProg, "bUseTexture"), 0);
	}

	gltCheckErrors(processProg);
}

///////////////////////////////////////////////////////////////////////////////
// Rysowanie sceny
// 
void DrawWorld(GLfloat yRot)
{
	M3DMatrix44f mCamera;
	modelViewMatrix.GetMatrix(mCamera);
	
	// Potrzebne jest po�o�enie �r�d�a �wiat�a wzgl�dem kamery
	M3DVector4f vLightTransformed;
	m3dTransformVector4(vLightTransformed, vLightPos, mCamera);

	// Rysowanie �r�d�a �wiat�a jako ma�ej bia�ej niecieniowanej kuli
	modelViewMatrix.PushMatrix();
		modelViewMatrix.Translatev(vLightPos);

		if(bUseFBO)
			UseProcessProgram(vLightPos, vWhite, -1);
		else
			shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vWhite);

		sphereBatch.Draw();
	modelViewMatrix.PopMatrix();

	// Rysowanie obiekt�w wzgl�dem kamery
	modelViewMatrix.PushMatrix();
		modelViewMatrix.Translate(0.0f, 0.2f, -2.5f);

		modelViewMatrix.PushMatrix();
			modelViewMatrix.Rotate(yRot, 0.0f, 1.0f, 0.0f);
            modelViewMatrix.Translate(0.0, (GLfloat)-0.60, 0.0);
            modelViewMatrix.Scale((GLfloat)0.02, (GLfloat)0.006, (GLfloat)0.02);
            
            glBindTexture(GL_TEXTURE_2D, ninjaTex[0]);

			if(bUseFBO)
			{
				UseProcessProgram(vLightTransformed, vWhite, 0);
			}
			else
			{
                shaderManager.UseStockShader(GLT_SHADER_TEXTURE_REPLACE, transformPipeline.GetModelViewProjectionMatrix(), 0);
			}
            ninja.Render(0,0);
        modelViewMatrix.PopMatrix();

	modelViewMatrix.PopMatrix();
}


///////////////////////////////////////////////////////////////////////////////
// Rednerowanie klatki. Szkielet odpowiada za zamiany bufor�w,
// opr�niania bufor�w itp.
void RenderScene(void)
{
	static CStopWatch animationTimer;
	float yRot = animationTimer.GetElapsedSeconds() * 60.0f;
//	MoveCamera();

	modelViewMatrix.PushMatrix();	
		M3DMatrix44f mCamera;
		cameraFrame.GetCameraMatrix(mCamera);
		modelViewMatrix.MultMatrix(mCamera);
		
		GLfloat vFloorColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

		if(bUseFBO)
		{
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboName);
			glDrawBuffers(3, fboBuffs);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// Potrzebne jest po�o�enie �r�d�a �wiat�a wzgl�dem kamery
			M3DVector4f vLightTransformed;
			m3dTransformVector4(vLightTransformed, vLightPos, mCamera);
			UseProcessProgram(vLightTransformed, vFloorColor, 0);
		}
		else
		{
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
			glDrawBuffers(1, windowBuff);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			shaderManager.UseStockShader(GLT_SHADER_TEXTURE_MODULATE, transformPipeline.GetModelViewProjectionMatrix(), vFloorColor, 0);
		}

        glBindTexture(GL_TEXTURE_2D, textures[0]); // Marmur
		floorBatch.Draw();
		DrawWorld(yRot);

	modelViewMatrix.PopMatrix();

	if(bUseFBO)
	{
		// Rysowanie bezpo�rednio w oknie
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glDrawBuffers(1, windowBuff);
		glViewport(0, 0, screenWidth, screenHeight);

		// Bufor �r�d�owy odczytuje dane z obiektu bufora obrazu
		glBindFramebuffer(GL_READ_FRAMEBUFFER, fboName);

		// Kopiowanie skali szaro�ci do lewej po�owy ekranu
		glReadBuffer(GL_COLOR_ATTACHMENT1);
		glBlitFramebuffer(0, 0, screenWidth/2, screenHeight,
						  0, 0, screenWidth/2, screenHeight,
						  GL_COLOR_BUFFER_BIT, GL_NEAREST );
	
		// Kopiowanie koloru z ustawion� luminancj� do prawej po�owy ekranu
		glReadBuffer(GL_COLOR_ATTACHMENT2);	
		glBlitFramebuffer(screenWidth/2, 0, screenWidth, screenHeight,
						  screenWidth/2, 0, screenWidth, screenHeight,
						  GL_COLOR_BUFFER_BIT, GL_NEAREST );

		// Skalowanie niezmienionego obrazu do g�rnej prawej cz�ci ekranu
		glReadBuffer(GL_COLOR_ATTACHMENT0);
		glBlitFramebuffer(0, 0, screenWidth, screenHeight,
						  (int)(screenWidth *(0.8)), (int)(screenHeight*(0.8)), 
						  screenWidth, screenHeight,
						  GL_COLOR_BUFFER_BIT, GL_LINEAR );

		glBindTexture(GL_TEXTURE_2D, 0);
	} 

    // Zamiana bufor�w
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
    bUseFBO = true;
    fboName = 0;
    depthBufferName = 0;

	gltSetWorkingDirectory(argv[0]);
		
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(screenWidth,screenHeight);
  
    glutCreateWindow("FBO Drawbuffers");
 
    glutReshapeFunc(ChangeSize);
    glutDisplayFunc(RenderScene);
    glutSpecialFunc(SpecialKeys);

    SetupRC();
    glutMainLoop();    
    ShutdownRC();
    return 0;
}
