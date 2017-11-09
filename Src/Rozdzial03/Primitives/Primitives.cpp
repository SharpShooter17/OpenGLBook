// Primitieves.cpp
// OpenGL. Ksiêga eksperta, rozdzia³ 2
// Demonstracja 7 podstawowych obiektów geometrycznych
// Program autorstwa Richarda S. Wrighta Jr.

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
// Potrzebne klasy
GLShaderManager		shaderManager;
GLMatrixStack		modelViewMatrix;
GLMatrixStack		projectionMatrix;
GLFrame				cameraFrame;
GLFrame             objectFrame;
GLFrustum			viewFrustum;

GLBatch				pointBatch;
GLBatch				lineBatch;
GLBatch				lineStripBatch;
GLBatch				lineLoopBatch;
GLBatch				triangleBatch;
GLBatch             triangleStripBatch;
GLBatch             triangleFanBatch;

GLGeometryTransform	transformPipeline;
M3DMatrix44f		shadowMatrix;


GLfloat vGreen[] = { 0.0f, 1.0f, 0.0f, 1.0f };
GLfloat vBlack[] = { 0.0f, 0.0f, 0.0f, 1.0f };


// Œledzenie rodzaju wyœwietlanych efektów
int nStep = 0;


///////////////////////////////////////////////////////////////////////////////
// Ta funkcja wykonuje wszystkie dzia³ania zwi¹zane z inicjalizowaniem w kontekœcie renderowania. 
// Jest to pierwsza okazja do wykonania jakichœ zadañ zwi¹zanych z OpenGL.
void SetupRC()
	{
    // Czarne t³o
    glClearColor(0.7f, 0.7f, 0.7f, 1.0f );

	shaderManager.InitializeStockShaders();

	glEnable(GL_DEPTH_TEST);

	transformPipeline.SetMatrixStacks(modelViewMatrix, projectionMatrix);

	cameraFrame.MoveForward(-15.0f);
    
    //////////////////////////////////////////////////////////////////////
    // Kilka punktów tworz¹cych w przybli¿eniu kszta³t Florydy
    GLfloat vCoast[24][3] = {{2.80, 1.20, 0.0 }, {2.0,  1.20, 0.0 },
                            {2.0,  1.08, 0.0 },  {2.0,  1.08, 0.0 },
                            {0.0,  0.80, 0.0 },  {-.32, 0.40, 0.0 },
                            {-.48, 0.2, 0.0 },   {-.40, 0.0, 0.0 },
                            {-.60, -.40, 0.0 },  {-.80, -.80, 0.0 },
                            {-.80, -1.4, 0.0 },  {-.40, -1.60, 0.0 },
                            {0.0, -1.20, 0.0 },  { .2, -.80, 0.0 },
                            {.48, -.40, 0.0 },   {.52, -.20, 0.0 },
                            {.48,  .20, 0.0 },   {.80,  .40, 0.0 },
                            {1.20, .80, 0.0 },   {1.60, .60, 0.0 },
                            {2.0, .60, 0.0 },    {2.2, .80, 0.0 },
                            {2.40, 1.0, 0.0 },   {2.80, 1.0, 0.0 }};
    
    // Wczytanie porcji punktów
    pointBatch.Begin(GL_POINTS, 24);
    pointBatch.CopyVertexData3f(vCoast);
    pointBatch.End();
    
    // Zbiór odcinków
    lineBatch.Begin(GL_LINES, 24);
    lineBatch.CopyVertexData3f(vCoast);
    lineBatch.End();
    
    // Linia ³amana
    lineStripBatch.Begin(GL_LINE_STRIP, 24);
    lineStripBatch.CopyVertexData3f(vCoast);
    lineStripBatch.End();
    
    // £amana zamkniêta
    lineLoopBatch.Begin(GL_LINE_LOOP, 24);
    lineLoopBatch.CopyVertexData3f(vCoast);
    lineLoopBatch.End();
    
    // Trójk¹ty — zrobimy ostros³up
    GLfloat vPyramid[12][3] = { -2.0f, 0.0f, -2.0f, 
                                2.0f, 0.0f, -2.0f, 
                                0.0f, 4.0f, 0.0f,
                                
                                2.0f, 0.0f, -2.0f,
                                2.0f, 0.0f, 2.0f,
                                0.0f, 4.0f, 0.0f,
                                
                                2.0f, 0.0f, 2.0f,
                                -2.0f, 0.0f, 2.0f,
                                0.0f, 4.0f, 0.0f,
                                
                                -2.0f, 0.0f, 2.0f,
                                -2.0f, 0.0f, -2.0f,
                                 0.0f, 4.0f, 0.0f};
    
    triangleBatch.Begin(GL_TRIANGLES, 12);
    triangleBatch.CopyVertexData3f(vPyramid);
    triangleBatch.End();
    

    // Rozeta z trójk¹tów — szeœciok¹t. Podniesiony trochê œrodek
    GLfloat vPoints[100][3];    // Tablica pocz¹tkowa — wiêcej ni¿ potrzebujemy
    int nVerts = 0;
    GLfloat r = 3.0f;
    vPoints[nVerts][0] = 0.0f;
    vPoints[nVerts][1] = 0.0f;
    vPoints[nVerts][2] = 0.0f;

    for(GLfloat angle = 0; angle < M3D_2PI; angle += M3D_2PI / 6.0f) {
        nVerts++;
        vPoints[nVerts][0] = float(cos(angle)) * r;
        vPoints[nVerts][1] = float(sin(angle)) * r;
        vPoints[nVerts][2] = -0.5f;
        }

    // Zamkniêcie rozety
    nVerts++;
    vPoints[nVerts][0] = r;
    vPoints[nVerts][1] = 0;
    vPoints[nVerts][2] = 0.0f;
        
    // £adowanie
    triangleFanBatch.Begin(GL_TRIANGLE_FAN, 8);
    triangleFanBatch.CopyVertexData3f(vPoints);
    triangleFanBatch.End();     
        
    // Pas trójk¹tów — niewielki pierœcieñ lub cylinder
    int iCounter = 0;
    GLfloat radius = 3.0f;
    for(GLfloat angle = 0.0f; angle <= (2.0f*M3D_PI); angle += 0.3f)
		{
        GLfloat x = radius * sin(angle);
        GLfloat y = radius * cos(angle);
            
        // Okreœlenie punktu i przesuniêcie wartoœci Z nieco do góry
        vPoints[iCounter][0] = x;
        vPoints[iCounter][1] = y;
        vPoints[iCounter][2] = -0.5;
        iCounter++;

        vPoints[iCounter][0] = x;
        vPoints[iCounter][1] = y;
        vPoints[iCounter][2] = 0.5;
        iCounter++;            
		}

    // Zamkniêcie pêtli
    vPoints[iCounter][0] = vPoints[0][0];
    vPoints[iCounter][1] = vPoints[0][1];
    vPoints[iCounter][2] = -0.5;
    iCounter++;
    
    vPoints[iCounter][0] = vPoints[1][0];
    vPoints[iCounter][1] = vPoints[1][1];
    vPoints[iCounter][2] = 0.5;
    iCounter++;            
        
    // £adowanie pasa trójk¹tów
    triangleStripBatch.Begin(GL_TRIANGLE_STRIP, iCounter);
    triangleStripBatch.CopyVertexData3f(vPoints);
    triangleStripBatch.End();    
    }


