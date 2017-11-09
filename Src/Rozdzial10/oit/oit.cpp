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

#pragma warning( disable : 4305 )

static GLfloat vLtBlue[]    = { 0.00f, 0.00f, 1.00f, 0.90f };
static GLfloat vLtPink[]    = { 0.40f, 0.00f, 0.20f, 0.50f };
static GLfloat vLtYellow[]  = { 0.98f, 0.96f, 0.14f, 0.30f };
static GLfloat vLtMagenta[] = { 0.83f, 0.04f, 0.83f, 0.70f };
static GLfloat vLtGreen[]   = { 0.05f, 0.98f, 0.14f, 0.30f };

static GLfloat vGrey[]   = { 0.5f, 0.5f, 0.5f, 1.0f };

#define USER_OIT   1 
#define USER_BLEND 2

GLsizei	 screenWidth;			// ¯¹dana szerokoœæ okna lub pulpitu
GLsizei  screenHeight;			// ¯¹dana wysokoœæ okna lub pulpitu

GLboolean bFullScreen;			// ¯¹danie dzia³ania w trybie pe³noekranowym
GLboolean bAnimated;			// ¯¹danie ci¹g³ych aktualizacji


GLShaderManager		shaderManager;			// Manager wierzcho³ków
GLMatrixStack		modelViewMatrix;		// Macierz model-widok
GLMatrixStack		projectionMatrix;		// Macierz rzutowania
GLFrustum			viewFrustum;			// Frusta widoku
GLGeometryTransform	transformPipeline;		// Potok przetwarzania geometrii
GLFrame				cameraFrame;			// Uk³ad odniesienia kamery

GLTriangleBatch		bckgrndCylBatch;
GLTriangleBatch		diskBatch;
GLBatch				glass1Batch;
GLBatch				glass2Batch;
GLBatch				glass3Batch;
GLBatch				glass4Batch;
GLBatch             screenQuad;
M3DMatrix44f        orthoMatrix;  
GLfloat             worldAngle;

GLint               blendMode;
GLint               mode;

GLuint              msFBO;
GLuint              textures[2];
GLuint		    msTexture[1];
GLuint              depthTextureName; 
GLuint              msResolve;
GLuint              oitResolve;
GLuint              flatBlendProg;

int					Sampling;

