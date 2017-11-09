// ModelviewProjection.cpp
// OpenGL. Ksiêga eksperta
// Demonstruje macierz rzutowania model-widok biblioteki OpenGL
// Autor programu: Richard S. Wright Jr.
#include <GLTools.h>	// Biblioteka OpenGL
#include <GLMatrixStack.h>
#include <GLFrame.h>
#include <GLFrustum.h>
#include <GLGeometryTransform.h>
#include <GLBatch.h>
#include <StopWatch.h>

#include <math.h>
#ifdef __APPLE__
#include <glut/glut.h>
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>
#endif


// Egzemplarz klasy GLFrustum
GLFrustum           viewFrustum;

// Mened¿er shaderów
GLShaderManager     shaderManager;

// Torus
GLTriangleBatch     torusBatch;


// Ustawianie widoku i macierzy rzutowania
void ChangeSize(int w, int h)
    {
	// Ochrona przed dzieleniem przez zero
	if(h == 0)
		h = 1;
    
	// Ustawienie widoku na wymiary okna
    glViewport(0, 0, w, h);
    
    viewFrustum.SetPerspective(35.0f, float(w)/float(h), 1.0f, 1000.0f);
    }


// Rysowanie sceny
void RenderScene(void)
	{
    // Animacja czasowa
    static CStopWatch rotTimer;
    float yRot = rotTimer.GetElapsedSeconds() * 20.0f;
    
	// Czyszczenie okna i bufora g³êbi
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Zmienne macierzowe
    M3DMatrix44f mTranslate, mRotate, mModelview, mModelViewProjection;
    
    // Tworzenie macierzy przesuniêcia przesuwaj¹cej torus do ty³u i w kierunku widoku
    m3dTranslationMatrix44(mTranslate, 0.0f, 0.0f, -2.5f);
    
    // Tworzenie macierzy obrotu na podstawie bie¿¹cej wartoœci zmiennej yRot
    m3dRotationMatrix44(mRotate, m3dDegToRad(yRot), 0.0f, 1.0f, 0.0f);
    
    //Dodanie obrotu do przesuniêcia i zapisanie wyniku w zmiennej mModelView
    m3dMatrixMultiply44(mModelview, mTranslate, mRotate);
    
    //Dodanie macierzy model-widok do macierzy rzutowania
    // Ostatecznie powstaje macierz rzutowania model-widok
    m3dMatrixMultiply44(mModelViewProjection, viewFrustum.GetProjectionMatrix(),mModelview);
		
    // Przekazanie utworzonej macierzy do shadera i renderowanie torusa
    GLfloat vBlack[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    shaderManager.UseStockShader(GLT_SHADER_FLAT, mModelViewProjection, vBlack);  
    torusBatch.Draw();


    // Zamiana buforów i natychmiastowe odœwie¿enie
    glutSwapBuffers();
    glutPostRedisplay();
	}

// Inicjalizowanie w kontekœcie renderowania 
void SetupRC()
	{
	// czarne t³o
	glClearColor(0.8f, 0.8f, 0.8f, 1.0f );

    glEnable(GL_DEPTH_TEST);

    shaderManager.InitializeStockShaders();
  
    // Tworzenie torusa
    gltMakeTorus(torusBatch, 0.4f, 0.15f, 30, 30);


    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}


///////////////////////////////////////////////////////////////////////////////
// Punkt rozpoczêcia dzia³ania programu
int main(int argc, char* argv[])
    {
	gltSetWorkingDirectory(argv[0]);
	
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Macierz rzutowania model-widok");
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
