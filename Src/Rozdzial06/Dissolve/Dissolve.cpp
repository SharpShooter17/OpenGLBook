// Dissolve.cpp
// OpenGL SuperBible
// Demonstruje zastosowanie instrukcji discard
// Program by Richard S. Wright Jr.

#include <GLTools.h>	// Biblioteka OpenGL
#include <GLMatrixStack.h>
#include <GLFrame.h>
#include <GLFrustum.h>
#include <GLGeometryTransform.h>
#include <StopWatch.h>

#include <math.h>
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

GLuint	ADSDissloveShader;	// Shader rozk³adu œwiat³a
GLint	locAmbient;			// Lokalizacja koloru otoczenia
GLint   locDiffuse;			// Lokalizacja koloru rozproszonego
GLint   locSpecular;		// Lokalizacja koloru odbicia zwierciadlanego
GLint	locLight;			// Lokalizacja œwiat³a we wspó³rzêdnych oka
GLint	locMVP;				// Lokalizacja zmiennej uniform macierzy rzutowania model-widok
GLint	locMV;				// Lokalizacja zmiennej uniform macierzy model-widok
GLint	locNM;				// Lokalizacja zmiennej uniform macierzy normalnych
GLint	locTexture;			// Lokalizacja zmiennej uniform tekstury
GLint   locDissolveFactor;  // Lokalizacja czynnika rozk³adu

GLuint	cloudTexture;		// Obiekt tekstury chmury



// Wczytanie pliku TGA jako tekstury dwuwymiarowej. Pe³ne zainicjalizowanie stanu
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


// Ta funkcja wykonuje wszystkie dzia³ania zwi¹zane z inicjalizowaniem w kontekœcie renderowania.

void SetupRC(void)
	{
	// T³o
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f );

	glEnable(GL_DEPTH_TEST);

    shaderManager.InitializeStockShaders();
    viewFrame.MoveForward(4.0f);

    // Tworzenie torusa
    gltMakeTorus(torusBatch, .80f, 0.25f, 52, 26);

	ADSDissloveShader = gltLoadShaderPairWithAttributes("Dissolve.vp", "Dissolve.fp", 3, GLT_ATTRIBUTE_VERTEX, "vVertex",
			GLT_ATTRIBUTE_NORMAL, "vNormal", GLT_ATTRIBUTE_TEXTURE0, "vTexCoords0");

	locAmbient = glGetUniformLocation(ADSDissloveShader, "ambientColor");
	locDiffuse = glGetUniformLocation(ADSDissloveShader, "diffuseColor");
	locSpecular = glGetUniformLocation(ADSDissloveShader, "specularColor");
	locLight = glGetUniformLocation(ADSDissloveShader, "vLightPosition");
	locMVP = glGetUniformLocation(ADSDissloveShader, "mvpMatrix");
	locMV  = glGetUniformLocation(ADSDissloveShader, "mvMatrix");
	locNM  = glGetUniformLocation(ADSDissloveShader, "normalMatrix");
	locTexture = glGetUniformLocation(ADSDissloveShader, "cloudTexture");
	locDissolveFactor = glGetUniformLocation(ADSDissloveShader, "dissolveFactor");

	glGenTextures(1, &cloudTexture);
	glBindTexture(GL_TEXTURE_1D, cloudTexture);
	LoadTGATexture("Clouds.tga", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE);
	}

// Czyszczenie
void ShutdownRC(void)
{

}


// Rysowanie sceny
void RenderScene(void)
	{
	static CStopWatch rotTimer;

	// Wyczyszczenie okna i bufora g³êbi
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
    modelViewMatrix.PushMatrix(viewFrame);
		modelViewMatrix.Rotate(rotTimer.GetElapsedSeconds() * 10.0f, 0.0f, 1.0f, 0.0f);

		GLfloat vEyeLight[] = { -100.0f, 100.0f, 100.0f };
		GLfloat vAmbientColor[] = { 0.1f, 0.1f, 0.1f, 1.0f };
		GLfloat vDiffuseColor[] = { 0.1f, 1.0f, 0.1f, 1.0f };
		GLfloat vSpecularColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };

		glUseProgram(ADSDissloveShader);
		glUniform4fv(locAmbient, 1, vAmbientColor);
		glUniform4fv(locDiffuse, 1, vDiffuseColor);
		glUniform4fv(locSpecular, 1, vSpecularColor);
		glUniform3fv(locLight, 1, vEyeLight);
		glUniformMatrix4fv(locMVP, 1, GL_FALSE, transformPipeline.GetModelViewProjectionMatrix());
		glUniformMatrix4fv(locMV, 1, GL_FALSE, transformPipeline.GetModelViewMatrix());
		glUniformMatrix3fv(locNM, 1, GL_FALSE, transformPipeline.GetNormalMatrix());
		glUniform1i(locTexture, 1);

		float fFactor = fmod(rotTimer.GetElapsedSeconds(), 10.0f);
		fFactor /= 10.0f;
		glUniform1f(locDissolveFactor, fFactor);
    torusBatch.Draw();

    modelViewMatrix.PopMatrix();


    glutSwapBuffers();
	glutPostRedisplay();
	}



void ChangeSize(int w, int h)
	{
	// Zabezpieczenie przed dzieleniem przez zero
	if(h == 0)
		h = 1;

	// Ustawienie widoku na wymiary okna
    glViewport(0, 0, w, h);

    viewFrustum.SetPerspective(35.0f, float(w)/float(h), 1.0f, 100.0f);
    
    projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
    transformPipeline.SetMatrixStacks(modelViewMatrix, projectionMatrix);
	}

///////////////////////////////////////////////////////////////////////////////
// Punkt rozpoczêcia wykonywania ka¿dego programu opartego na bibliotece GLUT
int main(int argc, char* argv[])
    {
	gltSetWorkingDirectory(argv[0]);
	
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Pomocy, rozk³adam siê!");
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
