// TextureArrays.cpp
// OpenGL SuperBible
// Demonstruje przekazywanie tablic teksturowych do shadera
// Autor programu: Richard S. Wright Jr.
#include <GLTools.h>	// OpenGL
#include <GLFrustum.h>
#include <Stopwatch.h>

#ifdef __APPLE__
#include <glut/glut.h>
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>
#endif

GLShaderManager shaderManager;
GLFrustum viewFrustum;
GLBatch smallStarBatch;
GLBatch mediumStarBatch;
GLBatch largeStarBatch;
GLBatch mountainRangeBatch;
GLBatch moonBatch;

GLuint  starTexture;
GLuint	starFieldShader;	// Shader sprajtów
GLint	locMVP;				// Lokalizacja zmiennej uniform przechowuj¹cej macierz rzutowania model-widok
GLint	locStarTexture;		// Lokalizacja zmiennej jednorodnej tekstury


GLuint  moonTexture;
GLuint  moonShader;
GLint   locMoonMVP;
GLint   locMoonTexture;
GLint	locMoonTime;

GLint   locTimeStamp;       // Lokalizacja znacznika czasu


// Tablica niewielkich gwiazd
#define SMALL_STARS     100
#define MEDIUM_STARS     40
#define LARGE_STARS      15

#define SCREEN_X        800
#define SCREEN_Y        600



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




///////////////////////////////////////////////////
// Rysuje scenê
void RenderScene(void)
    {		        
	static CStopWatch timer;

    // Wyczyszczenie okna
    glClear(GL_COLOR_BUFFER_BIT);
         
    // Wszystko jest bia³e
    GLfloat vWhite [] = { 1.0f, 1.0f, 1.0f, 1.0f };

	glBindTexture(GL_TEXTURE_2D, starTexture);
	glUseProgram(starFieldShader);
    glUniformMatrix4fv(locMVP, 1, GL_FALSE, viewFrustum.GetProjectionMatrix());
    glUniform1i(locStarTexture, 0);
    
	// Rysowanie ma³ych gwiazd
    glPointSize(4.0f);
    smallStarBatch.Draw();
            
    // Rysowanie œrednich gwiazd
    glPointSize(8.0f);
    mediumStarBatch.Draw();
    
    // Rysowanie du¿ych gwiazd
    glPointSize(12.0f);
    largeStarBatch.Draw();
        
    // Draw distant horizon
	shaderManager.UseStockShader(GLT_SHADER_FLAT, viewFrustum.GetProjectionMatrix(), vWhite);
    glLineWidth(3.5);
    mountainRangeBatch.Draw();
    
	// Rysowanie ksi¹¿yca
	glBindTexture(GL_TEXTURE_2D_ARRAY, moonTexture);
	glUseProgram(moonShader);
    glUniformMatrix4fv(locMoonMVP, 1, GL_FALSE, viewFrustum.GetProjectionMatrix());
    glUniform1i(locMoonTexture, 0);

	// Zmienna fTime zmienia cyklicznie wartoœci w zakresie od 0.0 do 28.0f
	float fTime = timer.GetElapsedSeconds();
	fTime = fmod(fTime, 28.0f);
    glUniform1f(locTimeStamp, fTime);

    moonBatch.Draw();


    // Zamiana buforów
    glutSwapBuffers();

	glutPostRedisplay();
    }


// Funkcja inicjalizuj¹ca kontekst renderowania

