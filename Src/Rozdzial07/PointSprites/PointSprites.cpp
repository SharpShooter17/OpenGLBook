// PointSprites.cpp
// OpenGL SuperBible
// Demonstracja sprajt�w punktowych przy u�yciu gwiazd
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

GLuint	starFieldShader;	// Shader sprajt�w
GLint	locMVP;				// Lokalizacja zmiennej uniform przechowuj�cej macierz rzutowania model-widok
GLint   locTimeStamp;       // Lokalizacja znacznika czasu
GLint	locTexture;			// Lokalizacja zmiennej jednorodnej tekstury

GLuint	starTexture;		// Obiekt tekstury gwiazd


/// Wczytanie TGA jako tekstury dwuwymiarowej. Pe�na inicjalizacja stanu
bool LoadTGATexture(const char *szFileName, GLenum minFilter, GLenum magFilter, GLenum wrapMode)
{
	GLbyte *pBits;
	int nWidth, nHeight, nComponents;
	GLenum eFormat;
	
	// Wczytanie bit�w tekstury
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


// Funkcja inicjalizuj�ca kontekst renderowania

void SetupRC(void)
	{
	// T�o
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f );

	// Niekt�re standardowe sterowniki s� niezgodne z aktualn� wersj� biblioteki OpenGL
	// i wymagaj� tego (zw�aszcza karty firmy NVIDIA). W specyfikacji OpenGL jest zapis,
	// �e to zawsze powinno by� w��czone, w istocie nie mo�na ju� tego ani w��czy� ani wy��czy�.
	// Dodanie tych wierszy kodu eliminuje problemy z niezgodnymi sterownikami, ale pami�taj,
	// je�li masz czysty i poprawnie dzia�aj�cy kontekst GL,
	// to nie nale�y tego robi�.
	glEnable(GL_POINT_SPRITE);

	GLfloat fColors[4][4] = {{ 1.0f, 1.0f, 1.0f, 1.0f}, // Bia�y
                             { 0.67f, 0.68f, 0.82f, 1.0f}, // Niebieskie gwiazdy
                             { 1.0f, 0.5f, 0.5f, 1.0f}, // Czerwony
	                         { 1.0f, 0.82f, 0.65f, 1.0f}}; // Pomara�czowy


    // Losowe pocz�tkowe rozmieszczenie gwiazd i wyb�r losowego koloru
    starsBatch.Begin(GL_POINTS, NUM_STARS);
    for(int i = 0; i < NUM_STARS; i++)
        {
		int iColor = 0;		// Wszystkie gwiazdy s� pocz�tkowo bia�e

		// Co pi�ta b�dzie niebieska
        if(rand() % 5 == 1)
			iColor = 1;

		// Co 50 b�dzie czerwona
		if(rand() % 50 == 1)
			iColor = 2;

		// Co 100 b�dzie zielona
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

// Porz�dkowanie
void ShutdownRC(void)
{
    glDeleteTextures(1, &starTexture);
}


// Rysuje scen�
void RenderScene(void)
	{
	static CStopWatch timer;

	// Czy�ci okno i bufor g��bi
	glClear(GL_COLOR_BUFFER_BIT);
		
    // W��cza mieszanie addytywne
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

    // Okre�lenie przez shader wierzcho�k�w rozmiaru punkt�w
	glEnable(GL_PROGRAM_POINT_SIZE);

    // Powi�zanie z shaderem, ustawienie zmiennych uniform
    glUseProgram(starFieldShader);
    glUniformMatrix4fv(locMVP, 1, GL_FALSE, viewFrustum.GetProjectionMatrix());
    glUniform1i(locTexture, 0);

	// Zmienna fTime zmienia cyklicznie warto�ci w zakresie od 0.0 do 999.0
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
// G��wny punkt wej�ciowy wszystkich program�w opartych na bibliotece GLUT
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
		fprintf(stderr, "B��d GLEW: %s\n", glewGetErrorString(err));
		return 1;
    }
	
	SetupRC();    
	glutMainLoop();
	ShutdownRC();
	return 0;
    }
