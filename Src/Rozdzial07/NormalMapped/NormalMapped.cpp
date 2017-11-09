// NormalMapped.cpp
// OpenGL. Ksiêga eksperta
// Demonstruje normalne mapowanie
// Autor programu: Richard S. Wright Jr.

#include <GLTools.h>	// Biblioteka OpenGL
#include <GLMatrixStack.h>
#include <GLFrame.h>
#include <GLFrustum.h>
#include <GLGeometryTransform.h>
#include <StopWatch.h>
#include <stdlib.h>

#include <math.h>
#ifdef __APPLE__
#include <glut/glut.h>
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>
#endif


GLFrame             viewFrame;
GLFrustum           viewFrustum;
GLTriangleBatch     sphereBatch;
GLMatrixStack       modelViewMatrix;
GLMatrixStack       projectionMatrix;
GLGeometryTransform transformPipeline;
GLShaderManager     shaderManager;

GLuint	normalMapShader;	// Shader teksturowego rozproszonego œwiat³a
GLint	locAmbient;			// Lokalizacja koloru otoczenia
GLint   locDiffuse;			// Lokalizacja koloru rozproszonego
GLint	locLight;			// Lokalizacja œwiat³a we wsp³rzêdnych oka
GLint	locMVP;				// Lokalizacja zmiennej uniform przechowuj¹cej macierz rzutowania model-widok
GLint	locMV;				// Lokalizacja zmiennej uniform macierzy model-widok
GLint	locNM;				// Lokalizacja zmiennej uniform macierzy normalnej
GLint   locColorMap;        // Lokalizacja samplera mapy kolorów
GLint   locNormalMap;       // Lokalizacja samplera normalnej mapy
GLuint  texture[2];         // Dwie tekstury, mapa kolorów i mapa normalna

/// Wczytanie TGA jako tekstury dwuwymiarowej. Pe³na inicjalizacja stanu
bool LoadTGATexture(const char *szFileName, GLenum minFilter, GLenum magFilter, GLenum wrapMode)
{
	GLbyte *pBits;
	int nWidth, nHeight, nComponents;
	GLenum eFormat;
	
	// Wczytanie bitów tekstury
	pBits = gltReadTGABits(szFileName, &nWidth, &nHeight, &nComponents, &eFormat);
	if(pBits == NULL) 
		return false;
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
    
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, nComponents, nWidth, nHeight, 0,
				 eFormat, GL_UNSIGNED_BYTE, pBits);
	
    free(pBits);
    
    if(minFilter == GL_LINEAR_MIPMAP_LINEAR || 
       minFilter == GL_LINEAR_MIPMAP_NEAREST ||
       minFilter == GL_NEAREST_MIPMAP_LINEAR ||
       minFilter == GL_NEAREST_MIPMAP_NEAREST)
        glGenerateMipmap(GL_TEXTURE_2D);
    
	return true;
}

// Ta funkcja wykonuje wszystkie dzia³ania zwi¹zane z inicjalizowaniem w kontekœcie renderowania
 
void SetupRC(void)
	{
	// T³o
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f );

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

    shaderManager.InitializeStockShaders();
    viewFrame.MoveForward(4.0f);

    // Tworzenie kuli
    gltMakeSphere(sphereBatch, 1.0f, 52, 26);

	normalMapShader = gltLoadShaderPairWithAttributes("NormalMapped.vp", "NormalMapped.fp", 3, GLT_ATTRIBUTE_VERTEX, "vVertex",
			GLT_ATTRIBUTE_NORMAL, "vNormal", GLT_ATTRIBUTE_TEXTURE0, "vTexture0");

	locAmbient = glGetUniformLocation(normalMapShader, "ambientColor");
	locDiffuse = glGetUniformLocation(normalMapShader, "diffuseColor");
	locLight = glGetUniformLocation(normalMapShader, "vLightPosition");
	locMVP = glGetUniformLocation(normalMapShader, "mvpMatrix");
	locMV  = glGetUniformLocation(normalMapShader, "mvMatrix");
	locNM  = glGetUniformLocation(normalMapShader, "normalMatrix");
	locColorMap = glGetUniformLocation(normalMapShader, "colorMap");
    locNormalMap = glGetUniformLocation(normalMapShader, "normalMap");

	glGenTextures(2, texture);
    glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	LoadTGATexture("IceMoon.tga", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture[1]);
    LoadTGATexture("IceMoonBump.tga", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_REPEAT);
	}

// Porz¹dkowanie
void ShutdownRC(void)
   {
   glDeleteTextures(2, texture);
   }


// Rysowanie sceny
void RenderScene(void)
	{
	static CStopWatch rotTimer;

	// Wyczyszczenie okna and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
    modelViewMatrix.PushMatrix(viewFrame);
        modelViewMatrix.Rotate(-90.0f, 1.0f, 0.0f, 0.0f);
		modelViewMatrix.Rotate(rotTimer.GetElapsedSeconds() * 10.0f, 0.0f, 0.0f, 1.0f);

		GLfloat vEyeLight[] = { -100.0f, 100.0f, 150.0f };
		GLfloat vAmbientColor[] = { 0.2f, 0.2f, 0.2f, 1.0f };
		GLfloat vDiffuseColor[] = { 1.0f, 1.0f, 1.0f, 1.0f};

		glUseProgram(normalMapShader);
		glUniform4fv(locAmbient, 1, vAmbientColor);
		glUniform4fv(locDiffuse, 1, vDiffuseColor);
		glUniform3fv(locLight, 1, vEyeLight);
		glUniformMatrix4fv(locMVP, 1, GL_FALSE, transformPipeline.GetModelViewProjectionMatrix());
		glUniformMatrix4fv(locMV, 1, GL_FALSE, transformPipeline.GetModelViewMatrix());
		glUniformMatrix3fv(locNM, 1, GL_FALSE, transformPipeline.GetNormalMatrix());
		glUniform1i(locColorMap, 0);
        glUniform1i(locNormalMap, 1);
    sphereBatch.Draw();

    modelViewMatrix.PopMatrix();

    glutSwapBuffers();
	glutPostRedisplay();
	}



void ChangeSize(int w, int h)
	{
	// Ochrona przed dzieleniem przez zero
	if(h == 0)
		h = 1;

	// Ustawienie widoku na rozmiar okna
    glViewport(0, 0, w, h);

    viewFrustum.SetPerspective(35.0f, float(w)/float(h), 1.0f, 100.0f);
    
    projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
    transformPipeline.SetMatrixStacks(modelViewMatrix, projectionMatrix);
	}

///////////////////////////////////////////////////////////////////////////////
// G³ówny punkt wejœciowy wszystkich programów opartych na bibliotece GLUT
int main(int argc, char* argv[])
    {
	gltSetWorkingDirectory(argv[0]);
	
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Lit Texture");
    glutReshapeFunc(ChangeSize);
    glutDisplayFunc(RenderScene);

	GLenum err = glewInit();
	if (GLEW_OK != err) {
		fprintf(stderr, "B³¹d GLEW: %s\n", glewGetErrorString(err));
		return 1;
    }
	
	SetupRC();    
	glutMainLoop();
	ShutdownRC();
	return 0;
    }
