// SphereWorld.cpp
// OpenGL. Ksiêga eksperta
// Nowa i ulepszona wersja programu SphereWorld
// Autor programu: Richard S. Wright Jr.

#include <GLTools.h>
#include <GLShaderManager.h>
#include <GLFrustum.h>
#include <GLBatch.h>
#include <GLFrame.h>
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

#define NUM_SPHERES 50
GLFrame spheres[NUM_SPHERES];


GLShaderManager		shaderManager;			// Mened¿er shaderów
GLMatrixStack		modelViewMatrix;		// Macierz model-widok
GLMatrixStack		projectionMatrix;		// Macierz rzutowania
GLFrustum			viewFrustum;			// Frusta widoku
GLGeometryTransform	transformPipeline;		// Potok przekszta³cania geometrii

GLTriangleBatch		torusBatch;
GLBatch				floorBatch;
GLTriangleBatch     sphereBatch;
GLFrame             cameraFrame;
        
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
	
    // Tworzenie kuli
    gltMakeSphere(sphereBatch, 0.1f, 26, 13);
    	
	floorBatch.Begin(GL_LINES, 324);
    for(GLfloat x = -20.0; x <= 20.0f; x+= 0.5) {
        floorBatch.Vertex3f(x, -0.55f, 20.0f);
        floorBatch.Vertex3f(x, -0.55f, -20.0f);
        
        floorBatch.Vertex3f(20.0f, -0.55f, x);
        floorBatch.Vertex3f(-20.0f, -0.55f, x);
        }
    floorBatch.End();    

    // Losowe rozmieszczenie kul
    for(int i = 0; i < NUM_SPHERES; i++) {
        GLfloat x = ((GLfloat)((rand() % 400) - 200) * 0.1f);
        GLfloat z = ((GLfloat)((rand() % 400) - 200) * 0.1f);
        spheres[i].SetOrigin(x, 0.0f, z);
        }
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
    static GLfloat vSphereColor[] = { 0.0f, 0.0f, 1.0f, 1.0f };

    // Animacja czasowa
	static CStopWatch	rotTimer;
	float yRot = rotTimer.GetElapsedSeconds() * 60.0f;
	
	// Czyszczenie buforów kolorów i g³êbi
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
    
    // Zapisanie bie¿¹cej macierzy model-widok (macierz jednostkowa)
	modelViewMatrix.PushMatrix();	
    
    M3DMatrix44f mCamera;
    cameraFrame.GetCameraMatrix(mCamera);
    modelViewMatrix.PushMatrix(mCamera);
		
	// Rysowanie pod³o¿a
	shaderManager.UseStockShader(GLT_SHADER_FLAT,
								 transformPipeline.GetModelViewProjectionMatrix(),
								 vFloorColor);	
	floorBatch.Draw();    
    
    for(int i = 0; i < NUM_SPHERES; i++) {
        modelViewMatrix.PushMatrix();
        modelViewMatrix.MultMatrix(spheres[i]);
        shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(),
                                vSphereColor);
        sphereBatch.Draw();
        modelViewMatrix.PopMatrix();
        }

    // Rysowanie obracaj¹cego siê torusa
    modelViewMatrix.Translate(0.0f, 0.0f, -2.5f);
    
    // Zapisanie przesuniêcia
    modelViewMatrix.PushMatrix();
    
        // Zastosowanie obrotu i narysowanie torusa
        modelViewMatrix.Rotate(yRot, 0.0f, 1.0f, 0.0f);
        shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(),
                                vTorusColor);
        torusBatch.Draw();
    modelViewMatrix.PopMatrix(); // Wyczyszczenie poprzedniego obrotu

    // Zastosowanie kolejnego obrotu i przesuniêcia, a nastêpnie narysowanie kuli
    modelViewMatrix.Rotate(yRot * -2.0f, 0.0f, 1.0f, 0.0f);
    modelViewMatrix.Translate(0.8f, 0.0f, 0.0f);
    shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(),
                                     vSphereColor);
    sphereBatch.Draw();

	// Przywrócenie poprzedniej macierzy model-widok (macierz jednostkowa)
	modelViewMatrix.PopMatrix();
    modelViewMatrix.PopMatrix();    
    // Zamiana buforów
    glutSwapBuffers();
        
    // Ponowne wyœwietlenie za pomoc¹ funkcji z biblioteki GLUT
    glutPostRedisplay();
    }


// Poruszanie uk³adem odniesienia kamery w odpowiedzi na naciœniêcia klawiszy strza³ek
void SpecialKeys(int key, int x, int y)
    {
	float linear = 0.1f;
	float angular = float(m3dDegToRad(5.0f));
	
	if(key == GLUT_KEY_UP)
		cameraFrame.MoveForward(linear);
	
	if(key == GLUT_KEY_DOWN)
		cameraFrame.MoveForward(-linear);
	
	if(key == GLUT_KEY_LEFT)
		cameraFrame.RotateWorld(angular, 0.0f, 1.0f, 0.0f);
	
	if(key == GLUT_KEY_RIGHT)
		cameraFrame.RotateWorld(-angular, 0.0f, 1.0f, 0.0f);		
    }

int main(int argc, char* argv[])
    {
	gltSetWorkingDirectory(argv[0]);
		
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800,600);
  
    glutCreateWindow("SphereWorld OpenGL");
 
    glutSpecialFunc(SpecialKeys);
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
