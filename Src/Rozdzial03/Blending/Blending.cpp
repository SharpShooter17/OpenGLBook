// Blending.cpp
// Przesuwa blok za pomoc� klawiszy strza�ek,
// i miesza jego kolor z kolorami t�a.

#include <GLTools.h>	// Biblioteka OpenGL
#include <GLShaderManager.h>

#ifdef __APPLE__
#include <glut/glut.h>
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>
#endif

GLBatch	squareBatch;
GLBatch greenBatch;
GLBatch redBatch;
GLBatch blueBatch;
GLBatch blackBatch;

GLShaderManager	shaderManager;


GLfloat blockSize = 0.2f;
GLfloat vVerts[] = { -blockSize, -blockSize, 0.0f, 
	                  blockSize, -blockSize, 0.0f,
					  blockSize,  blockSize, 0.0f,
					 -blockSize,  blockSize, 0.0f};

///////////////////////////////////////////////////////////////////////////////
// Ta funkcja wykonuje wszystkie dzia�ania zwi�zane z inicjalizowaniem w kontek�cie renderowania. 
// Jest to pierwsza okazja do wykonania jakich� zada� zwi�zanych z OpenGL.
void SetupRC()
	{
	// Czarne t�o
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f );
    
	shaderManager.InitializeStockShaders();

	// Za�adowanie wachlarza tr�jk�t�w
	squareBatch.Begin(GL_TRIANGLE_FAN, 4);
	squareBatch.CopyVertexData3f(vVerts);
	squareBatch.End();

    GLfloat vBlock[] = { 0.25f, 0.25f, 0.0f,
                         0.75f, 0.25f, 0.0f,
                         0.75f, 0.75f, 0.0f,
                         0.25f, 0.75f, 0.0f};
    
    greenBatch.Begin(GL_TRIANGLE_FAN, 4);
    greenBatch.CopyVertexData3f(vBlock);
    greenBatch.End();
    

    GLfloat vBlock2[] = { -0.75f, 0.25f, 0.0f,
                          -0.25f, 0.25f, 0.0f,
                          -0.25f, 0.75f, 0.0f,
                          -0.75f, 0.75f, 0.0f};
        
    redBatch.Begin(GL_TRIANGLE_FAN, 4);
    redBatch.CopyVertexData3f(vBlock2);
    redBatch.End();
    

    GLfloat vBlock3[] = { -0.75f, -0.75f, 0.0f,
                        -0.25f, -0.75f, 0.0f,
                        -0.25f, -0.25f, 0.0f,
                        -0.75f, -0.25f, 0.0f};
        
    blueBatch.Begin(GL_TRIANGLE_FAN, 4);
    blueBatch.CopyVertexData3f(vBlock3);
    blueBatch.End();


    GLfloat vBlock4[] = { 0.25f, -0.75f, 0.0f,
                        0.75f, -0.75f, 0.0f,
                        0.75f, -0.25f, 0.0f,
                        0.25f, -0.25f, 0.0f};
        
    blackBatch.Begin(GL_TRIANGLE_FAN, 4);
    blackBatch.CopyVertexData3f(vBlock4);
    blackBatch.End();
	}

// Obs�uga klawiszy strza�ek polegaj�ca na poruszaniu uk�adu odniesienia kamery
void SpecialKeys(int key, int x, int y)
    {
	GLfloat stepSize = 0.025f;

	GLfloat blockX = vVerts[0];   // Wsp�rz�dna X lewego g�rnego rogu
	GLfloat blockY = vVerts[7];  // Wsp�rz�dna Y lewego g�rnego rogu

	if(key == GLUT_KEY_UP)
		blockY += stepSize;

	if(key == GLUT_KEY_DOWN)
		blockY -= stepSize;
	
	if(key == GLUT_KEY_LEFT)
		blockX -= stepSize;

	if(key == GLUT_KEY_RIGHT)
		blockX += stepSize;

	// Wykrywanie zetkni�cia z kraw�dzi� okna
	if(blockX < -1.0f) blockX = -1.0f;
	if(blockX > (1.0f - blockSize * 2)) blockX = 1.0f - blockSize * 2;;
	if(blockY < -1.0f + blockSize * 2)  blockY = -1.0f + blockSize * 2;
	if(blockY > 1.0f) blockY = 1.0f;

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

	glutPostRedisplay();
	}





///////////////////////////////////////////////////////////////////////////////
// Rysowanie sceny
void RenderScene(void)
	{
	// Wyczyszczenie okna przy u�yciu ustawionego koloru
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	GLfloat vRed[] = { 1.0f, 0.0f, 0.0f, 0.5f };
    GLfloat vGreen[] = { 0.0f, 1.0f, 0.0f, 1.0f };
    GLfloat vBlue[] = { 0.0f, 0.0f, 1.0f, 1.0f };
    GLfloat vBlack[] = { 0.0f, 0.0f, 0.0f, 1.0f };


    shaderManager.UseStockShader(GLT_SHADER_IDENTITY, vGreen);
    greenBatch.Draw();

    shaderManager.UseStockShader(GLT_SHADER_IDENTITY, vRed);
    redBatch.Draw();

    shaderManager.UseStockShader(GLT_SHADER_IDENTITY, vBlue);
    blueBatch.Draw();

    shaderManager.UseStockShader(GLT_SHADER_IDENTITY, vBlack);
    blackBatch.Draw();



    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    shaderManager.UseStockShader(GLT_SHADER_IDENTITY, vRed);
    squareBatch.Draw();
    glDisable(GL_BLEND);
    

	// Wykonanie instrukcji rysowania
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
	glutInitWindowSize(800, 600);
	glutCreateWindow("U�yj klawiszy strza�ek, aby porusza� blokiem i zaobserwowa� efekt mieszania kolor�w");
	
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
