// Scissor.cpp
// OpenGL. Ksiêga eksperta, wydanie pi¹te
// Autor: Richard S. Wright Jr.
// opengl@bellsouth.net

#include <GLTools.h>        // Biblioteka OpenGL

#ifdef __APPLE__
#include <glut/glut.h>
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>
#endif

///////////////////////////////////////////////////////////
// Rysowanie sceny
void RenderScene(void)
	{
        // Czyszczenie okna kolorem niebieskim
        glClearColor(0.0f, 0.0f, 1.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        // Wykrojenie mniejszego, czerwonego obszaru
        glClearColor(1.0f, 0.0f, 0.0f, 0.0f);
        glScissor(100, 100, 600, 400);
        glEnable(GL_SCISSOR_TEST);
        glClear(GL_COLOR_BUFFER_BIT);
        
        // Jeszcze mniejszy, zielony prostok¹t
        glClearColor(0.0f, 1.0f, 0.0f, 0.0f);
        glScissor(200, 200, 400, 200);
        glClear(GL_COLOR_BUFFER_BIT);
        
        // Wy³¹czenie okrawania przed nastêpnym renderowaniem
        glDisable(GL_SCISSOR_TEST);

	glutSwapBuffers();
	}


///////////////////////////////////////////////////////////
// Ustawienie widoku i rzutowania
void ChangeSize(int w, int h)
	{
	// Ochrona przed dzieleniem przez zero
	if(h == 0)
		h = 1;

	// Ustawienie widoku na wymiary okna
    glViewport(0, 0, w, h);
	}


///////////////////////////////////////////////////////////
// Punkt pocz¹tkowy wykonywania programu
int main(int argc, char* argv[])
	{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(800,600);
	glutCreateWindow("OpenGL Scissor");
	glutReshapeFunc(ChangeSize);
	glutDisplayFunc(RenderScene);
	glutMainLoop();

	return 0;
	}