/////////////////////////////////////////////////////////////////////////
void DrawWireFramedBatch(GLBatch* pBatch)
    {
    // Nadanie jednolitego zielonego koloru
    shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vGreen);
    pBatch->Draw();
    
    // Rysowanie czarnego konturu
    glPolygonOffset(-1.0f, -1.0f);      // Przesuniêcie wartoœci g³êbi
    glEnable(GL_POLYGON_OFFSET_LINE);

    // Wyg³adzanie rysowanych linii
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Rysowanie czarnego szkieletu geometrii
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glLineWidth(2.5f);
    shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlack);
    pBatch->Draw();
    
    // Przywrócenie wszystkiego do postaci pocz¹tkowej
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
	// Czyœci okno aktualnie ustawionym kolorem
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
                glPointSize(4.0f);
                pointBatch.Draw();
                glPointSize(1.0f);
                break;
            case 1:
                glLineWidth(2.0f);
                lineBatch.Draw();
                glLineWidth(1.0f);
                break;
            case 2:
                glLineWidth(2.0f);
                lineStripBatch.Draw();
                glLineWidth(1.0f);
                break;
            case 3:
                glLineWidth(2.0f);
                lineLoopBatch.Draw();
                glLineWidth(1.0f);
                break;
            case 4:
                DrawWireFramedBatch(&triangleBatch);
                break;
            case 5:
                DrawWireFramedBatch(&triangleStripBatch);
                break;
            case 6:
                DrawWireFramedBatch(&triangleFanBatch);
                break;
            }
		
	modelViewMatrix.PopMatrix();

	// Wykonanie instrukcji rysowania
	glutSwapBuffers();
    }


// Odpowiadanie na naciœniêcia klawiszy strza³ek poprzez poruszanie uk³adu odniesienia kamery
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
// Naciœniêto zwyk³y klawisz znaku ASCII.
// Zmiana sceny na nastêpn¹ w wyniku naciœniêcia spacji.
void KeyPressFunc(unsigned char key, int x, int y)
	{
	if(key == 32)
		{
		nStep++;

		if(nStep > 6)
			nStep = 0;
		}
        
    switch(nStep)
        {
        case 0: 
            glutSetWindowTitle("GL_POINTS");
            break;
        case 1:
            glutSetWindowTitle("GL_LINES");
            break;
        case 2:
            glutSetWindowTitle("GL_LINE_STRIP");
            break;
        case 3:
            glutSetWindowTitle("GL_LINE_LOOP");
            break;
        case 4:
            glutSetWindowTitle("GL_TRIANGLES");
            break;
        case 5:
            glutSetWindowTitle("GL_TRIANGLE_STRIP");
            break;
        case 6:
            glutSetWindowTitle("GL_TRIANGLE_FAN");
            break;
        }
                
    glutPostRedisplay();
	}

///////////////////////////////////////////////////////////////////////////////
// Zmieni³ siê rozmiar okna lub okno zosta³o w³aœnie utworzone. W obu przypadkach musimy
// sprawdziæ wymiary tego okna, aby ustawiæ obszar widoku i utworzyæ macierz rzutowania.
void ChangeSize(int w, int h)
	{
	glViewport(0, 0, w, h);
	viewFrustum.SetPerspective(35.0f, float(w) / float(h), 1.0f, 500.0f);
	projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
	modelViewMatrix.LoadIdentity();
	}

///////////////////////////////////////////////////////////////////////////////
// G³ówny punkt pocz¹tkowy programu opartego na bibliotece GLUT
int main(int argc, char* argv[])
	{
	gltSetWorkingDirectory(argv[0]);
	
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
	glutInitWindowSize(800, 600);
	glutCreateWindow("GL_POINTS");
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
