// DiffuseLight.cpp
// OpenGL. Ksiêga eksperta
// Demonstruje proste œwiat³o rozproszone
// Autor programu: Richard S. Wright Jr.

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
GLTriangleBatch     sphereBatch;
GLMatrixStack       modelViewMatrix;
GLMatrixStack       projectionMatrix;
GLGeometryTransform transformPipeline;
GLShaderManager     shaderManager;

GLuint	diffuseLightShader;	// Shader œwiat³a rozproszonego
GLint	locColor;			// Lokalizacja koloru rozproszonego
GLint	locLight;			// Lokalizacja œwiat³a we wspó³rzêdnych oka
GLint	locMVP;				// Lokalizacja zmiennej uniform macierzy rzutowania model-widok
GLint	locMV;				// Lokalizacja zmiennej uniform macierzy model-widok
GLint	locNM;				// Lokalizacja zmiennej uniform macierzy normalnych


// Ta funkcja wykonuje wszystkie dzia³ania zwi¹zane z inicjalizowaniem w kontekœcie renderowania.

void SetupRC(void)
	{
	// T³o
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f );

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

    shaderManager.InitializeStockShaders();
    viewFrame.MoveForward(4.0f);

    // Tworzenie kuli
    gltMakeSphere(sphereBatch, 1.0f, 26, 13);

	diffuseLightShader = shaderManager.LoadShaderPairWithAttributes("DiffuseLight.vp", "DiffuseLight.fp", 2, GLT_ATTRIBUTE_VERTEX, "vVertex",
			GLT_ATTRIBUTE_NORMAL, "vNormal");

	locColor = glGetUniformLocation(diffuseLightShader, "diffuseColor");
	locLight = glGetUniformLocation(diffuseLightShader, "vLightPosition");
	locMVP = glGetUniformLocation(diffuseLightShader, "mvpMatrix");
	locMV  = glGetUniformLocation(diffuseLightShader, "mvMatrix");
	locNM  = glGetUniformLocation(diffuseLightShader, "normalMatrix");
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
		GLfloat vDiffuseColor[] = { 0.0f, 0.0f, 1.0f, 1.0f };

		glUseProgram(diffuseLightShader);
		glUniform4fv(locColor, 1, vDiffuseColor);
		glUniform3fv(locLight, 1, vEyeLight);
		glUniformMatrix4fv(locMVP, 1, GL_FALSE, transformPipeline.GetModelViewProjectionMatrix());
		glUniformMatrix4fv(locMV, 1, GL_FALSE, transformPipeline.GetModelViewMatrix());
		glUniformMatrix3fv(locNM, 1, GL_FALSE, transformPipeline.GetNormalMatrix());
    sphereBatch.Draw();

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
	glutCreateWindow("Proste œwiat³o rozproszone");
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
