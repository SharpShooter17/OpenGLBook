// GeoTest.cpp
// OpenGL. Ksi�ga eksperta
// Demonstruje technik� eliminowania powierzchni ukrytych
// Autor: Richard S. Wright Jr.
#include <GLTools.h>	// Biblioteka OpenGL
#include <GLMatrixStack.h>
#include <GLFrame.h>
#include <GLFrustum.h>
#include <GLGeometryTransform.h>

#include <math.h>
#ifdef __APPLE__
#include <glut/glut.h>
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>
#endif


GLFrame             viewFrame;
GLFrustum           viewFrustum;
GLTriangleBatch     torusBatch;
GLMatrixStack       modelViewMatix;
GLMatrixStack       projectionMatrix;
GLGeometryTransform transformPipeline;
GLShaderManager     shaderManager;

// Znaczniki efekt�w
int iCull = 0;
int iDepth = 0;

///////////////////////////////////////////////////////////////////////////////
// Resetowanie znacznik�w w odpowiedzi na wybrane opcje z menu
void ProcessMenu(int value)
	{
	switch(value)
		{
		case 1:
			iDepth = !iDepth;
			break;

		case 2:
			iCull = !iCull;
			break;

        case 3:
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            break;

        case 4:
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            break;
                
        case 5:
            glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
            break;
		}

	glutPostRedisplay();
	}


// Rysuje scen�
void RenderScene(void)
	{
	// Czyszczenie okna i bufora g��bi
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// W��cza eliminowanie p�aszczyzn, gdy znacznik jest ustawiony
	if(iCull)
		glEnable(GL_CULL_FACE);
	else
		glDisable(GL_CULL_FACE);

	// W��cza testowanie g��bi, gdy znacznik jest ustawiony
	if(iDepth)
		glEnable(GL_DEPTH_TEST);
	else
		glDisable(GL_DEPTH_TEST);

		
    modelViewMatix.PushMatrix(viewFrame);
            
    GLfloat vRed[] = { 1.0f, 0.0f, 0.0f, 1.0f };
    //shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vRed);
    shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(), transformPipeline.GetProjectionMatrix(), vRed);
    

    torusBatch.Draw();

    modelViewMatix.PopMatrix();


    glutSwapBuffers();
	}

// Ta funkcja wykonuje wszystkie dzia�ania zwi�zane z inicjalizowaniem w kontek�cie renderowania.
void SetupRC()
	{
	// Czarne t�o
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f );

    shaderManager.InitializeStockShaders();
    viewFrame.MoveForward(7.0f);

    // Tworzenie torusa
    gltMakeTorus(torusBatch, 1.0f, 0.3f, 52, 26);

    glPointSize(4.0f);
	}

void SpecialKeys(int key, int x, int y)
	{
	if(key == GLUT_KEY_UP)
		viewFrame.RotateWorld(m3dDegToRad(-5.0), 1.0f, 0.0f, 0.0f);

	if(key == GLUT_KEY_DOWN)
		viewFrame.RotateWorld(m3dDegToRad(5.0), 1.0f, 0.0f, 0.0f);
        
	if(key == GLUT_KEY_LEFT)
		viewFrame.RotateWorld(m3dDegToRad(-5.0), 0.0f, 1.0f, 0.0f);
        
	if(key == GLUT_KEY_RIGHT)
		viewFrame.RotateWorld(m3dDegToRad(5.0), 0.0f, 1.0f, 0.0f);

	// Od�wie�enie okna
	glutPostRedisplay();
	}


void ChangeSize(int w, int h)
	{
	// Zabezpieczenie przed dzieleniem przez zero
	if(h == 0)
		h = 1;

	// Ustawienie widoku na wymiary okna
    glViewport(0, 0, w, h);

    viewFrustum.SetPerspective(35.0f, float(w)/float(h), 1.0f, 100.0f);
    
    projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
    transformPipeline.SetMatrixStacks(modelViewMatix, projectionMatrix);
	}

///////////////////////////////////////////////////////////////////////////////
// G��wny punkt pocz�tkowy programu opartego na bibliotece GLUT.
int main(int argc, char* argv[])
    {
	gltSetWorkingDirectory(argv[0]);
	
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Program do testowania geometrii");
    glutReshapeFunc(ChangeSize);
    glutSpecialFunc(SpecialKeys);
    glutDisplayFunc(RenderScene);

    // Utworzenie menu
    glutCreateMenu(ProcessMenu);
    glutAddMenuEntry("Testowanie g��bi",1);
    glutAddMenuEntry("Usu� powierzchnie tylne",2);
    glutAddMenuEntry("Tryb wype�niania", 3);
    glutAddMenuEntry("Tryb liniowy", 4);
    glutAddMenuEntry("Tryb punktowy", 5);
    
    glutAttachMenu(GLUT_RIGHT_BUTTON);
        
    
	GLenum err = glewInit();
	if (GLEW_OK != err) {
		fprintf(stderr, "B��d GLEW: %s\n", glewGetErrorString(err));
		return 1;
    }
	
	SetupRC();
    
	glutMainLoop();
	return 0;
    }
