#include <stdio.h>
#include <iostream>

#include <GLTools.h>
#include <GLShaderManager.h>
#include <GLFrustum.h>
#include <GLBatch.h>
#include <GLMatrixStack.h>
#include <GLGeometryTransform.h>
#include <StopWatch.h>

//#include <GL/glu.h>

#ifdef __APPLE__
#include <glut/glut.h>
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>
#endif

static GLfloat vGreen[] = { 0.0f, 1.0f, 0.0f, 1.0f };
static GLfloat vBlue[] = { 0.0f, 0.0f, 1.0f, 1.0f };
static GLfloat vWhite[] = { 1.0f, 1.0f, 1.0f, 1.0f };
static GLfloat vBlack[] = { 0.0f, 0.0f, 0.0f, 1.0f };
static GLfloat vGrey[] =  { 0.5f, 0.5f, 0.5f, 1.0f };
static GLfloat vLightPos[] = { -2.0f, 3.0f, -2.0f, 1.0f };
static const GLenum windowBuff[] = { GL_BACK_LEFT };
static const GLenum fboBuffs[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
static GLint mirrorTexWidth  = 800;
static GLint mirrorTexHeight = 800;

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
GLFrame				mirrorFrame;			// Uk�ad odniesienia lustra

GLTriangleBatch		torusBatch;
GLTriangleBatch		sphereBatch;
GLTriangleBatch		cylinderBatch;
GLBatch				floorBatch;
GLBatch				mirrorBatch;
GLBatch				mirrorBorderBatch;

GLuint              fboName;
GLuint				textures[1];
GLuint				mirrorTexture;
GLuint              depthBufferName; 

void DrawWorld(GLfloat yRot);
bool LoadBMPTexture(const char *szFileName, GLenum minFilter, GLenum magFilter, GLenum wrapMode);


///////////////////////////////////////////////////////////////////////////////////////////////////////
// Wczytanie pliku BMP jako tekstury. Umo�liwia okre�lanie filtr�w i trybu zawijania
bool LoadBMPTexture(const char *szFileName, GLenum minFilter, GLenum magFilter, GLenum wrapMode)	
{
	GLbyte *pBits;
	GLint iWidth, iHeight;

	pBits = gltReadBMPBits(szFileName, &iWidth, &iHeight);
	if(pBits == NULL)
		return false;

	// Ustawienia tryb�w zawijania
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


///////////////////////////////////////////////////////////////////////////////
// Tu mo�na bezpiecznie umie�ci� kod innicjuj�cy OpenGL, �adowanie tekstur itp.
void SetupRC()
{
    GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		/* Problem: wywo�anie funkcji glewInit nie powiod�o si� */
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
	}

	// Inicjalizacja managera shader�w
	shaderManager.InitializeStockShaders();
	glEnable(GL_DEPTH_TEST);

	// Czarny
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	gltMakeTorus(torusBatch, 0.4f, 0.15f, 35, 35);
	gltMakeSphere(sphereBatch, 0.1f, 26, 13);
	gltMakeCylinder(cylinderBatch,0.3f, 0.2f, 1.0, 10,10);

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

	mirrorBatch.Begin(GL_TRIANGLE_FAN, 4, 1);
		mirrorBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
		mirrorBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
		mirrorBatch.Normal3f( 0.0f, 1.0f, 0.0f);
		mirrorBatch.Vertex3f(-1.0f, 0.0f, 0.0f);

		mirrorBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
		mirrorBatch.MultiTexCoord2f(0, 1.0f, 0.0f);
		mirrorBatch.Normal3f(0.0f, 1.0f, 0.0f);
		mirrorBatch.Vertex3f(1.0f, 0.0f, 0.0f);

		mirrorBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
		mirrorBatch.MultiTexCoord2f(0, 1.0f, 1.0f);
		mirrorBatch.Normal3f(0.0f, 1.0f, 0.0f);
		mirrorBatch.Vertex3f(1.0f, 2.0f, 0.0f);

		mirrorBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
		mirrorBatch.MultiTexCoord2f(0, 0.0f, 1.0f);
		mirrorBatch.Normal3f( 0.0f, 1.0f, 0.0f);
		mirrorBatch.Vertex3f(-1.0f, 2.0f, 0.0f);
	mirrorBatch.End();

	mirrorBorderBatch.Begin(GL_TRIANGLE_STRIP, 13);
		mirrorBorderBatch.Normal3f( 0.0f, 0.0f, 1.0f);
		mirrorBorderBatch.Vertex3f(-1.0f, 0.1f, 0.01f);

		mirrorBorderBatch.Normal3f(0.0f, 0.0f, 1.0f);
		mirrorBorderBatch.Vertex3f(-1.0f, 0.0f, 0.01f);

		mirrorBorderBatch.Normal3f(0.0f, 0.0f, 1.0f);
		mirrorBorderBatch.Vertex3f(1.0f, 0.1f, 0.01f);

		mirrorBorderBatch.Normal3f( 0.0f, 0.0f, 1.0f);
		mirrorBorderBatch.Vertex3f(1.0f, 0.0f, 0.01f);

		mirrorBorderBatch.Normal3f( 0.0f, 0.0f, 1.0f);
		mirrorBorderBatch.Vertex3f(0.9f, 0.0f, 0.01f);

		mirrorBorderBatch.Normal3f( 0.0f, 0.0f, 1.0f);
		mirrorBorderBatch.Vertex3f(1.0f, 2.0f, 0.01f);
			
		mirrorBorderBatch.Normal3f( 0.0f, 0.0f, 1.0f);
		mirrorBorderBatch.Vertex3f(0.9f, 2.0f, 0.01f);
			
		mirrorBorderBatch.Normal3f( 0.0f, 0.0f, 1.0f);
		mirrorBorderBatch.Vertex3f(1.0f, 1.9f, 0.01f);

		mirrorBorderBatch.Normal3f( 0.0f, 0.0f, 1.0f);
		mirrorBorderBatch.Vertex3f(-1.0f, 2.f, 0.01f);

		mirrorBorderBatch.Normal3f( 0.0f, 0.0f, 1.0f);
		mirrorBorderBatch.Vertex3f(-1.0f, 1.9f, 0.01f);

		mirrorBorderBatch.Normal3f( 0.0f, 0.0f, 1.0f);
		mirrorBorderBatch.Vertex3f(-0.9f, 2.f, 0.01f);

		mirrorBorderBatch.Normal3f( 0.0f, 0.0f, 1.0f);
		mirrorBorderBatch.Vertex3f(-1.0f, 0.0f, 0.01f);

		mirrorBorderBatch.Normal3f( 0.0f, 0.0f, 1.0f);
		mirrorBorderBatch.Vertex3f(-0.9f, 0.0f, 0.01f);
	mirrorBorderBatch.End();

	glGenTextures(1, textures);
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	LoadBMPTexture("marble.bmp", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_REPEAT);

	// Utworzenie i zwi�zanie FBO
	glGenFramebuffers(1,&fboName);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboName);

	// Utworzenie bufora renderowania g��bi
	glGenRenderbuffers(1, &depthBufferName);
	glBindRenderbuffer(GL_RENDERBUFFER, depthBufferName);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, mirrorTexWidth, mirrorTexHeight);
	 
	// Utworzenie tekstury odbicia
	glGenTextures(1, &mirrorTexture);
	glBindTexture(GL_TEXTURE_2D, mirrorTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, mirrorTexWidth, mirrorTexHeight, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Zwi�zanie tekstury z pierwszym punktem wi�zania kolor�w i RBO g��bi
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mirrorTexture, 0);
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBufferName);

	// Upewnienie si�, czy wszystko posz�o dobrze
	gltCheckErrors();
	
	// Reset wi�za� bufora obrazu
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}


