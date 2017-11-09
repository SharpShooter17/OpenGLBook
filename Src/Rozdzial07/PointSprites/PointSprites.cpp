// PointSprites.cpp
// OpenGL SuperBible
// Demonstracja sprajtów punktowych przy u¿yciu gwiazd
// Autor programu: Richard S. Wright Jr.

#include <GLTools.h>	// OpenGL
#include <GLFrustum.h>
#include <StopWatch.h>

#include <math.h>
#include <stdlib.h>

#ifdef __APPLE__
#include <glut/glut.h>
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>
#endif

#define NUM_STARS 10000

GLFrustum           viewFrustum;
GLBatch             starsBatch;

GLuint	starFieldShader;	// Shader sprajtów
GLint	locMVP;				// Lokalizacja zmiennej uniform przechowuj¹cej macierz rzutowania model-widok
GLint   locTimeStamp;       // Lokalizacja znacznika czasu
GLint	locTexture;			// Lokalizacja zmiennej jednorodnej tekstury

GLuint	starTexture;		// Obiekt tekstury gwiazd


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


// Funkcja inicjalizuj¹ca kontekst renderowania

void SetupRC(void)
	{
	// T³o
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f );

	// Niektóre standardowe sterowniki s¹ niezgodne z aktualn¹ wersj¹ biblioteki OpenGL
	// i wymagaj¹ tego (zw³aszcza karty firmy NVIDIA). W specyfikacji OpenGL jest zapis,
	// ¿e to zawsze powinno byæ w³¹czone, w istocie nie mo¿na ju¿ tego ani w³¹czyæ ani wy³¹czyæ.
	// Dodanie tych wierszy kodu eliminuje problemy z niezgodnymi sterownikami, ale pamiêtaj,
	// jeœli masz czysty i poprawnie dzia³aj¹cy kontekst GL,
	// to nie nale¿y tego robiæ.
	glEnable(GL_POINT_SPRITE);

	GLfloat fColors[4][4] = {{ 1.0f, 1.0f, 1.0f, 1.0f}, // Bia³y
                             { 0.67f, 0.68f, 0.82f, 1.0f}, // Niebieskie gwiazdy
                             { 1.0f, 0.5f, 0.5f, 1.0f}, // Czerwony
	                         { 1.0f, 0.82f, 0.65f, 1.0f}}; // Pomarañczowy


    // Losowe pocz¹tkowe rozmieszczenie gwiazd i wybór losowego koloru
    starsBatch.Begin(GL_POINTS, NUM_STARS);
    for(int i = 0; i < NUM_STARS; i++)
        {
		int iColor = 0;		// Wszystkie gwiazdy s¹ pocz¹tkowo bia³e

		// Co pi¹ta bêdzie niebieska
        if(rand() % 5 == 1)
			iColor = 1;

		// Co 50 bêdzie czerwona
		if(rand() % 50 == 1)
			iColor = 2;

		// Co 100 bêdzie zielona
		if(rand() % 100 == 1)
			iColor = 3;

		
		starsBatch.Color4fv(fColors[iColor]);
			    
		M3DVector3f vPosition;
		vPosition[0] = float(3000 - (rand() % 6000)) * 0.1f;
		vPosition[1] = float(3000 - (rand() % 6000)) * 0.1f;
		vPosition[2] = -float(rand() % 1000)-1.0f;  // -1 do -1000.0f

		starsBatch.Vertex3fv(vPosition);
		}
    starsBatch.End();


    starFieldShader = gltLoadShaderPairWithAttributes("SpaceFlight.vp", "SpaceFlight.fp", 2, GLT_ATTRIBUTE_VERTEX, "vVertex",
			GLT_ATTRIBUTE_COLOR, "vColor");

	locMVP = glGetUniformLocation(starFieldShader, "mvpMatrix");
	locTexture = glGetUniformLocation(starFieldShader, "starImage");
    locTimeStamp = glGetUniformLocation(starFieldShader, "timeStamp");
    
	glGenTextures(1, &starTexture);
	glBindTexture(GL_TEXTURE_2D, starTexture);
	LoadTGATexture("Star.tga", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE);
    }

// Porz¹dkowanie
void ShutdownRC(void)
{
    glDeleteTextures(1, &starTexture);
}


// Rysuje scenê
void RenderScene(void)
	{
	static CStopWatch timer;

	// Czyœci okno i bufor g³êbi
	glClear(GL_COLOR_BUFFER_BIT);
		
    // W³¹cza mieszanie addytywne
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

    // Okreœlenie przez shader wierzcho³ków rozmiaru punktów
	glEnable(GL_PROGRAM_POINT_SIZE);

    // Powi¹zanie z shaderem, ustawienie zmiennych uniform
    glUseProgram(starFieldShader);
    glUniformMatrix4fv(locMVP, 1, GL_FALSE, viewFrustum.GetProjectionMatrix());
    glUniform1i(locTexture, 0);

	// Zmienna fTime zmienia cyklicznie wartoœci w zakresie od 0.0 do 999.0
	float fTime = timer.GetElapsedSeconds() * 10.0f;
	fTime = fmod(fTime, 999.0f);
    glUniform1f(locTimeStamp, fTime);

    // Rysowanie gwiazd
    starsBatch.Draw();
    
    glutSwapBuffers();
	glutPostRedisplay();
	}



void ChangeSize(int w, int h)
	{
	// Zabezpieczenie przed dzieleniem przez zero
	if(h == 0)
		h = 1;

	// Ustawia widok na wymiary okna
    glViewport(0, 0, w, h);

    viewFrustum.SetPerspective(35.0f, float(w)/float(h), 1.0f, 1000.0f);
	}

///////////////////////////////////////////////////////////////////////////////
// G³ówny punkt wejœciowy wszystkich programów opartych na bibliotece GLUT
int main(int argc, char* argv[])
    {
	gltSetWorkingDirectory(argv[0]);
	
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Spaced Out");
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
