// Smoother.cpp
// OpenGL. Ksiêga eksperta
// Demonstruje wyg³adzanie punktów i linii
// Autor: Richard S. Wright Jr.
#include <GLTools.h>	// Biblioteka OpenGL
#include <GLFrustum.h>

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

// Tablica ma³ych gwiazd
#define SMALL_STARS     100
#define MEDIUM_STARS     40
#define LARGE_STARS      15

#define SCREEN_X        800
#define SCREEN_Y        600

///////////////////////////////////////////////////////////////////////
// Resetowanie odpowiednich znaczników w odpowiedzi na wybrane opcje menu
void ProcessMenu(int value)
    {
    switch(value)
        {
        case 1:
            // W³¹czenie wyg³adzania i przekazanie wskazówki, ¿e zale¿y nam
            // na mo¿liwie najlepszym wyniku.
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glEnable(GL_BLEND);
            glEnable(GL_POINT_SMOOTH);
            glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
            glEnable(GL_LINE_SMOOTH);
            glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
            glEnable(GL_POLYGON_SMOOTH);
            glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
            break;

        case 2:
            // Wy³¹czenie mieszania kolorów i wyg³adzania
            glDisable(GL_BLEND);
            glDisable(GL_LINE_SMOOTH);
            glDisable(GL_POINT_SMOOTH);
            break;

        default:
            break;
        }
        
    // Ponowne rysowanie
    glutPostRedisplay();
    }


///////////////////////////////////////////////////
// Rysowanie sceny
void RenderScene(void)
    {		        
    // Clear the window
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
         
    // Wszystko jest bia³e
    GLfloat vWhite [] = { 1.0f, 1.0f, 1.0f, 1.0f };
    shaderManager.UseStockShader(GLT_SHADER_FLAT, viewFrustum.GetProjectionMatrix(), vWhite);
    
    // Rysuje ma³e gwiazdy
    glPointSize(1.0f);
    smallStarBatch.Draw();
            
    // Rysuje œrednie gwiazdy
    glPointSize(4.0f);
    mediumStarBatch.Draw();
    
    // Rysuje du¿e gwiazdy
    glPointSize(8.0f);
    largeStarBatch.Draw();
        
    // Rysuje ksiê¿yc
    moonBatch.Draw();

    // Rysuje odleg³y horyzont
    glLineWidth(3.5);
    mountainRangeBatch.Draw();
    
    moonBatch.Draw();

    // Zamiana buforów
    glutSwapBuffers();
    }


// Ta funkcja wykonuje wszystkie dzia³ania zwi¹zane z inicjalizowaniem w kontekœcie renderowania.
void SetupRC()
    {
    M3DVector3f vVerts[SMALL_STARS];       // SMALL_STARS to najwiêksza porcja, jakiej bêdziemy potrzebowaæ
    int i;
        
    shaderManager.InitializeStockShaders();
        
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
        
    moonBatch.Begin(GL_TRIANGLE_FAN, 34);
    int nVerts = 0;
    vVerts[nVerts][0] = x;
    vVerts[nVerts][1] = y;
    vVerts[nVerts][2] = 0.0f;
        for(angle = 0; angle < 2.0f * 3.141592f; angle += 0.2f) {
           nVerts++;
           vVerts[nVerts][0] = x + float(cos(angle)) * r;
           vVerts[nVerts][1] = y + float(sin(angle)) * r;
           vVerts[nVerts][2] = 0.0f;
           }
    nVerts++;
   
    vVerts[nVerts][0] = x + r;;
    vVerts[nVerts][1] = y;
    vVerts[nVerts][2] = 0.0f;
    moonBatch.CopyVertexData3f(vVerts);
    moonBatch.End();     
            
    // Czarne t³o
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f );
    }



void ChangeSize(int w, int h)
    {
    // Ochrona przed dzieleniem przez zero
    if(h == 0)
        h = 1;

    // Ustawienie widoku na rozmiar okna
    glViewport(0, 0, w, h);

    // Ustawienie przestrzeni widocznej (lewa, prawa, dó³, góra, blisko, daleko)
    viewFrustum.SetOrthographic(0.0f, SCREEN_X, 0.0f, SCREEN_Y, -1.0f, 1.0f);
    }

int main(int argc, char* argv[])
	{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Wyg³adzanie schodków");
	
	// Tworzenie menu
	glutCreateMenu(ProcessMenu);
	glutAddMenuEntry("Renderowanie z wyg³adzaniem",1);
	glutAddMenuEntry("Normalne renderowanie",2);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
	
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