void SetupRC()
    {
    M3DVector3f vVerts[SMALL_STARS];      // SMALL_STARS to najwiêkszy zbiór, jakiego bêdziemy potrzebowaæ
    int i;
    
    glEnable(GL_POINT_SPRITE);
        
    shaderManager.InitializeStockShaders();
        
	// Niektóre standardowe sterowniki s¹ niezgodne z aktualn¹ wersj¹ biblioteki OpenGL
	// i wymagaj¹ tego (zw³aszcza karty firmy NVIDIA). W specyfikacji OpenGL jest zapis,
	// ¿e to zawsze powinno byæ w³¹czone, w istocie nie mo¿na ju¿ tego ani w³¹czyæ ani wy³¹czyæ.
	// Dodanie tych wierszy kodu eliminuje problemy z niezgodnymi sterownikami, ale pamiêtaj,
	// jeœli masz czysty i poprawnie dzia³aj¹cy kontekst GL,
	// to nie nale¿y tego robiæ.
	glEnable(GL_POINT_SPRITE);

    // Zape³nienie listy gwiazd
    smallStarBatch.Begin(GL_POINTS, SMALL_STARS);
    for(i = 0; i < SMALL_STARS; i++)
        {
        vVerts[i][0] = (GLfloat)(rand() % SCREEN_X);
        vVerts[i][1] = (GLfloat)(rand() % (SCREEN_Y - 100)) + 100.0f;
        vVerts[i][2] = 0.0f;
        }
    smallStarBatch.CopyVertexData3f(vVerts);
    smallStarBatch.End();
            
    // Zape³nienie listy gwiazd
    mediumStarBatch.Begin(GL_POINTS, MEDIUM_STARS);
    for(i = 0; i < MEDIUM_STARS; i++)
        {
        vVerts[i][0] = (GLfloat)(rand() % SCREEN_X);
        vVerts[i][1] = (GLfloat)(rand() % (SCREEN_Y - 100)) + 100.0f;
        vVerts[i][2] = 0.0f; 
        }
    mediumStarBatch.CopyVertexData3f(vVerts);
    mediumStarBatch.End();

    // Zape³nienie listy gwiazd
    largeStarBatch.Begin(GL_POINTS, LARGE_STARS);
    for(i = 0; i < LARGE_STARS; i++)
        {
        vVerts[i][0] = (GLfloat)(rand() % SCREEN_X);
        vVerts[i][1] = (GLfloat)(rand() % (SCREEN_Y - 100)) + 100.0f;
        vVerts[i][2] = 0.0f;
        }
    largeStarBatch.CopyVertexData3f(vVerts);
    largeStarBatch.End();
            
    M3DVector3f vMountains[12] = { 0.0f, 25.0f, 0.0f, 
                                 50.0f, 100.0f, 0.0f,
                                 100.0f, 25.0f, 0.0f,
                                225.0f, 125.0f, 0.0f,
                                300.0f, 50.0f, 0.0f,
                                375.0f, 100.0f, 0.0f,
                                460.0f, 25.0f, 0.0f,
                                525.0f, 100.0f, 0.0f,
                                600.0f, 20.0f, 0.0f,
                                675.0f, 70.0f, 0.0f,
                                750.0f, 25.0f, 0.0f,
                                800.0f, 90.0f, 0.0f };    
        
    mountainRangeBatch.Begin(GL_LINE_STRIP, 12);
    mountainRangeBatch.CopyVertexData3f(vMountains);
    mountainRangeBatch.End();
    
    // Ksiê¿yc
    GLfloat x = 700.0f;     // Lokalizacja i promieñ ksiê¿yca
    GLfloat y = 500.0f;
    GLfloat r = 50.0f;
    GLfloat angle = 0.0f;   // Kolejna zmienna pêtlowa
        
    moonBatch.Begin(GL_TRIANGLE_FAN, 4, 1);
		moonBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
		moonBatch.Vertex3f(x - r, y - r, 0.0f);

		moonBatch.MultiTexCoord2f(0, 1.0f, 0.0f);
		moonBatch.Vertex3f(x + r, y - r, 0.0f);

		moonBatch.MultiTexCoord2f(0, 1.0f, 1.0f);
		moonBatch.Vertex3f(x + r, y + r, 0.0f);

		moonBatch.MultiTexCoord2f(0, 0.0f, 1.0f);
		moonBatch.Vertex3f(x - r, y + r, 0.0f);
	moonBatch.End();     
            
    // Czarne t³o
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f );

    // W³¹czenie wyg³adzania liniowego i wydanie polecenia, aby program wykona³ mo¿liwe jak najlepiej swoje zadanie
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    starFieldShader = gltLoadShaderPairWithAttributes("StarField.vp", "StarField.fp", 1, GLT_ATTRIBUTE_VERTEX, "vVertex");

	locMVP = glGetUniformLocation(starFieldShader, "mvpMatrix");
	locStarTexture = glGetUniformLocation(starFieldShader, "starImage");

	moonShader = gltLoadShaderPairWithAttributes("MoonShader.vp", "MoonShader.fp", 2, GLT_ATTRIBUTE_VERTEX, "vVertex",
					GLT_ATTRIBUTE_TEXTURE0, "vTexCoords");
	locMoonMVP = glGetUniformLocation(moonShader, "mvpMatrix");
	locMoonTexture = glGetUniformLocation(moonShader, "moonImage");
    locMoonTime = glGetUniformLocation(moonShader, "fTime");


	glGenTextures(1, &starTexture);
	glBindTexture(GL_TEXTURE_2D, starTexture);
	LoadTGATexture("Star.tga", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE);


	glGenTextures(1, &moonTexture);
	glBindTexture(GL_TEXTURE_2D_ARRAY, moonTexture);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, 64, 64, 30, 0,
					 GL_BGRA, GL_UNSIGNED_BYTE, NULL);

	for(int i = 0; i < 29; i++) {
		char cFile[32];
		sprintf(cFile, "moon%02d.tga", i);

		GLbyte *pBits;
		int nWidth, nHeight, nComponents;
		GLenum eFormat;
		
		// Wczytanie bitów tekstury
		pBits = gltReadTGABits(cFile, &nWidth, &nHeight, &nComponents, &eFormat);
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, nWidth, nHeight, 1, GL_BGRA, GL_UNSIGNED_BYTE, pBits);
			    
		free(pBits);
		}
   

	}


void ChangeSize(int w, int h)
    {
    // Zabezpieczenie przed dzieleniem przez zero
    if(h == 0)
        h = 1;

    // Ustawia widok na wymiary okna
    glViewport(0, 0, w, h);

    // Ustanowienie przestrzeni ograniczaj¹cej (lewa, prawa, dó³, góra, blisko, daleko)
    viewFrustum.SetOrthographic(0.0f, SCREEN_X, 0.0f, SCREEN_Y, -1.0f, 1.0f);
    }

int main(int argc, char* argv[])
	{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Tablice teksturowe");
	
	glutReshapeFunc(ChangeSize);
	glutDisplayFunc(RenderScene);
    
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        fprintf(stderr, "B³¹d GLEW: %s\n", glewGetErrorString(err));
        return 1;
        }
        
	SetupRC();
	glutMainLoop();

	return 0;
	}