///////////////////////////////////////////////////////////////////////////////
// Procedury porz�dkowe, zwalnianie tekstur, obiekt�w bufora itp.
void ShutdownRC(void)
{
	// Upewnienie si�, �e domy�lny FBO jest zwi�zany
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

	// Czyszczenie tekstur
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	glDeleteTextures(1, &mirrorTexture);
	glDeleteTextures(1, textures);

	// Czyszczenie RBO
	glDeleteRenderbuffers(1, &depthBufferName);

	// Czyszczenie FBO
	glDeleteFramebuffers(1, &fboName);

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
}


///////////////////////////////////////////////////////////////////////////////
// Aktualizacja kamery na podstawie danych od u�ytkownika, prze��czanie tryb�w wy�wietlania
// 
void SpecialKeys(int key, int x, int y)
{ 
	float linear = 0.40f;
	float angular = float(m3dDegToRad(2.5f));

	if(key == GLUT_KEY_UP)
		cameraFrame.MoveForward(linear);

	if(key == GLUT_KEY_DOWN)
		cameraFrame.MoveForward(-linear);

	if(key == GLUT_KEY_LEFT)
		cameraFrame.RotateWorld(angular, 0.0f, 1.0f, 0.0f);

	if(key == GLUT_KEY_RIGHT)
		cameraFrame.RotateWorld(-angular, 0.0f, 1.0f, 0.0f);
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
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vWhite);
		sphereBatch.Draw();
	modelViewMatrix.PopMatrix();

	// Rysowanie obiekt�w wzgl�dem kamery
	modelViewMatrix.PushMatrix();
		modelViewMatrix.Translate(0.0f, 0.2f, -2.5f);
		modelViewMatrix.Rotate(yRot, 0.0f, 1.0f, 0.0f);
	
		shaderManager.UseStockShader(GLT_SHADER_POINT_LIGHT_DIFF, 
				modelViewMatrix.GetMatrix(), 
				transformPipeline.GetProjectionMatrix(), 
				vLightTransformed, vGreen, 0);
		torusBatch.Draw();
	modelViewMatrix.PopMatrix();
}


