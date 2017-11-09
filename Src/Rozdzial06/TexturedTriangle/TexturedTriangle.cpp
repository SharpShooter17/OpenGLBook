// TexturedTriangle.cpp
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

GLint	myTexturedIdentityShader;
GLuint	textureID;

///////////////////////////////////////////////////////////////////////////////
// Zmieni� si� rozmiar okna lub okno zosta�o w�a�nie utworzone. W obu przypadkach musimy
// sprawdzi� wymiary tego okna, aby ustawi� obszar widoku i utworzy� macierz rzutowania.
void ChangeSize(int w, int h)
    {
	glViewport(0, 0, w, h);
    }


// Wczytanie pliku TGA jako tekstury dwuwymiarowej. Pe�ne zainicjalizowanie stanu
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

	GLfloat vTexCoords [] = { 0.0f, 0.0f,
		                      1.0f, 0.0f, 
						      0.5f, 1.0f };

	triangleBatch.Begin(GL_TRIANGLES, 3, 1);
	triangleBatch.CopyVertexData3f(vVerts);
	triangleBatch.CopyTexCoordData2f(vTexCoords, 0);
	triangleBatch.End();

	myTexturedIdentityShader = gltLoadShaderPairWithAttributes("TexturedIdentity.vp", "TexturedIdentity.fp", 2, 
		                            GLT_ATTRIBUTE_VERTEX, "vVertex", GLT_ATTRIBUTE_TEXTURE0, "vTexCoords");

	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	LoadTGATexture("stone.tga", GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE);
	}


///////////////////////////////////////////////////////////////////////////////
// Czyszczenie
void ShutdownRC()
   {
   glDeleteProgram(myTexturedIdentityShader);
   glDeleteTextures(1, &textureID);
   }


///////////////////////////////////////////////////////////////////////////////
// Rysowanie sceny
void RenderScene(void)
	{
	// Wyczyszczenie okna bie��cym kolorem
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	glUseProgram(myTexturedIdentityShader);
    glBindTexture(GL_TEXTURE_2D, textureID);
    GLint iTextureUniform = glGetUniformLocation(myTexturedIdentityShader, "colorMap");
	glUniform1i(iTextureUniform, 0);

	triangleBatch.Draw();

	// Zamiana bufor�w w celu wy�wietlenia zawarto�ci bufora tylnego
	glutSwapBuffers();
	}


///////////////////////////////////////////////////////////////////////////////
// Punkt rozpocz�cia wykonywania ka�dego programu opartego na bibliotece GLUT
int main(int argc, char* argv[])
	{
	gltSetWorkingDirectory(argv[0]);
	
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Tr�jk�t z na�o�on� tekstur�");
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
