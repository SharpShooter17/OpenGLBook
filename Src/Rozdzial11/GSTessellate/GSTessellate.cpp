// Teselacja przy u¿yciu shaderów geometrii
// OpenGL. Ksiêga eksperta
// Autor programu: Graham Sellers

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
static GLMatrixStack        modelViewMatrix;
static GLMatrixStack        projectionMatrix;
static GLGeometryTransform  transformPipeline;
static GLShaderManager      shaderManager;

static GLuint               tessellateProgram;      // Shader œwiat³a rozproszonego
static GLint                locMVP;                 // Lokalizacja zmiennej uniform przechowuj¹cej macierz rzutowania model-widok
static GLint                locMV;                  // Lokalizacja zmiennej uniform macierzy model-widok
static GLint                locNM;                  // Lokalizacja zmiennej uniform macierzy normalnej

static GLuint               vao;
static GLuint               vertex_buffer;
static GLuint               element_buffer;

// Funkcja inicjalizuj¹ca kontekst renderowania
void SetupRC(void)
{
    // T³o
    glClearColor(0.2f, 0.2f, 0.3f, 1.0f );

    glEnable(GL_DEPTH_TEST);

    shaderManager.InitializeStockShaders();
    viewFrame.MoveForward(4.0f);

    tessellateProgram = gltLoadShaderTripletWithAttributes("../../../Data/Shaders/Rozdzial11/GSTessellate.vs",
                                                           "../../../Data/Shaders/Rozdzial11/GSTessellate.gs",
                                                           "../../../Data/Shaders/Rozdzial11/GSTessellate.fs",
                                                           2,
                                                           GLT_ATTRIBUTE_VERTEX, "vVertex",
                                                           GLT_ATTRIBUTE_NORMAL, "vNormal");

    locMVP = glGetUniformLocation(tessellateProgram, "mvpMatrix");
    locMV  = glGetUniformLocation(tessellateProgram, "mvMatrix");
    locNM  = glGetUniformLocation(tessellateProgram, "normalMatrix");

    static const GLfloat vertices[] =
    {
        -1.0f, -1.0f, -1.0f,        // A
        -1.0f, -1.0f,  1.0f,        // B
        -1.0f,  1.0f, -1.0f,        // C
        -1.0f,  1.0f,  1.0f,        // D
         1.0f, -1.0f, -1.0f,        // E
         1.0f, -1.0f,  1.0f,        // F
         1.0f,  1.0f, -1.0f,        // G
         1.0f,  1.0f,  1.0f         // H
    };

    static const GLshort indices[] =
    {
        0, 1, 2,
        3, 2, 1,
        1, 5, 3,
        7, 3, 5,
        5, 4, 7,
        6, 7, 4,
        4, 0, 6,
        2, 6, 0,
        4, 5, 0,
        1, 0, 5,
        3, 7, 2,
        6, 2, 7
    };

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);
    glGenBuffers(1, &element_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
}

// Porz¹dkowanie
void ShutdownRC(void)
{
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &element_buffer);
    glDeleteBuffers(1, &vertex_buffer);
}

// Rysuje scenê
void RenderScene(void)
{
    static CStopWatch rotTimer;

    // Czyœci okno i bufor g³êbi
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    modelViewMatrix.PushMatrix(viewFrame);
    modelViewMatrix.Rotate(rotTimer.GetElapsedSeconds() * 20.0f, 0.0f, 1.0f, 0.0f);
    modelViewMatrix.Rotate(rotTimer.GetElapsedSeconds() * 17.0f, 1.0f, 0.0f, 0.0f);

    GLfloat vEyeLight[] = { -100.0f, 100.0f, 100.0f };
    GLfloat vAmbientColor[] = { 0.1f, 0.1f, 0.1f, 1.0f };
    GLfloat vDiffuseColor[] = { 0.1f, 1.0f, 0.1f, 1.0f };
    GLfloat vSpecularColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };

    glUseProgram(tessellateProgram);
    glUniformMatrix4fv(locMVP, 1, GL_FALSE, transformPipeline.GetModelViewProjectionMatrix());
    glUniformMatrix4fv(locMV, 1, GL_FALSE, transformPipeline.GetModelViewMatrix());
    glUniformMatrix3fv(locNM, 1, GL_FALSE, transformPipeline.GetNormalMatrix());

    glDisable(GL_CULL_FACE);
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, NULL);

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
    glutCreateWindow("Teselacja przy u¿yciu shadera geometrii");
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
