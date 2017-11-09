// Objects.cpp
// OpenGL. Ksiêga eksperta, rozdzia³ 4
// Demonstracja obiektów standardowych biblioteki OpenGL
// Autor programu: Richard S. Wright Jr.

#include <GLTools.h>	// Biblioteka OpenGL
#include <GLMatrixStack.h>
#include <GLFrame.h>
#include <GLFrustum.h>
#include <GLBatch.h>
#include <GLGeometryTransform.h>

#include <math.h>
#ifdef __APPLE__
#include <glut/glut.h>
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>
#endif

/////////////////////////////////////////////////////////////////////////////////
// Zestaw potrzebnych klas
GLShaderManager		shaderManager;
GLMatrixStack		modelViewMatrix;
GLMatrixStack		projectionMatrix;
GLFrame				cameraFrame;
GLFrame             objectFrame;
GLFrustum			viewFrustum;

GLTriangleBatch     sphereBatch;
GLTriangleBatch     torusBatch;
GLTriangleBatch     cylinderBatch;
GLTriangleBatch     coneBatch;
GLTriangleBatch     diskBatch;


GLGeometryTransform	transformPipeline;
M3DMatrix44f		shadowMatrix;


GLfloat vGreen[] = { 0.0f, 1.0f, 0.0f, 1.0f };
GLfloat vBlack[] = { 0.0f, 0.0f, 0.0f, 1.0f };


// Rejestruje krok efektów
int nStep = 0;


///////////////////////////////////////////////////////////////////////////////
// Funkcja inicjalizuj¹ca w kontekœcie renderowania. 
// Pierwsza okazja do wykonania jakichœ dzia³añ zwi¹zanych z OpenGL.
void SetupRC()
	{
    // Czarne t³o
    glClearColor(0.7f, 0.7f, 0.7f, 1.0f );

	shaderManager.InitializeStockShaders();

	glEnable(GL_DEPTH_TEST);

	transformPipeline.SetMatrixStacks(modelViewMatrix, projectionMatrix);

	cameraFrame.MoveForward(-15.0f);

    
    // Kula
    gltMakeSphere(sphereBatch, 3.0, 10, 20);
 
    // Torus
    gltMakeTorus(torusBatch, 3.0f, 0.75f, 15, 15);
    
    // Cylinder
    gltMakeCylinder(cylinderBatch, 2.0f, 2.0f, 3.0f, 13, 2);
    
    // Sto¿ek
    gltMakeCylinder(coneBatch, 2.0f, 0.0f, 3.0f, 13, 2);
    
    // Dysk
    gltMakeDisk(diskBatch, 1.5f, 3.0f, 13, 3);
    }


/////////////////////////////////////////////////////////////////////////
void DrawWireFramedBatch(GLTriangleBatch* pBatch)
    {
    shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vGreen);
    pBatch->Draw();
    
    // Rysowanie czarnego obrysu
    glPolygonOffset(-1.0f, -1.0f);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_POLYGON_OFFSET_LINE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glLineWidth(2.5f);
    shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlack);
    pBatch->Draw();
    
    // Przywrócenie trybu wielok¹ta i testowanie g³êbi
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_POLYGON_OFFSET_LINE);
    glLineWidth(1.0f);
    glDisable(GL_BLEND);
    glDisable(GL_LINE_SMOOTH);
    }


///////////////////////////////////////////////////////////////////////////////
// Rysuje scenê
void RenderScene(void)
	{    
	// Czyœci okno bie¿¹cym kolorem
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	modelViewMatrix.PushMatrix();
		M3DMatrix44f mCamera;
		cameraFrame.GetCameraMatrix(mCamera);
		modelViewMatrix.MultMatrix(mCamera);

        M3DMatrix44f mObjectFrame;
        objectFrame.GetMatrix(mObjectFrame);
        modelViewMatrix.MultMatrix(mObjectFrame);

        shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlack);

        switch(nStep) {
            case 0:
                DrawWireFramedBatch(&sphereBatch);
                break;
            case 1:
                DrawWireFramedBatch(&torusBatch);
                break;
            case 2:
                DrawWireFramedBatch(&cylinderBatch);
                break;
            case 3:
                DrawWireFramedBatch(&coneBatch);
                break;
            case 4:
                DrawWireFramedBatch(&diskBatch);
                break;
            }
		
	modelViewMatrix.PopMatrix();

	// Wykonuje wszystkie polecenia rysowania
	glutSwapBuffers();
    }


// Porusza uk³adem odniesienia kamery w odpowiedzi na naciœniêcia klawiszy
void SpecialKeys(int key, int x, int y)
    {
	if(key == GLUT_KEY_UP)
		objectFrame.RotateWorld(m3dDegToRad(-5.0f), 1.0f, 0.0f, 0.0f);
    
	if(key == GLUT_KEY_DOWN)
		objectFrame.RotateWorld(m3dDegToRad(5.0f), 1.0f, 0.0f, 0.0f);
	
	if(key == GLUT_KEY_LEFT)
		objectFrame.RotateWorld(m3dDegToRad(-5.0f), 0.0f, 1.0f, 0.0f);
    
	if(key == GLUT_KEY_RIGHT)
		objectFrame.RotateWorld(m3dDegToRad(5.0f), 0.0f, 1.0f, 0.0f);
    
	glutPostRedisplay();
    }




///////////////////////////////////////////////////////////////////////////////
// Naciœniêty zwyk³y klawisz ze znakiem z zestawu ASCII.
// Gdy naciœniêta jest spacja, przesuwa scenê do przodu
void KeyPressFunc(unsigned char key, int x, int y)
	{
	if(key == 32)
		{
		nStep++;

		if(nStep > 4)
			nStep = 0;
		}
        
    switch(nStep)
        {
        case 0: 
            glutSetWindowTitle("Kula");
            break;
        case 1:
            glutSetWindowTitle("Torus");
            break;
        case 2:
            glutSetWindowTitle("Walec");
            break;
        case 3:
            glutSetWindowTitle("Sto¿ek");
            break;
        case 4:
            glutSetWindowTitle("Dysk");
            break;
        }
                
    glutPostRedisplay();
	}

///////////////////////////////////////////////////////////////////////////////
// Zmieni³ siê rozmiar okna lub dopiero je utworzono. W obu przypadkach musimy
// ustawiæ obszar widoku na wymiary okna.
void ChangeSize(int w, int h)
	{
	glViewport(0, 0, w, h);
	viewFrustum.SetPerspective(35.0f, float(w) / float(h), 1.0f, 500.0f);
	projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
	modelViewMatrix.LoadIdentity();
	}

///////////////////////////////////////////////////////////////////////////////
// Punkt pocz¹tkowy programu
int main(int argc, char* argv[])
	{
	gltSetWorkingDirectory(argv[0]);
	
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Kula");
    glutReshapeFunc(ChangeSize);
    glutKeyboardFunc(KeyPressFunc);
    glutSpecialFunc(SpecialKeys);
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