void DrawWorld();
bool LoadBMPTexture(const char *szFileName, GLenum minFilter, GLenum magFilter, GLenum wrapMode);
void GenerateOrtho2DMat(GLuint imageWidth, GLuint imageHeight);
void SetupResolveProg();
void SetupOITResolveProg();


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
        glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);

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

    // Inicjalizacja managera shaderów
    shaderManager.InitializeStockShaders();
    glEnable(GL_DEPTH_TEST);

    gltMakeCylinder(bckgrndCylBatch, 4.0, 4.0, 5.2, 1024, 1);

    gltMakeDisk(diskBatch, 0.0, 1.5, 40, 10);

    glass1Batch.Begin(GL_TRIANGLE_FAN, 4, 1);
        glass1Batch.Vertex3f(-1.0f, -1.0f, 0.0f);
        glass1Batch.Vertex3f( 1.0f, -1.0f, 0.0f);
        glass1Batch.Vertex3f( 1.0f,  1.0f, 0.0f);
        glass1Batch.Vertex3f(-1.0f,  1.0f, 0.0f);
    glass1Batch.End();

    glass2Batch.Begin(GL_TRIANGLE_FAN, 4, 1);
        glass2Batch.Vertex3f( 0.0f,  1.0f, 0.0f);
        glass2Batch.Vertex3f( 1.0f,  0.0f, 0.0f);
        glass2Batch.Vertex3f( 0.0f, -1.0f, 0.0f);
        glass2Batch.Vertex3f(-1.0f,  0.0f, 0.0f);
    glass2Batch.End();
        
    glass3Batch.Begin(GL_TRIANGLE_FAN, 3, 1);
        glass3Batch.Vertex3f( 0.0f,  1.0f, 0.0f);
        glass3Batch.Vertex3f( 1.0f, -1.0f, 0.0f);
        glass3Batch.Vertex3f(-1.0f,  -1.0f, 0.0f);
    glass3Batch.End();

    glass4Batch.Begin(GL_TRIANGLE_FAN, 4, 1);
        glass4Batch.Vertex3f(-1.0f,  1.0f, 0.0f);
        glass4Batch.Vertex3f( 1.0f,  0.5f, 0.0f);
        glass4Batch.Vertex3f( 1.0f, -1.0f, 0.0f);
        glass4Batch.Vertex3f(-1.0f, -0.5f, 0.0f);
    glass4Batch.End();

    glGenTextures(2, textures);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    LoadBMPTexture("marble.bmp", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_REPEAT);
    glBindTexture(GL_TEXTURE_2D, textures[1]);
    LoadBMPTexture("start_line.bmp", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_REPEAT);

	Sampling = 8;
	bool SamplingFailed = false;
	do{
		if(SamplingFailed){
			Sampling /=2;
		}
		fprintf(stderr, "Liczba próbek: %i\n", Sampling);

		// Utworzenie i zwi¹zanie FBO
		glGenFramebuffers(1,&msFBO);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, msFBO);

		// Utworzenie tekstury g³êbi
		glGenTextures(1, &depthTextureName);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, depthTextureName);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, Sampling/*8*/, GL_DEPTH_COMPONENT24, screenWidth, screenHeight, GL_FALSE);
	             
		// Konfiguracja tekstur HDR
		glGenTextures(1, msTexture);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msTexture[0]);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, Sampling/*8*/, GL_RGBA8, screenWidth, screenHeight, GL_FALSE);
    
		// Utworzenie i zwi¹zanie FBO
		glGenFramebuffers(1, &msFBO);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, msFBO);

		// Zwi¹zanie tekstury z pierwszym punktem wi¹zania kolorów i RBO g³êbi
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, msTexture[0], 0);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, depthTextureName, 0);

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


	// Reset wi¹zañ bufora obrazu
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    // Wczytanie shadera oitResolve
    oitResolve =  gltLoadShaderPairWithAttributes("basic.vs", "oitResolve.fs", 3, 
                            GLT_ATTRIBUTE_VERTEX, "vVertex", 
                            GLT_ATTRIBUTE_NORMAL, "vNormal", 
                            GLT_ATTRIBUTE_TEXTURE0, "vTexCoord0");
    glBindFragDataLocation(oitResolve, 0, "oColor");
    glLinkProgram(oitResolve);

	// Wczytanie shadera msResolve
    msResolve =  gltLoadShaderPairWithAttributes("basic.vs", "msResolve.fs", 3, 
                            GLT_ATTRIBUTE_VERTEX, "vVertex", 
                            GLT_ATTRIBUTE_NORMAL, "vNormal", 
                            GLT_ATTRIBUTE_TEXTURE0, "vTexCoord0");

    glBindFragDataLocation(msResolve, 0, "oColor");
    glLinkProgram(msResolve);

    // Upewnienie siê, czy wszystko posz³o dobrze
    gltCheckErrors(oitResolve);
    gltCheckErrors(msResolve);
    
    int numMasks = 0;
    glGetIntegerv(GL_MAX_SAMPLE_MASK_WORDS, &numMasks);
}


///////////////////////////////////////////////////////////////////////////////
// Procedury porz¹dkowe, zwalnianie tekstur, obiektów bufora itp.
void ShutdownRC(void)
{
    // Upewnienie siê, ¿e domyœlny FBO jest zwi¹zany
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

    // Czyszczenie tekstur
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    glDeleteTextures(1, msTexture);
    glDeleteTextures(1, &depthTextureName);
    glDeleteTextures(1, textures);

    // Czyszczenie FBO
    glDeleteFramebuffers(1, &msFBO);

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

    GenerateOrtho2DMat(nWidth, nHeight);

    // Aktualizacja wymiarów ekranu
    screenWidth = nWidth;
    screenHeight = nHeight;

    // Zmiana rozmiaru tekstur
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, depthTextureName);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, Sampling/*8*/, GL_DEPTH_COMPONENT24, screenWidth, screenHeight, GL_FALSE);

    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msTexture[0]);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, Sampling/*8*/, GL_RGBA8, screenWidth, screenHeight, GL_FALSE);
}


