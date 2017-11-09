// Triangle.cpp
// Nasz pierwszy program OpenGL, który rysuje trójk¹t na ekranie.

#include <GLTools.h>            // Biblioteka OpenGL
#include <GLShaderManager.h>    // Klasa mened¿era shaderów

#ifdef __APPLE__
#include <glut/glut.h>          // Wersja biblioteki GLUT dla systemu OS X
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>            // Biblioteka FreeGlut dla systemu Windows
#endif

GLBatch	triangleBatch;
GLShaderManager	shaderManager;

GLint	myIdentityShader;

///////////////////////////////////////////////////////////////////////////////
// Zmieni³ siê rozmiar okna lub okno zosta³o w³aœnie utworzone. W obu przypadkach musimy
// sprawdziæ wymiary tego okna, aby ustawiæ obszar widoku i utworzyæ macierz rzutowania.
void ChangeSize(int w, int h)
    {
	glViewport(0, 0, w, h);
    }


///////////////////////////////////////////////////////////////////////////////
// Ta funkcja wykonuje wszystkie dzia³ania zwi¹zane z inicjalizowaniem w kontekœcie renderowania. 
// Jest to pierwsza okazja do wykonania jakichœ zadañ zwi¹zanych z OpenGL.
void SetupRC()
	{
	// Niebieskie t³o
	glClearColor(0.0f, 0.0f, 1.0f, 1.0f );
    
	shaderManager.InitializeStockShaders();

	// Za³adowanie trójk¹ta
	GLfloat vVerts[] = { -0.5f, 0.0f, 0.0f, 
		                  0.5f, 0.0f, 0.0f,
						  0.0f, 0.5f, 0.0f };

	triangleBatch.Begin(GL_TRIANGLES, 3);
	triangleBatch.CopyVertexData3f(vVerts);
	triangleBatch.End();

	myIdentityShader = shaderManager.LoadShaderPairWithAttributes("Identity.vp", "Identity.fp", 1, GLT_ATTRIBUTE_VERTEX, "vVertex");
	}


///////////////////////////////////////////////////////////////////////////////
// Czyszczenie
void ShutdownRC()
   {
   glDeleteProgram(myIdentityShader);

   }


///////////////////////////////////////////////////////////////////////////////
// Rysowanie sceny
void RenderScene(void)
	{
	// Wyczyszczenie okna bie¿¹cym kolorem
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	GLfloat vRed[] = { 1.0f, 0.0f, 0.0f, 1.0f };
	glUseProgram(myIdentityShader);

	GLint iColorLocation = glGetUniformLocation(myIdentityShader, "vColor");
	GLfloat vColor[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
	glUniform4fv(iColorLocation, 1, vColor);

	triangleBatch.Draw();

	// Zamiana buforów w celu wyœwietlenia zawartoœci bufora tylnego
	glutSwapBuffers();
	}


///////////////////////////////////////////////////////////////////////////////
// Punkt rozpoczêcia wykonywania ka¿dego programu opartego na bibliotece GLUT
int main(int argc, char* argv[])
	{
	gltSetWorkingDirectory(argv[0]);
	
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Triangle");
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
