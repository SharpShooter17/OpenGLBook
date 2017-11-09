// Move.cpp
// Przesuwa blok w odpowiedzi na naciskanie klawiszy strza�ek

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

	// Wykrywanie zetkni�cia z kraw�dzi� okna
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
	// Wyczyszczenie okna przy u�yciu ustawionego koloru
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	GLfloat vRed[] = { 1.0f, 0.0f, 0.0f, 1.0f };
    
    M3DMatrix44f mFinalTransform, mTranslationMatrix, mRotationMatrix;
    
    // Przesuni�cie
    m3dTranslationMatrix44(mTranslationMatrix, xPos, yPos, 0.0f);
    
    // Obr�t o pi�� stopni przy ka�dym rysowaniu
    static float yRot = 0.0f;
    yRot += 5.0f;
    m3dRotationMatrix44(mRotationMatrix, m3dDegToRad(yRot), 0.0f, 0.0f, 1.0f);
    
    m3dMatrixMultiply44(mFinalTransform, mTranslationMatrix, mRotationMatrix);
    
    
	shaderManager.UseStockShader(GLT_SHADER_FLAT, mFinalTransform, vRed);
	squareBatch.Draw();

	// Zamiana bufor�w
	glutSwapBuffers();
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
	glutInitWindowSize(600, 600);
	glutCreateWindow("Poruszanie blokiem za pomoc� klawiszy strza�ek");
	
	GLenum err = glewInit();
	if (GLEW_OK != err)
		{
		// Problem: awaria glewInit, jest bardzo �le.
		fprintf(stderr, "B��d: %s\n", glewGetErrorString(err));
		return 1;
		}
	
	glutReshapeFunc(ChangeSize);
	glutDisplayFunc(RenderScene);
    glutSpecialFunc(SpecialKeys);

	SetupRC();

	glutMainLoop();
	return 0;
	}