///////////////////////////////////////////////////////////////////////////////
// Rednerowanie klatki. Szkielet odpowiada za zamiany bufor�w,
// opr�niania bufor�w itp.
void RenderScene(void)
{
	static CStopWatch animationTimer;
	float yRot = animationTimer.GetElapsedSeconds() * 60.0f;

	M3DVector3f vCameraPos;
	M3DVector3f vCameraForward;
	M3DVector3f vMirrorPos;
	M3DVector3f vMirrorForward;
	cameraFrame.GetOrigin(vCameraPos);
	cameraFrame.GetForwardVector(vCameraForward);

	// Ustawienie po�o�enia lustra (kamery)
	vMirrorPos[0] = 0.0;
	vMirrorPos[1] = 0.1f;
	vMirrorPos[2] = -6.0f; // Pozycja widoku jest za lustrem
	mirrorFrame.SetOrigin(vMirrorPos);

	// Obliczenie kierunku lustra (kamery)
	// Poniewa� po�o�enie lustra jest ustalone wzgl�dem pocz�tku uk�adu, znajdujemy wektor kierunku poprzez dodanie przesuni�cia
	// lustra do wektora obserwatora � pocz�tku uk�adu
	vMirrorForward[0] = vCameraPos[0];
	vMirrorForward[1] = vCameraPos[1];
	vMirrorForward[2] = (vCameraPos[2] + 5);
	m3dNormalizeVector3(vMirrorForward);
	mirrorFrame.SetForwardVector(vMirrorForward);
	
	// Renderowanie z perspektywy lustra
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboName);
	glDrawBuffers(1, fboBuffs);
	glViewport(0, 0, mirrorTexWidth, mirrorTexHeight);

	// Rysowanie sceny z perspektywy kamery lustra
	modelViewMatrix.PushMatrix();	
		M3DMatrix44f mMirrorView;
		mirrorFrame.GetCameraMatrix(mMirrorView);
		modelViewMatrix.MultMatrix(mMirrorView);

		// Obr�cenie kamery lustra w poziomie w celu uzyskania odbicia
		modelViewMatrix.Scale(-1.0f, 1.0f, 1.0f);
		
		glBindTexture(GL_TEXTURE_2D, textures[0]); // Marmur
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shaderManager.UseStockShader(GLT_SHADER_TEXTURE_MODULATE, transformPipeline.GetModelViewProjectionMatrix(), vWhite, 0);
		floorBatch.Draw();
		DrawWorld(yRot);

		// Narysowanie sto�ka reprezentuj�cego obserwatora
		M3DVector4f vLightTransformed;
		modelViewMatrix.GetMatrix(mMirrorView);
		m3dTransformVector4(vLightTransformed, vLightPos, mMirrorView);
		modelViewMatrix.Translate(vCameraPos[0],vCameraPos[1]-0.8f,vCameraPos[2]-1.0f);	
		modelViewMatrix.Rotate(-90.0f, 1.0f, 0.0f, 0.0f);

		shaderManager.UseStockShader(GLT_SHADER_POINT_LIGHT_DIFF, 
					modelViewMatrix.GetMatrix(), 
					transformPipeline.GetProjectionMatrix(), 
					vLightTransformed, vBlue, 0);
		cylinderBatch.Draw();
	modelViewMatrix.PopMatrix();

	// Reset FBO. Narysowanie �wiata jeszcze raz z perspektywy prawdziwych kamer
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glDrawBuffers(1, windowBuff);
	glViewport(0, 0, screenWidth, screenHeight);
	modelViewMatrix.PushMatrix();	
		M3DMatrix44f mCamera;
		cameraFrame.GetCameraMatrix(mCamera);
		modelViewMatrix.MultMatrix(mCamera);
		
		glBindTexture(GL_TEXTURE_2D, textures[0]); // Marmur
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shaderManager.UseStockShader(GLT_SHADER_TEXTURE_MODULATE, transformPipeline.GetModelViewProjectionMatrix(), vWhite, 0);

		floorBatch.Draw();
		DrawWorld(yRot);

		// Rysowanie powierzchni lustra
		modelViewMatrix.PushMatrix();
			modelViewMatrix.Translate(0.0f, -0.4f, -5.0f);
			if(vCameraPos[2] > -5.0)
			{
				glBindTexture(GL_TEXTURE_2D, mirrorTexture); // Odbicie
				shaderManager.UseStockShader(GLT_SHADER_TEXTURE_REPLACE, transformPipeline.GetModelViewProjectionMatrix(), 0);
			}
			else
			{
				// Je�li kamera znajduje si� za lustrem, stosujemy czarne wype�nienie
				shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlack);
			}
			mirrorBatch.Draw();
			shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vGrey);
			mirrorBorderBatch.Draw();
		modelViewMatrix.PopMatrix();
	modelViewMatrix.PopMatrix();
	
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
    fboName = 0;
    depthBufferName = 0;

	gltSetWorkingDirectory(argv[0]);
		
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(screenWidth,screenHeight);
  
    glutCreateWindow("FBO Textures");
 
    glutReshapeFunc(ChangeSize);
    glutDisplayFunc(RenderScene);
    glutSpecialFunc(SpecialKeys);

    SetupRC();
    glutMainLoop();    
    ShutdownRC();
    return 0;
}