///////////////////////////////////////////////////////////////////////////////
// Aktualizacja kamery na podstawie danych od u¿ytkownika, prze³¹czanie trybów wyœwietlania
// 
void SpecialKeys(int key, int x, int y)
{ 
    static CStopWatch cameraTimer;
    float fTime = cameraTimer.GetElapsedSeconds();
    cameraTimer.Reset(); 

    float linear = fTime * 3.0f;
    float angular = fTime * float(m3dDegToRad(60.0f));

    if(key == GLUT_KEY_LEFT)
    {
        worldAngle += angular*50;
        if(worldAngle > 360)
            worldAngle -= 360;
    }

    if(key == GLUT_KEY_RIGHT)
    {
        worldAngle -= angular*50;
        if(worldAngle < 360)
            worldAngle += 360;
    }
}
void ProcessKeys(unsigned char key, int x, int y)
{
    if(key == 'o' || key == 'O')
        mode = USER_OIT;
    if(key == 'b' || key == 'B')
    	mode = USER_BLEND;

    if(key == '1')
    	blendMode = 1;
    if(key == '2')
    	blendMode = 2;
    if(key == '3')
    	blendMode = 3;
    if(key == '4')
    	blendMode = 4;
    if(key == '5')
    	blendMode = 5;
    if(key == '6')
    	blendMode = 6;
    if(key == '7')
    	blendMode = 7;
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


void SetupResolveProg()
{
    glUseProgram(msResolve);

    // Konfiguracja macierzy rzutowania
    glUniformMatrix4fv(glGetUniformLocation(msResolve, "pMatrix"), 
        1, GL_FALSE, transformPipeline.GetProjectionMatrix());

    // Macierz rzutowania model-widok
    glUniformMatrix4fv(glGetUniformLocation(msResolve, "mvMatrix"), 
        1, GL_FALSE, transformPipeline.GetModelViewMatrix());

    // Konfiguracja odpowiednich tekstur
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msTexture[0]);
    glUniform1i(glGetUniformLocation(msResolve, "origImage"), 0);

    glUniform1i(glGetUniformLocation(msResolve, "sampleCount"), 8);
    
    glActiveTexture(GL_TEXTURE0);
	    
	gltCheckErrors(msResolve);
} 

void SetupOITResolveProg()
{
    glUseProgram(oitResolve);

    // Konfiguracja macierzy rzutowania
    glUniformMatrix4fv(glGetUniformLocation(oitResolve, "pMatrix"), 
        1, GL_FALSE, transformPipeline.GetProjectionMatrix());

    // Macierz rzutowania model-widok
    glUniformMatrix4fv(glGetUniformLocation(oitResolve, "mvMatrix"), 
        1, GL_FALSE, transformPipeline.GetModelViewMatrix());

    // Konfiguracja odpowiednich tekstur
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msTexture[0]);
    glUniform1i(glGetUniformLocation(oitResolve, "origImage"), 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, depthTextureName);
    glUniform1i(glGetUniformLocation(oitResolve, "origDepth"), 1);

    glUniform1f(glGetUniformLocation(oitResolve, "sampleCount"), 8);
    
    glActiveTexture(GL_TEXTURE0);
    gltCheckErrors(oitResolve);
} 

///////////////////////////////////////////////////////////////////////////////
// Rysowanie sceny
// 
void DrawWorld()
{
    modelViewMatrix.Translate(0.0f, 0.8f, 0.0f);
    modelViewMatrix.PushMatrix();
        modelViewMatrix.Translate(-0.3f, 0.f, 0.0f);
        modelViewMatrix.Scale(0.40, 0.8, 0.40);
        modelViewMatrix.Rotate(50.0, 0.0, 10.0, 0.0);
		if(0x02 <= Sampling)
			glSampleMaski(0, 0x02);
		else
			glSampleMaski(0, Sampling);
        shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vLtYellow);
        glass1Batch.Draw();
    modelViewMatrix.PopMatrix();

    modelViewMatrix.PushMatrix();
        modelViewMatrix.Translate(0.4f, 0.0f, 0.0f);
        modelViewMatrix.Scale(0.5, 0.8, 1.0);
        modelViewMatrix.Rotate(-20.0, 0.0, 1.0, 0.0);
        if(0x04 <= Sampling)
			glSampleMaski(0, 0x04);
		else
			glSampleMaski(0, Sampling);
        shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vLtGreen);
        glass2Batch.Draw();
    modelViewMatrix.PopMatrix();

    modelViewMatrix.PushMatrix();
        modelViewMatrix.Translate(1.0f, 0.0f, -0.6f);
        modelViewMatrix.Scale(0.3, 0.9, 1.0);
        modelViewMatrix.Rotate(-40.0, 0.0, 1.0, 0.0);
        if(0x08 <= Sampling)
			glSampleMaski(0, 0x08);
		else
			glSampleMaski(0, Sampling);		
        shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vLtMagenta);
        glass3Batch.Draw();
    modelViewMatrix.PopMatrix();

    modelViewMatrix.PushMatrix();
        modelViewMatrix.Translate(-0.8f, 0.0f, -0.60f);
        modelViewMatrix.Scale(0.6, 0.9, 0.40);
        modelViewMatrix.Rotate(60.0, 0.0, 1.0, 0.0);
        if(0x10 <= Sampling)
			glSampleMaski(0, 0x10);
		else
			glSampleMaski(0, Sampling);	
        shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vLtBlue);
        glass4Batch.Draw();
    modelViewMatrix.PopMatrix();

    modelViewMatrix.PushMatrix();
        modelViewMatrix.Translate(0.1f, 0.0f, 0.50f);
        modelViewMatrix.Scale(0.4, 0.9, 0.4);
        modelViewMatrix.Rotate(205.0, 0.0, 1.0, 0.0);
        if(0x20 <= Sampling)
			glSampleMaski(0, 0x20);
		else
			glSampleMaski(0, Sampling);	
        shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vLtPink);
        glass4Batch.Draw();
    modelViewMatrix.PopMatrix();
}

