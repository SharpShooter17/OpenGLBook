// Bounce.cpp
// Odbijanie bloku w obrêbie okna

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
// Ta funkcja wykonuje wszystkie dzia³ania zwi¹zane z inicjalizowaniem w kontekœcie renderowania. 
// Jest to pierwsza okazja do wykonania jakichœ zadañ zwi¹zanych z OpenGL.
void SetupRC()
	{
	// Czarne t³o
	glClearColor(0.0f, 0.0f, 1.0f, 1.0f );
    
	shaderManager.InitializeStockShaders();

	// £adowanie trójk¹ta
	squareBatch.Begin(GL_TRIANGLE_FAN, 4);
	squareBatch.CopyVertexData3f(vVerts);
	squareBatch.End();
	}

// Odpowiadanie na naciœniêcia klawiszy strza³ek poprzez poruszanie uk³adu odniesienia kamery
void BounceFunction(void)
    {
	static GLfloat xDir = 1.0f;
	static GLfloat yDir = 1.0f;

	GLfloat stepSize = 0.005f;

	GLfloat blockX = vVerts[0];   // Wspó³rzêdna X lewego górnego rogu
	GLfloat blockY = vVerts[7];  // Wspó³rzêdna Y lewego górnego rogu

	blockY += stepSize * yDir;
	blockX += stepSize * xDir;

	// Wykrywanie zetkniêcia z krawêdzi¹ okna
	if(blockX < -1.0f) { blockX = -1.0f; xDir *= -1.0f; }
	if(blockX > (1.0f - blockSize * 2)) { blockX = 1.0f - blockSize * 2; xDir *= -1.0f; }
	if(blockY < -1.0f + blockSize * 2)  { blockY = -1.0f + blockSize * 2; yDir *= -1.0f; }
	if(blockY > 1.0f) { blockY = 1.0f; yDir *= -1.0f; }

	// Ponowne obliczenie po³o¿enia wierzcho³ków
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
	// Wyczyszczenie okna przy u¿yciu ustawionego koloru
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
// Zmieni³ siê rozmiar okna lub okno zosta³o w³aœnie utworzone. W obu przypadkach musimy
// sprawdziæ wymiary tego okna, aby ustawiæ obszar widoku i utworzyæ macierz rzutowania.
void ChangeSize(int w, int h)
	{
	glViewport(0, 0, w, h);
	}

///////////////////////////////////////////////////////////////////////////////
// G³ówny punkt pocz¹tkowy programu opartego na bibliotece GLUT
int main(int argc, char* argv[])
	{
	gltSetWorkingDirectory(argv[0]);
	
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Odbijaj¹cy siê blok");
	
	GLenum err = glewInit();
	if (GLEW_OK != err)
		{
		// Problem: awaria glewInit, jest bardzo Ÿle.
		fprintf(stderr, "B³¹d: %s\n", glewGetErrorString(err));
		return 1;
		}
	
	glutReshapeFunc(ChangeSize);
	glutDisplayFunc(RenderScene);

	SetupRC();

	glutMainLoop();
	return 0;
	}
