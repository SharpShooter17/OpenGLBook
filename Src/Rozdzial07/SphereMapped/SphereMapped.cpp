// SphereMapped.cpp
// OpenGL. Ksiêga eksperta
// Rzutowanie tekstury na kulê
// Autor programu: Richard S. Wright Jr.

#include <GLTools.h>	// Biblioteka OpenGL
#include <GLMatrixStack.h>
#include <GLFrame.h>
#include <GLFrustum.h>
#include <GLGeometryTransform.h>
#include <StopWatch.h>

#include <math.h>
#include <stdlib.h>

#ifdef __APPLE__
#include <glut/glut.h>
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>
#endif


GLFrame             viewFrame;
GLFrustum           viewFrustum;
GLTriangleBatch     torusBatch;
GLMatrixStack       modelViewMatrix;
GLMatrixStack       projectionMatrix;
GLGeometryTransform transformPipeline;
GLShaderManager     shaderManager;

GLuint	sphereMapShader;	// Shader œwiat³a rozproszonego
GLint	locMVP;				// Lokalizacja zmiennej uniform przechowuj¹cej macierz rzutowania model-widok
GLint   locMV;              // Lokalizacja macierzy model-widok
GLint   locNormalMatrix;    // Lokalizacja macierzy normalnej
GLint	locTexture;			// Lokalizacja zmiennej jednorodnej tekstury

GLuint	sphereTexture;		// Obiekt tekstury chmury



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

    shaderManager.InitializeStockShaders();
    viewFrame.MoveForward(4.0f);

    // Tworzenie torusa
    gltMakeTorus(torusBatch, .80f, 0.25f, 52, 26);

	sphereMapShader = gltLoadShaderPairWithAttributes("SphereMapped.vp", "SphereMapped.fp", 3, GLT_ATTRIBUTE_VERTEX, "vVertex",
			GLT_ATTRIBUTE_NORMAL, "vNormal", GLT_ATTRIBUTE_TEXTURE0, "vTexCoords0");

	locMVP = glGetUniformLocation(sphereMapShader, "mvpMatrix");
	locTexture = glGetUniformLocation(sphereMapShader, "sphereMap");
    locMV = glGetUniformLocation(sphereMapShader, "mvMatrix");
    locNormalMatrix = glGetUniformLocation(sphereMapShader, "normalMatrix");

	glGenTextures(1, &sphereTexture);
	glBindTexture(GL_TEXTURE_2D, sphereTexture);
	LoadTGATexture("SphereMap.tga", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE);
	}

// Porz¹dkowanie
void ShutdownRC(void)
{
    glDeleteTextures(1, &sphereTexture);
}


// Rysowanie sceny
void RenderScene(void)
	{
	static CStopWatch rotTimer;

	// Wyczyszczenie okna and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
    modelViewMatrix.PushMatrix(viewFrame);
		modelViewMatrix.Rotate(rotTimer.GetElapsedSeconds() * 10.0f, 0.0f, 1.0f, 0.0f);

		glUseProgram(sphereMapShader);
		glUniformMatrix4fv(locMVP, 1, GL_FALSE, transformPipeline.GetModelViewProjectionMatrix());
        glUniformMatrix4fv(locMV, 1, GL_FALSE, transformPipeline.GetModelViewMatrix());
        glUniformMatrix3fv(locNormalMatrix, 1, GL_FALSE, transformPipeline.GetNormalMatrix(false));
        glUniform1i(locTexture, 0);

        torusBatch.Draw();

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
	glutCreateWindow("Teksturowanie kuli");
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
