// Bounce.cpp
// Odbijanie bloku w obr�bie okna

#include <GLTools.h>	// Biblioteka OpenGL
#include <GLShaderManager.h>

#ifdef __APPLE__
#include <glut/glut.h>
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>
#endif

GLBatch	squareBatch;
GLShaderManager	shaderManager;


GLfloat blockSize = 0.1f;
GLfloat vVerts[] = { -blockSize - 0.5f, -blockSize, 0.0f, 
	                  blockSize - 0.5f, -blockSize, 0.0f,
					  blockSize - 0.5f,  blockSize, 0.0f,
					 -blockSize - 0.5f,  blockSize, 0.0f};

///////////////////////////////////////////////////////////////////////////////
// Ta funkcja wykonuje wszystkie dzia�ania zwi�zane z inicjalizowaniem w kontek�cie renderowania. 
// Jest to pierwsza okazja do wykonania jakich� zada� zwi�zanych z OpenGL.
void SetupRC()
	{
	// Czarne t�o
	glClearColor(0.0f, 0.0f, 1.0f, 1.0f );
    
	shaderManager.InitializeStockShaders();

	// �adowanie tr�jk�ta
	squareBatch.Begin(GL_TRIANGLE_FAN, 4);
	squareBatch.CopyVertexData3f(vVerts);
	squareBatch.End();
	}

// Odpowiadanie na naci�ni�cia klawiszy strza�ek poprzez poruszanie uk�adu odniesienia kamery
void BounceFunction(void)
    {
	static GLfloat xDir = 1.0f;
	static GLfloat yDir = 1.0f;

	GLfloat stepSize = 0.005f;

	GLfloat blockX = vVerts[0];   // Wsp�rz�dna X lewego g�rnego rogu
	GLfloat blockY = vVerts[7];  // Wsp�rz�dna Y lewego g�rnego rogu

	blockY += stepSize * yDir;
	blockX += stepSize * xDir;

	// Wykrywanie zetkni�cia z kraw�dzi� okna
	if(blockX < -1.0f) { blockX = -1.0f; xDir *= -1.0f; }
	if(blockX > (1.0f - blockSize * 2)) { blockX = 1.0f - blockSize * 2; xDir *= -1.0f; }
	if(blockY < -1.0f + blockSize * 2)  { blockY = -1.0f + blockSize * 2; yDir *= -1.0f; }
	if(blockY > 1.0f) { blockY = 1.0f; yDir *= -1.0f; }

	// Ponowne obliczenie po�o�enia wierzcho�k�w
	vVerts[0] = blockX;
	vVerts[1] = blockY - blockSize*2;
	
	vVerts[3] = blockX + blockSize*2;
	vVerts[4] = blockY - blockSize*2;
	
	vVerts[6] = blockX + blockSize*2;
	vVerts[7] = blockY;

	vVerts[9] = blockX;
	vVerts[10] = blockY;

	squareBatch.CopyVertexData3f(vVerts);
	}





///////////////////////////////////////////////////////////////////////////////
// Rysowanie sceny
void RenderScene(void)
	{
	// Wyczyszczenie okna przy u�yciu ustawionego koloru
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	GLfloat vRed[] = { 1.0f, 0.0f, 0.0f, 1.0f };
	shaderManager.UseStockShader(GLT_SHADER_IDENTITY, vRed);
	squareBatch.Draw();

	// Wykonanie instrukcji rysowania
	glutSwapBuffers();

	BounceFunction();
	glutPostRedisplay(); // Ponowne rysowanie
	}



///////////////////////////////////////////////////////////////////////////////
// Zmieni� si� rozmiar okna lub okno zosta�o w�a�nie utworzone. W obu przypadkach musimy
// sprawdzi� wymiary tego okna, aby ustawi� obszar widoku i utworzy� macierz rzutowania.
void ChangeSize(int w, int h)
	{
	glViewport(0, 0, w, h);
	}

///////////////////////////////////////////////////////////////////////////////
// G��wny punkt pocz�tkowy programu opartego na bibliotece GLUT
int main(int argc, char* argv[])
	{
	gltSetWorkingDirectory(argv[0]);
	
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Odbijaj�cy si� blok");
	
	GLenum err = glewInit();
	if (GLEW_OK != err)
		{
		// Problem: awaria glewInit, jest bardzo �le.
		fprintf(stderr, "B��d: %s\n", glewGetErrorString(err));
		return 1;
		}
	
	glutReshapeFunc(ChangeSize);
	glutDisplayFunc(RenderScene);

	SetupRC();

	glutMainLoop();
	return 0;
	}
