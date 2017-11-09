// ProvokingVertex.cpp
// Nasz pierwszy program OpenGL, kt�ry rysuje tr�jk�t na ekranie.

#include <GLTools.h>            // Biblioteka OpenGL
#include <GLShaderManager.h>    // Klasa mened�era shader�w

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
// Zmieni� si� rozmiar okna lub okno zosta�o w�a�nie utworzone. W obu przypadkach musimy
// sprawdzi� wymiary tego okna, aby ustawi� obszar widoku i utworzy� macierz rzutowania.
void ChangeSize(int w, int h)
    {
	glViewport(0, 0, w, h);
    }


///////////////////////////////////////////////////////////////////////////////
// Ta funkcja wykonuje wszystkie dzia�ania zwi�zane z inicjalizowaniem w kontek�cie renderowania. 
// Jest to pierwsza okazja do wykonania jakich� zada� zwi�zanych z OpenGL.
void SetupRC()
	{
	// Niebieskie t�o
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f );
    
	shaderManager.InitializeStockShaders();

	// Za�adowanie tr�jk�ta
	GLfloat vVerts[] = { -0.5f, 0.0f, 0.0f, 
		                  0.5f, 0.0f, 0.0f,
						  0.0f, 0.5f, 0.0f };

	GLfloat vColors [] = { 1.0f, 0.0f, 0.0f, 1.0f,
		                   0.0f, 1.0f, 0.0f, 1.0f,
						   0.0f, 0.0f, 1.0f, 1.0f };

	triangleBatch.Begin(GL_TRIANGLES, 3);
	triangleBatch.CopyVertexData3f(vVerts);
	triangleBatch.CopyColorData4f(vColors);
	triangleBatch.End();

	myIdentityShader = gltLoadShaderPairWithAttributes("ProvokingVertex.vp", "ProvokingVertex.fp", 2, 
		                            GLT_ATTRIBUTE_VERTEX, "vVertex", GLT_ATTRIBUTE_COLOR, "vColor");

	glProvokingVertex(GL_FIRST_VERTEX_CONVENTION);
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
	// Wyczyszczenie okna bie��cym kolorem
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	glUseProgram(myIdentityShader);
	triangleBatch.Draw();

	// Zamiana bufor�w w celu wy�wietlenia zawarto�ci bufora tylnego
	glutSwapBuffers();
	}

int nToggle = 1;
void KeyPressFunc(unsigned char key, int x, int y)
	{
	if(key == 32)
		{
		nToggle++;

		if(nToggle %2 == 0) {
			glProvokingVertex(GL_LAST_VERTEX_CONVENTION);
			glutSetWindowTitle("Prowokowanie wierzcho�k�w - ostatni wierzcho�ek - naci�nij spacj�");
			}
		else {
			glProvokingVertex(GL_FIRST_VERTEX_CONVENTION);
			glutSetWindowTitle("Prowokowanie wierzcho�k�w - pierwszy wierzcho�ek - naci�nij spacj�");
			}

		glutPostRedisplay();
		}
	}
       


///////////////////////////////////////////////////////////////////////////////
// Punkt rozpocz�cia wykonywania ka�dego programu opartego na bibliotece GLUT
int main(int argc, char* argv[])
	{
	gltSetWorkingDirectory(argv[0]);
	
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Prowokowanie wierzcho�k�w - pierwszy wierzcho�ek - naci�nij spacj�");
    glutReshapeFunc(ChangeSize);
    glutDisplayFunc(RenderScene);
	glutKeyboardFunc(KeyPressFunc);


	GLenum err = glewInit();
	if (GLEW_OK != err) {
		fprintf(stderr, "B��d GLEW: %s\n", glewGetErrorString(err));
		return 1;
		}
	
	SetupRC();

	glutMainLoop();

	ShutdownRC();

	return 0;
	}
