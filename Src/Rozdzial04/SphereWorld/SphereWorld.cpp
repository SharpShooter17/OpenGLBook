// SphereWorld.cpp
// OpenGL. Ksiêga eksperta
// Nowy i poprawiony program SphereWorld
// Autor programu: Richard S. Wright Jr.

#include <GLTools.h>
#include <GLShaderManager.h>
#include <GLFrustum.h>
#include <GLBatch.h>
#include <GLMatrixStack.h>
#include <GLGeometryTransform.h>
#include <StopWatch.h>

#include <math.h>
#include <stdio.h>

#ifdef __APPLE__
#include <glut/glut.h>
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>
#endif




GLShaderManager		shaderManager;			// Mened¿er shaderów
GLMatrixStack		modelViewMatrix;		// Macierz model-widok
GLMatrixStack		projectionMatrix;		// Macierz rzutowania
GLFrustum			viewFrustum;			// Frusta widoku
GLGeometryTransform	transformPipeline;		// Potok przekszta³cania geometrii

GLTriangleBatch		torusBatch;
GLBatch				floorBatch;

        
//////////////////////////////////////////////////////////////////
// Ta funkcja wykonuje wszystkie dzia³ania zwi¹zane z inicjalizowaniem w kontekœcie renderowania.
void SetupRC()
    {
	// Inicjalizacja mened¿era shaderów
	shaderManager.InitializeStockShaders();
	
	glEnable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	
	// Tworzenie torusa
	gltMakeTorus(torusBatch, 0.4f, 0.15f, 30, 30);
	
    	
	floorBatch.Begin(GL_LINES, 324);
    for(GLfloat x = -20.0; x <= 20.0f; x+= 0.5) {
        floorBatch.Vertex3f(x, -0.55f, 20.0f);
        floorBatch.Vertex3f(x, -0.55f, -20.0f);
        
        floorBatch.Vertex3f(20.0f, -0.55f, x);
        floorBatch.Vertex3f(-20.0f, -0.55f, x);
        }
    floorBatch.End();    
    }


///////////////////////////////////////////////////
// Zmieni³ siê rozmiar okna lub okno zosta³o w³aœnie zainicjalizowane.
void ChangeSize(int nWidth, int nHeight)
    {
	glViewport(0, 0, nWidth, nHeight);
	
    // Tworzenie macierzy rzutowania i wstawienie jej na stos macierzy rzutowania.
	viewFrustum.SetPerspective(35.0f, float(nWidth)/float(nHeight), 1.0f, 100.0f);
	projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
    
    // Ustawienie dwóch stosów macierzy w potoku przekszta³cania.
	transformPipeline.SetMatrixStacks(modelViewMatrix, projectionMatrix);
    }

        
// Rysowanie sceny
void RenderScene(void)
	{
    // Wartoœci kolorów
    static GLfloat vFloorColor[] = { 0.0f, 1.0f, 0.0f, 1.0f};
    static GLfloat vTorusColor[] = { 1.0f, 0.0f, 0.0f, 1.0f };

    // Animacja czasowa
	static CStopWatch	rotTimer;
	float yRot = rotTimer.GetElapsedSeconds() * 60.0f;
	
	// Czyszczenie bufora kolorów i g³êbi
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
    
    // Zapisanie bie¿¹cej macierzy model-widok (macierz jednostkowa)
	modelViewMatrix.PushMatrix();	
		
	// Rysowanie t³a
	shaderManager.UseStockShader(GLT_SHADER_FLAT,
								 transformPipeline.GetModelViewProjectionMatrix(),
								 vFloorColor);	
	floorBatch.Draw();

    // Rysowanie obracaj¹cego siê torusa
    modelViewMatrix.Translate(0.0f, 0.0f, -2.5f);
    modelViewMatrix.Rotate(yRot, 0.0f, 1.0f, 0.0f);
    shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(),
                                vTorusColor);
    torusBatch.Draw();

	// Przywrócenie poprzedniej macierzy model-widok (macierz jednostkowa)
	modelViewMatrix.PopMatrix();
        
    // Zamiana buforów
    glutSwapBuffers();
        
    // Ponowne wyœwietlenie za pomoc¹ funkcji z biblioteki GLUT
    glutPostRedisplay();
    }


int main(int argc, char* argv[])
    {
	gltSetWorkingDirectory(argv[0]);
		
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800,600);
  
    glutCreateWindow("SphereWorld OpenGL");
 
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
