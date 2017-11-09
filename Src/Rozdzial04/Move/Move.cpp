// Move.cpp
// Przesuwa blok w odpowiedzi na naciskanie klawiszy strza³ek

#include <GLTools.h>	// Biblioteka OpenGL
#include <GLShaderManager.h>
#include <math3d.h>

#ifdef __APPLE__
#include <glut/glut.h>
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>
#endif

GLBatch	squareBatch;
GLShaderManager	shaderManager;


GLfloat blockSize = 0.1f;
GLfloat vVerts[] = { -blockSize, -blockSize, 0.0f, 
	                  blockSize, -blockSize, 0.0f,
					  blockSize,  blockSize, 0.0f,
					 -blockSize,  blockSize, 0.0f};

GLfloat xPos = 0.0f;
GLfloat yPos = 0.0f;


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
void SpecialKeys(int key, int x, int y)
    {
	GLfloat stepSize = 0.025f;


	if(key == GLUT_KEY_UP)
		yPos += stepSize;

	if(key == GLUT_KEY_DOWN)
		yPos -= stepSize;
	
	if(key == GLUT_KEY_LEFT)
		xPos -= stepSize;

	if(key == GLUT_KEY_RIGHT)
		xPos += stepSize;

	// Wykrywanie zetkniêcia z krawêdzi¹ okna
	if(xPos < (-1.0f + blockSize)) xPos = -1.0f + blockSize;
    
	if(xPos > (1.0f - blockSize)) xPos = 1.0f - blockSize;
	
    if(yPos < (-1.0f + blockSize))  yPos = -1.0f + blockSize;
    
	if(yPos > (1.0f - blockSize)) yPos = 1.0f - blockSize;

	glutPostRedisplay();
	}





///////////////////////////////////////////////////////////////////////////////
// Rysowanie sceny
void RenderScene(void)
	{
	// Wyczyszczenie okna przy u¿yciu ustawionego koloru
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	GLfloat vRed[] = { 1.0f, 0.0f, 0.0f, 1.0f };
    
    M3DMatrix44f mFinalTransform, mTranslationMatrix, mRotationMatrix;
    
    // Przesuniêcie
    m3dTranslationMatrix44(mTranslationMatrix, xPos, yPos, 0.0f);
    
    // Obrót o piêæ stopni przy ka¿dym rysowaniu
    static float yRot = 0.0f;
    yRot += 5.0f;
    m3dRotationMatrix44(mRotationMatrix, m3dDegToRad(yRot), 0.0f, 0.0f, 1.0f);
    
    m3dMatrixMultiply44(mFinalTransform, mTranslationMatrix, mRotationMatrix);
    
    
	shaderManager.UseStockShader(GLT_SHADER_FLAT, mFinalTransform, vRed);
	squareBatch.Draw();

	// Zamiana buforów
	glutSwapBuffers();
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
	glutInitWindowSize(600, 600);
	glutCreateWindow("Poruszanie blokiem za pomoc¹ klawiszy strza³ek");
	
	GLenum err = glewInit();
	if (GLEW_OK != err)
		{
		// Problem: awaria glewInit, jest bardzo Ÿle.
		fprintf(stderr, "B³¹d: %s\n", glewGetErrorString(err));
		return 1;
		}
	
	glutReshapeFunc(ChangeSize);
	glutDisplayFunc(RenderScene);
    glutSpecialFunc(SpecialKeys);

	SetupRC();

	glutMainLoop();
	return 0;
	}
