// OpenGL SuperBible 5th Edition
// Demonstruje nieszczenie geometrii przy u¿yciu shaderów geometrii OpenGL
// Program by Graham Sellers

// OpenGL
#include <GLTools.h>
#include <GLMatrixStack.h>
#include <GLFrame.h>
#include <GLFrustum.h>
#include <GLGeometryTransform.h>
#include <Stopwatch.h>

#include <math.h>
#ifdef __APPLE__
#include <glut/glut.h>
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>
#endif

#ifdef _MSC_VER
#pragma comment (lib, "GLTools.lib")
#endif /* _MSC_VER */

static GLFrame              viewFrame;
static GLFrustum            viewFrustum;
static GLTriangleBatch      torusBatch;
static GLMatrixStack        modelViewMatrix;
static GLMatrixStack        projectionMatrix;
static GLGeometryTransform  transformPipeline;
static GLShaderManager      shaderManager;

static GLuint               explodeProgram;         // Shader geometrii
static GLint                locMVP;                 // Lokalizacja zmiennej uniform przechowuj¹cej macierz rzutowania model-widok
static GLint                locMV;                  // Lokalizacja zmiennej uniform macierzy model-widok
static GLint                locNM;                  // Lokalizacja zmiennej uniform macierzy normalnej

static GLint                locPushOut;             // Jak daleko wypchn¹æ geometriê

// Funkcja inicjalizuj¹ca kontekst renderowania
void SetupRC(void)
{
    // T³o
    glClearColor(0.2f, 0.2f, 0.3f, 1.0f );

    glEnable(GL_DEPTH_TEST);

    shaderManager.InitializeStockShaders();
    viewFrame.MoveForward(4.0f);

    // Tworzenie torusa
    gltMakeTorus(torusBatch, .70f, 0.10f, 11, 7);

    explodeProgram = gltLoadShaderTripletWithAttributes("../../../Data/Shaders/Rozdzial11/GSExplode.vs",
                                                        "../../../Data/Shaders/Rozdzial11/GSExplode.gs",
                                                        "../../../Data/Shaders/Rozdzial11/GSExplode.fs",
                                                        2,
                                                        GLT_ATTRIBUTE_VERTEX, "vVertex",
                                                        GLT_ATTRIBUTE_NORMAL, "vNormal");

    locMVP = glGetUniformLocation(explodeProgram, "mvpMatrix");
    locMV  = glGetUniformLocation(explodeProgram, "mvMatrix");
    locNM  = glGetUniformLocation(explodeProgram, "normalMatrix");
    locPushOut = glGetUniformLocation(explodeProgram, "push_out");
}

// Porz¹dkowanie
void ShutdownRC(void)
{
    glDeleteProgram(explodeProgram);
}

// Rysuje scenê
void RenderScene(void)
{
    static CStopWatch rotTimer;

    // Czyœci okno i bufor g³êbi
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    modelViewMatrix.PushMatrix(viewFrame);
    modelViewMatrix.Rotate(rotTimer.GetElapsedSeconds() * 10.0f, 0.0f, 1.0f, 0.0f);
    modelViewMatrix.Rotate(rotTimer.GetElapsedSeconds() * 13.0f, 1.0f, 0.0f, 0.0f);

    GLfloat vEyeLight[] = { -100.0f, 100.0f, 100.0f };
    GLfloat vAmbientColor[] = { 0.1f, 0.1f, 0.1f, 1.0f };
    GLfloat vDiffuseColor[] = { 0.1f, 1.0f, 0.1f, 1.0f };
    GLfloat vSpecularColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };

    glUseProgram(explodeProgram);
    glUniformMatrix4fv(locMVP, 1, GL_FALSE, transformPipeline.GetModelViewProjectionMatrix());
    glUniformMatrix4fv(locMV, 1, GL_FALSE, transformPipeline.GetModelViewMatrix());
    glUniformMatrix3fv(locNM, 1, GL_FALSE, transformPipeline.GetNormalMatrix());

    float push_out = sinf(rotTimer.GetElapsedSeconds() * 3.0f) * 0.1f + 0.2f;

    glUniform1f(locPushOut, push_out);

    torusBatch.Draw();

    modelViewMatrix.PopMatrix();


    glutSwapBuffers();
    glutPostRedisplay();
}

void ChangeSize(int w, int h)
{
    // Zabezpieczenie przed dzieleniem przez zero
    if(h == 0)
        h = 1;

    // Ustawia widok na wymiary okna
    glViewport(0, 0, w, h);

    viewFrustum.SetPerspective(35.0f, float(w)/float(h), 1.0f, 100.0f);

    projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
    transformPipeline.SetMatrixStacks(modelViewMatrix, projectionMatrix);
}

///////////////////////////////////////////////////////////////////////////////
// G³ówny punkt wejœciowy wszystkich programów opartych na bibliotece GLUT
int main(int argc, char* argv[])
{
    gltSetWorkingDirectory(argv[0]);

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Rozbieranie obiektu w shaderze geometrii");
    glutReshapeFunc(ChangeSize);
    glutDisplayFunc(RenderScene);

    GLenum err = glewInit();
    if (GLEW_OK != err) {
        fprintf(stderr, "B³¹d GLEW: %s\n", glewGetErrorString(err));
        return 1;
    }

    SetupRC();
    glutMainLoop();
    ShutdownRC();
    return 0;
}