///////////////////////////////////////////////////////////////////////////////
// Rednerowanie klatki. Szkielet odpowiada za zamiany buforów,
// opró¿niania buforów itp.
void RenderScene(void)
{
    // Wi¹zanie FBO z buforami wielopróbkowania
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, msFBO);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Wybrana przez u¿ytkownika przezroczystoœæ niezale¿na od kolejnoœci
    if (mode == USER_OIT)
    {
        glSampleMaski(0, 0x01);
        glEnable(GL_SAMPLE_MASK);

		// Uniemo¿liwienie usuniêcia przykrytych powierzchni
        glDepthFunc(GL_ALWAYS);
    }
    
    modelViewMatrix.PushMatrix();	
      M3DMatrix44f mCamera;
      cameraFrame.GetCameraMatrix(mCamera);
      modelViewMatrix.MultMatrix(mCamera);

      modelViewMatrix.PushMatrix();	
        modelViewMatrix.Translate(0.0f, -0.4f, -4.0f);
        modelViewMatrix.Rotate(worldAngle, 0.0, 1.0, 0.0);

		// Rysowanie t³a i dysku w pierwszej próbce
        modelViewMatrix.PushMatrix();
          modelViewMatrix.Translate(0.0f, 3.0f, 0.0f);
          modelViewMatrix.Rotate(90.0, 1.0, 0.0, 0.0);
          modelViewMatrix.Rotate(90.0, 0.0, 0.0, 1.0);
          glBindTexture(GL_TEXTURE_2D, textures[1]); 
          shaderManager.UseStockShader(GLT_SHADER_TEXTURE_REPLACE, transformPipeline.GetModelViewProjectionMatrix(), 0);
          bckgrndCylBatch.Draw();
        modelViewMatrix.PopMatrix();
        
        modelViewMatrix.Translate(0.0f, -0.3f, 0.0f);
        modelViewMatrix.PushMatrix();
            modelViewMatrix.Rotate(90.0, 1.0, 0.0, 0.0);
            shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vGrey);
            diskBatch.Draw();
        modelViewMatrix.PopMatrix();
		modelViewMatrix.Translate(0.0f, 0.1f, 0.0f);

		// Mieszanie wybrane przez u¿ytkownika
        if (mode == USER_BLEND)
        {
            // Ustawienie stanu mieszania
			glDisable(GL_DEPTH_TEST);
            glEnable(GL_BLEND);
            switch (blendMode)
            {
            case 1:
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                break;
            case 2:
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA);
                break;
            case 3:
                glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
                break;
            case 4:
                glBlendFunc(GL_SRC_ALPHA, GL_ONE);
                break;
            case 5:
                glBlendFunc(GL_SRC_ALPHA, GL_DST_COLOR);
                break;
            case 6:
                glBlendFuncSeparate(GL_SRC_ALPHA, GL_DST_ALPHA, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                break;
            case 7:
                glBlendFuncSeparate(GL_SRC_COLOR, GL_DST_COLOR, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                break;
            default:
                glDisable(GL_BLEND);
            }
        }
   
		// Rysowanie kawa³ków szk³a
        DrawWorld();
    
      modelViewMatrix.PopMatrix();
    modelViewMatrix.PopMatrix();
    
    // Czyszczenie stanu
    glDepthFunc(GL_LEQUAL);
    glDisable(GL_BLEND);
    glDisable(GL_SAMPLE_MASK);
    glSampleMaski(0, 0xffffffff);

    // Rozwi¹zanie bufora wielopróbkowania
    projectionMatrix.PushMatrix();
      projectionMatrix.LoadMatrix(orthoMatrix);
      modelViewMatrix.PushMatrix();
        modelViewMatrix.LoadIdentity();
		// Ustawienie i wyczyszczenie domyœlnego bufora obrazu
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glViewport(0, 0, screenWidth, screenHeight);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        if (mode == USER_OIT)
            SetupOITResolveProg();
        else if (mode == USER_BLEND)
            SetupResolveProg();

		// Narysowanie prostok¹ta do rozwi¹zania pwoierzchni wielopróbkowanych
        screenQuad.Draw();
      modelViewMatrix.PopMatrix();
    projectionMatrix.PopMatrix();
    
	// Wyzerowanie stanu tekstury
    glEnable(GL_DEPTH_TEST);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
    glActiveTexture(GL_TEXTURE1);
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
    msFBO = 0;
    depthTextureName = 0;
    worldAngle = 0;
    mode = 1;
    blendMode = 1;

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
