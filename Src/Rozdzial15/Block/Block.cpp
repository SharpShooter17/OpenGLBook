// Block.cpp
// OpenGL. Ksiêga eksperta
// Dmonstracja kilku podstawowych pojêæ zwi¹zanych z grafik¹ trójwymiarow¹
// Autor programu: Richard S. Wright Jr.

#include <glew.h>
#include <glxew.h>

#include <GLTools.h>	// OpenGL
#include <GLMatrixStack.h>
#include <GLFrame.h>
#include <GLFrustum.h>
#include <GLBatch.h>
#include <GLGeometryTransform.h>

#include <math.h>

#ifdef linux
#include <cstdlib> 
#endif

/////////////////////////////////////////////////////////////////////////////////
// Potrzebne klasy
GLShaderManager		shaderManager;
GLMatrixStack		modelViewMatrix;
GLMatrixStack		projectionMatrix;
GLFrame			cameraFrame;
GLFrustum		viewFrustum;
GLBatch			cubeBatch;
GLBatch			floorBatch;
GLBatch			topBlock;
GLBatch			frontBlock;
GLBatch			leftBlock;

GLGeometryTransform	transformPipeline;
M3DMatrix44f		shadowMatrix;

int nStep = 0;

// Dane oœwietlenia
GLfloat lightAmbient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
GLfloat lightDiffuse[] = { 0.7f, 0.7f, 0.7f, 1.0f };
GLfloat lightSpecular[] = { 0.9f, 0.9f, 0.9f };
GLfloat vLightPos[] = { -8.0f, 20.0f, 100.0f, 1.0f };

GLuint textures[4];


void EarlyInitGLXfnPointers()
{

    glGenVertexArraysAPPLE = (void(*)(GLsizei, const GLuint*))glXGetProcAddressARB((GLubyte*)"glGenVertexArrays");
    glBindVertexArrayAPPLE = (void(*)(const GLuint))glXGetProcAddressARB((GLubyte*)"glBindVertexArray");
    glDeleteVertexArraysAPPLE = (void(*)(GLsizei, const GLuint*))glXGetProcAddressARB((GLubyte*)"glGenVertexArrays");
 glXCreateContextAttribsARB = (GLXContext(*)(Display* dpy, GLXFBConfig config, GLXContext share_context, Bool direct, const int *attrib_list))glXGetProcAddressARB((GLubyte*)"glXCreateContextAttribsARB");
 glXChooseFBConfig = (GLXFBConfig*(*)(Display *dpy, int screen, const int *attrib_list, int *nelements))glXGetProcAddressARB((GLubyte*)"glXChooseFBConfig");
 glXGetVisualFromFBConfig = (XVisualInfo*(*)(Display *dpy, GLXFBConfig config))glXGetProcAddressARB((GLubyte*)"glXGetVisualFromFBConfig");
}

///////////////////////////////////////////////////////////////////////////////
// Tworzenie szeœcianu z ka¿dej serii trójk¹tów. Wspó³rzêdne teksturowe i normalne
// równie¿ s¹ dostarczone.
void MakeCube(GLBatch& cubeBatch)
    {
    cubeBatch.Begin(GL_TRIANGLES, 36, 1);

    /////////////////////////////////////////////
    // Wierzch szeœcianu
    cubeBatch.Normal3f(0.0f, 1.0f, 0.0f);
    cubeBatch.MultiTexCoord2f(0, 1.0f, 1.0f);
    cubeBatch.Vertex3f(1.0f, 1.0f, 1.0f);

    cubeBatch.Normal3f(0.0f, 1.0f, 0.0f);
    cubeBatch.MultiTexCoord2f(0, 1.0f, 0.0f);
    cubeBatch.Vertex3f(1.0f, 1.0f, -1.0f);

    cubeBatch.Normal3f(0.0f, 1.0f, 0.0f);
    cubeBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
    cubeBatch.Vertex3f(-1.0f, 1.0f, -1.0f);

    cubeBatch.Normal3f(0.0f, 1.0f, 0.0f);
    cubeBatch.MultiTexCoord2f(0, 1.0f, 1.0f);
    cubeBatch.Vertex3f(1.0f, 1.0f, 1.0f);

    cubeBatch.Normal3f(0.0f, 1.0f, 0.0f);
    cubeBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
    cubeBatch.Vertex3f(-1.0f, 1.0f, -1.0f);

    cubeBatch.Normal3f(0.0f, 1.0f, 0.0f);
    cubeBatch.MultiTexCoord2f(0, 0.0f, 1.0f);
    cubeBatch.Vertex3f(-1.0f, 1.0f, 1.0f);


    ////////////////////////////////////////////
    // Podstawa szeœcianu
    cubeBatch.Normal3f(0.0f, -1.0f, 0.0f);
    cubeBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
    cubeBatch.Vertex3f(-1.0f, -1.0f, -1.0f);

    cubeBatch.Normal3f(0.0f, -1.0f, 0.0f);
    cubeBatch.MultiTexCoord2f(0, 1.0f, 0.0f);
    cubeBatch.Vertex3f(1.0f, -1.0f, -1.0f);

    cubeBatch.Normal3f(0.0f, -1.0f, 0.0f);
    cubeBatch.MultiTexCoord2f(0, 1.0f, 1.0f);
    cubeBatch.Vertex3f(1.0f, -1.0f, 1.0f);

    cubeBatch.Normal3f(0.0f, -1.0f, 0.0f);
    cubeBatch.MultiTexCoord2f(0, 0.0f, 1.0f);
    cubeBatch.Vertex3f(-1.0f, -1.0f, 1.0f);

    cubeBatch.Normal3f(0.0f, -1.0f, 0.0f);
    cubeBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
    cubeBatch.Vertex3f(-1.0f, -1.0f, -1.0f);

    cubeBatch.Normal3f(0.0f, -1.0f, 0.0f);
    cubeBatch.MultiTexCoord2f(0, 1.0f, 1.0f);
    cubeBatch.Vertex3f(1.0f, -1.0f, 1.0f);

    ///////////////////////////////////////////
    // Lewy bok szeœcianu
    cubeBatch.Normal3f(-1.0f, 0.0f, 0.0f);
    cubeBatch.MultiTexCoord2f(0, 1.0f, 1.0f);
    cubeBatch.Vertex3f(-1.0f, 1.0f, 1.0f);

    cubeBatch.Normal3f(-1.0f, 0.0f, 0.0f);
    cubeBatch.MultiTexCoord2f(0, 1.0f, 0.0f);
    cubeBatch.Vertex3f(-1.0f, 1.0f, -1.0f);

    cubeBatch.Normal3f(-1.0f, 0.0f, 0.0f);
    cubeBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
    cubeBatch.Vertex3f(-1.0f, -1.0f, -1.0f);

    cubeBatch.Normal3f(-1.0f, 0.0f, 0.0f);
    cubeBatch.MultiTexCoord2f(0, 1.0f, 1.0f);
    cubeBatch.Vertex3f(-1.0f, 1.0f, 1.0f);

    cubeBatch.Normal3f(-1.0f, 0.0f, 0.0f);
    cubeBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
    cubeBatch.Vertex3f(-1.0f, -1.0f, -1.0f);

    cubeBatch.Normal3f(-1.0f, 0.0f, 0.0f);
    cubeBatch.MultiTexCoord2f(0, 0.0f, 1.0f);
    cubeBatch.Vertex3f(-1.0f, -1.0f, 1.0f);

    // Prawy bok szeœcianu
    cubeBatch.Normal3f(1.0f, 0.0f, 0.0f);
    cubeBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
    cubeBatch.Vertex3f(1.0f, -1.0f, -1.0f);

    cubeBatch.Normal3f(1.0f, 0.0f, 0.0f);
    cubeBatch.MultiTexCoord2f(0, 1.0f, 0.0f);
    cubeBatch.Vertex3f(1.0f, 1.0f, -1.0f);

    cubeBatch.Normal3f(1.0f, 0.0f, 0.0f);
    cubeBatch.MultiTexCoord2f(0, 1.0f, 1.0f);
    cubeBatch.Vertex3f(1.0f, 1.0f, 1.0f);

    cubeBatch.Normal3f(1.0f, 0.0f, 0.0f);
    cubeBatch.MultiTexCoord2f(0, 1.0f, 1.0f);
    cubeBatch.Vertex3f(1.0f, 1.0f, 1.0f);

    cubeBatch.Normal3f(1.0f, 0.0f, 0.0f);
    cubeBatch.MultiTexCoord2f(0, 0.0f, 1.0f);
    cubeBatch.Vertex3f(1.0f, -1.0f, 1.0f);

    cubeBatch.Normal3f(1.0f, 0.0f, 0.0f);
    cubeBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
    cubeBatch.Vertex3f(1.0f, -1.0f, -1.0f);

    // Przód i ty³
    // Przód
    cubeBatch.Normal3f(0.0f, 0.0f, 1.0f);
    cubeBatch.MultiTexCoord2f(0, 1.0f, 0.0f);
    cubeBatch.Vertex3f(1.0f, -1.0f, 1.0f);

    cubeBatch.Normal3f(0.0f, 0.0f, 1.0f);
    cubeBatch.MultiTexCoord2f(0, 1.0f, 1.0f);
    cubeBatch.Vertex3f(1.0f, 1.0f, 1.0f);

    cubeBatch.Normal3f(0.0f, 0.0f, 1.0f);
    cubeBatch.MultiTexCoord2f(0, 0.0f, 1.0f);
    cubeBatch.Vertex3f(-1.0f, 1.0f, 1.0f);

    cubeBatch.Normal3f(0.0f, 0.0f, 1.0f);
    cubeBatch.MultiTexCoord2f(0, 0.0f, 1.0f);
    cubeBatch.Vertex3f(-1.0f, 1.0f, 1.0f);

    cubeBatch.Normal3f(0.0f, 0.0f, 1.0f);
    cubeBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
    cubeBatch.Vertex3f(-1.0f, -1.0f, 1.0f);

    cubeBatch.Normal3f(0.0f, 0.0f, 1.0f);
    cubeBatch.MultiTexCoord2f(0, 1.0f, 0.0f);
    cubeBatch.Vertex3f(1.0f, -1.0f, 1.0f);

    // Ty³
    cubeBatch.Normal3f(0.0f, 0.0f, -1.0f);
    cubeBatch.MultiTexCoord2f(0, 1.0f, 0.0f);
    cubeBatch.Vertex3f(1.0f, -1.0f, -1.0f);

    cubeBatch.Normal3f(0.0f, 0.0f, -1.0f);
    cubeBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
    cubeBatch.Vertex3f(-1.0f, -1.0f, -1.0f);

    cubeBatch.Normal3f(0.0f, 0.0f, -1.0f);
    cubeBatch.MultiTexCoord2f(0, 0.0f, 1.0f);
    cubeBatch.Vertex3f(-1.0f, 1.0f, -1.0f);

    cubeBatch.Normal3f(0.0f, 0.0f, -1.0f);
    cubeBatch.MultiTexCoord2f(0, 0.0f, 1.0f);
    cubeBatch.Vertex3f(-1.0f, 1.0f, -1.0f);

    cubeBatch.Normal3f(0.0f, 0.0f, -1.0f);
    cubeBatch.MultiTexCoord2f(0, 1.0f, 1.0f);
    cubeBatch.Vertex3f(1.0f, 1.0f, -1.0f);

    cubeBatch.Normal3f(0.0f, 0.0f, -1.0f);
    cubeBatch.MultiTexCoord2f(0, 1.0f, 0.0f);
    cubeBatch.Vertex3f(1.0f, -1.0f, -1.0f);

    cubeBatch.End();
    }

/////////////////////////////////////////////////////////////////////////////
// Tworzenie pod³ogi, tylko wierzcho³ki i tekstura, ¿adnych normalnych
void MakeFloor(GLBatch& floorBatch)
    {
    GLfloat x = 5.0f;
    GLfloat y = -1.0f;

    floorBatch.Begin(GL_TRIANGLE_FAN, 4, 1);
        floorBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
        floorBatch.Vertex3f(-x, y, x);

        floorBatch.MultiTexCoord2f(0, 1.0f, 0.0f);
        floorBatch.Vertex3f(x, y, x);

        floorBatch.MultiTexCoord2f(0, 1.0f, 1.0f);
        floorBatch.Vertex3f(x, y, -x);

        floorBatch.MultiTexCoord2f(0, 0.0f, 1.0f);
        floorBatch.Vertex3f(-x, y, -x);
    floorBatch.End();
    }



typedef struct RenderContextRec
{
    GLXContext ctx;
    Display *dpy;
    Window win;
    int nWinWidth;
    int nWinHeight;
} RenderContext;


void CreateWindow(RenderContext *rcx)
{
    XSetWindowAttributes winAttribs;
    GLint winmask;
    GLint nMajorVer = 0;
    GLint nMinorVer = 0;
    XVisualInfo *visualInfo;
    GLXFBConfig *fbConfigs;
    int numConfigs = 0;
    static int fbAttribs[] = {
                    GLX_RENDER_TYPE,   GLX_RGBA_BIT,
                    GLX_X_RENDERABLE,  True,
                    GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
                    GLX_DOUBLEBUFFER,  True,
                    GLX_RED_SIZE, 8,
                    GLX_BLUE_SIZE, 8,
                    GLX_GREEN_SIZE, 8,
                    0 };

    // Informujemy X, ¿e bêdziemy u¿ywaæ ekranu
    rcx->dpy = XOpenDisplay(NULL);

    // Pobranie informacji o wersji
    glXQueryVersion(rcx->dpy, &nMajorVer, &nMinorVer);
    printf("Obs³ugiwana wersja GLX - %d.%d\n", nMajorVer, nMinorVer);   

    if(nMajorVer == 1 && nMinorVer < 2)
    {
        printf("B£¥D: wymagana GLX 1.2 lub nowsza\n");
        XCloseDisplay(rcx->dpy);
        exit(0);
    }

    // // Pobranie nowej konfiguracji bufora obrazu spe³niaj¹cej nasze wymagania
    fbConfigs = glXChooseFBConfig(rcx->dpy, DefaultScreen(rcx->dpy), fbAttribs, &numConfigs);
    visualInfo = glXGetVisualFromFBConfig(rcx->dpy, fbConfigs[0]);

    // Utworzenie okna X
    winAttribs.event_mask = ExposureMask | VisibilityChangeMask | 
                            KeyPressMask | PointerMotionMask    |
                            StructureNotifyMask ;

    winAttribs.border_pixel = 0;
    winAttribs.bit_gravity = StaticGravity;
    winAttribs.colormap = XCreateColormap(rcx->dpy, 
                                          RootWindow(rcx->dpy, visualInfo->screen), 
                                          visualInfo->visual, AllocNone);
    winmask = CWBorderPixel | CWBitGravity | CWEventMask| CWColormap;

    rcx->win = XCreateWindow(rcx->dpy, DefaultRootWindow(rcx->dpy), 20, 20,
                 rcx->nWinWidth, rcx->nWinHeight, 0, 
                             visualInfo->depth, InputOutput,
                 visualInfo->visual, winmask, &winAttribs);

    XMapWindow(rcx->dpy, rcx->win);

    // Tworzenie nowego kontekstu GL do renderowania
    GLint attribs[] = {
      GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
      GLX_CONTEXT_MINOR_VERSION_ARB, 2,
      0 };
    rcx->ctx = glXCreateContextAttribsARB(rcx->dpy, fbConfigs[0], 0, True, attribs);
    glXMakeCurrent(rcx->dpy, rcx->win, rcx->ctx);

    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        /* Problem: wywo³anie funkcji glewInit nie powiod³o siê */
        fprintf(stderr, "B³¹d: %s\n", glewGetErrorString(err));
    }
}

///////////////////////////////////////////////////////////////////////////////
// Funkcja inicjalizuj¹ca kontekst renderowania 
// Pierwsza okazja do wykonania jakichœ czynnoœci zwi¹zanych z OpenGL
void SetupRC(RenderContext *rcx)
{
    GLbyte *pBytes;
    GLint nWidth, nHeight, nComponents;
    GLenum format;

    shaderManager.InitializeStockShaders();

    // Czarne t³o
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f );
    glEnable(GL_DEPTH_TEST);
    glLineWidth(2.5f);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    transformPipeline.SetMatrixStacks(modelViewMatrix, projectionMatrix);

    cameraFrame.MoveForward(-15.0f);
    cameraFrame.MoveUp(6.0f);
    cameraFrame.RotateLocalX(float(m3dDegToRad(20.0f)));
    
    MakeCube(cubeBatch);
    MakeFloor(floorBatch);

    // Tworzenie góry
    topBlock.Begin(GL_TRIANGLE_FAN, 4, 1);
        topBlock.Normal3f(0.0f, 1.0f, 0.0f);
        topBlock.MultiTexCoord2f(0, 0.0f, 0.0f);
        topBlock.Vertex3f(-1.0f, 1.0f, 1.0f);

        topBlock.Normal3f(0.0f, 1.0f, 0.0f);
        topBlock.MultiTexCoord2f(0, 1.0f, 0.0f);
        topBlock.Vertex3f(1.0f, 1.0f, 1.0f);

        topBlock.Normal3f(0.0f, 1.0f, 0.0f);
        topBlock.MultiTexCoord2f(0, 1.0f, 1.0f);
        topBlock.Vertex3f(1.0f, 1.0f, -1.0f);

        topBlock.Normal3f(0.0f, 1.0f, 0.0f);
        topBlock.MultiTexCoord2f(0, 0.0f, 1.0f);
        topBlock.Vertex3f(-1.0f, 1.0f, -1.0f);
    topBlock.End();

    // Tworzenie przodu
    frontBlock.Begin(GL_TRIANGLE_FAN, 4, 1);
        frontBlock.Normal3f(0.0f, 0.0f, 1.0f);
        frontBlock.MultiTexCoord2f(0, 0.0f, 0.0f);
        frontBlock.Vertex3f(-1.0f, -1.0f, 1.0f);

        frontBlock.Normal3f(0.0f, 0.0f, 1.0f);
        frontBlock.MultiTexCoord2f(0, 1.0f, 0.0f);
        frontBlock.Vertex3f(1.0f, -1.0f, 1.0f);

        frontBlock.Normal3f(0.0f, 0.0f, 1.0f);
        frontBlock.MultiTexCoord2f(0, 1.0f, 1.0f);
        frontBlock.Vertex3f(1.0f, 1.0f, 1.0f);

        frontBlock.Normal3f(0.0f, 0.0f, 1.0f);
        frontBlock.MultiTexCoord2f(0, 0.0f, 1.0f);
        frontBlock.Vertex3f(-1.0f, 1.0f, 1.0f);
    frontBlock.End();

    // Tworzenie lewego boku
    leftBlock.Begin(GL_TRIANGLE_FAN, 4, 1);
        leftBlock.Normal3f(-1.0f, 0.0f, 0.0f);
        leftBlock.MultiTexCoord2f(0, 0.0f, 0.0f);
        leftBlock.Vertex3f(-1.0f, -1.0f, -1.0f);

        leftBlock.Normal3f(-1.0f, 0.0f, 0.0f);
        leftBlock.MultiTexCoord2f(0, 1.0f, 0.0f);
        leftBlock.Vertex3f(-1.0f, -1.0f, 1.0f);

        leftBlock.Normal3f(-1.0f, 0.0f, 0.0f);
        leftBlock.MultiTexCoord2f(0, 1.0f, 1.0f);
        leftBlock.Vertex3f(-1.0f, 1.0f, 1.0f);

        leftBlock.Normal3f(-1.0f, 0.0f, 0.0f);
        leftBlock.MultiTexCoord2f(0, 0.0f, 1.0f);
        leftBlock.Vertex3f(-1.0f, 1.0f, -1.0f);
    leftBlock.End();

    // Tworzenie macierzy rzutowania cienia
    GLfloat floorPlane[] = { 0.0f, 1.0f, 0.0f, 1.0f};
    m3dMakePlanarShadowMatrix(shadowMatrix, floorPlane, vLightPos);

    // Za³adowanie czterech tekstur
    glGenTextures(4, textures);
        
    // Pod³oga drewniana
    pBytes = gltReadTGABits("floor.tga", &nWidth, &nHeight, &nComponents, &format);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D,0,nComponents,nWidth, nHeight, 0,
        format, GL_UNSIGNED_BYTE, pBytes);
    free(pBytes);

    // Jeden z boków bloku
    pBytes = gltReadTGABits("Block4.tga", &nWidth, &nHeight, &nComponents, &format);
    glBindTexture(GL_TEXTURE_2D, textures[1]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D,0,nComponents,nWidth, nHeight, 0,
        format, GL_UNSIGNED_BYTE, pBytes);
    free(pBytes);

    // Inna œciana bloku
    pBytes = gltReadTGABits("Block5.tga", &nWidth, &nHeight, &nComponents, &format);
        glBindTexture(GL_TEXTURE_2D, textures[2]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D,0,nComponents,nWidth, nHeight, 0,
        format, GL_UNSIGNED_BYTE, pBytes);
    free(pBytes);

    // Kolejny bok bloku
    pBytes = gltReadTGABits("Block6.tga", &nWidth, &nHeight, &nComponents, &format);
        glBindTexture(GL_TEXTURE_2D, textures[3]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D,0,nComponents,nWidth, nHeight, 0,
        format, GL_UNSIGNED_BYTE, pBytes);
    free(pBytes);
}




///////////////////////////////////////////////////////////////////////////////
// Renderowanie bloku
void RenderBlock(void)
{
    GLfloat vRed[] = { 1.0f, 0.0f, 0.0f, 1.0f};
    GLfloat vWhite[] = { 1.0f, 1.0f, 1.0f, 1.0f };

    switch(nStep)
    {
        // Rama 
        case 0:
            glEnable(GL_BLEND);
            glEnable(GL_LINE_SMOOTH);
            shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vRed);
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glDisable(GL_CULL_FACE);

            // Rysowanie szeœcianu
            cubeBatch.Draw();

            break;

        // Rama, ale nie tylna strona... chcemy, aby blok by³ tak¿e w buforze szablonu
        case 1:
            shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vRed);

            // Rysowanie (z ty³u) bloku w buforze szablonu
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glEnable(GL_STENCIL_TEST);
            glStencilFunc(GL_NEVER, 0, 0);
            glStencilOp(GL_INCR, GL_INCR, GL_INCR);
            glFrontFace(GL_CW);
            cubeBatch.Draw();
            glFrontFace(GL_CCW);
            glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
            glDisable(GL_STENCIL_TEST);

            glEnable(GL_BLEND);
            glEnable(GL_LINE_SMOOTH);
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

            // Rysowanie przodu
            cubeBatch.Draw();
            break;

        // Solid
        case 2:
            shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vRed);
            
            // Rysowanie szeœcianu
            cubeBatch.Draw();
            break;

        // Lit
        case 3:
            shaderManager.UseStockShader(GLT_SHADER_POINT_LIGHT_DIFF, modelViewMatrix.GetMatrix(),
                projectionMatrix.GetMatrix(), vLightPos, vRed);

            // Rysowanie szeœcianu
            cubeBatch.Draw();
            break;

        // Teksturowany
        case 4:
        case 5:
        default:
            glBindTexture(GL_TEXTURE_2D, textures[2]);
            shaderManager.UseStockShader(GLT_SHADER_TEXTURE_POINT_LIGHT_DIFF, modelViewMatrix.GetMatrix(),
                projectionMatrix.GetMatrix(), vLightPos, vWhite, 0);

            glBindTexture(GL_TEXTURE_2D, textures[1]);
            topBlock.Draw();
            glBindTexture(GL_TEXTURE_2D, textures[2]);
            frontBlock.Draw();
            glBindTexture(GL_TEXTURE_2D, textures[3]);
            leftBlock.Draw();

            break;
    }
    
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_STENCIL_TEST);
}

///////////////////////////////////////////////////////////////////////////////
// Renderowanie pod³ogi
void RenderFloor(void)
{
    GLfloat vBrown [] = { 0.55f, 0.292f, 0.09f, 1.0f};
    GLfloat vFloor[] = { 1.0f, 1.0f, 1.0f, 0.6f };

    switch(nStep)
        {
        // Rama 
        case 0:
            glEnable(GL_BLEND);
            glEnable(GL_LINE_SMOOTH);
            shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBrown);
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glDisable(GL_CULL_FACE);
            break;

        // Rama, ale nie tylna strona.. i tylko, gdy szablon == 0
        case 1:
            glEnable(GL_BLEND);
            glEnable(GL_LINE_SMOOTH);

            glEnable(GL_STENCIL_TEST);
            glStencilFunc(GL_EQUAL, 0, 0xff);

            shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBrown);
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            break;

        // Solid
        case 2:
        case 3:	
            shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBrown);
            break;

        // Teksturowany
        case 4:
        case 5:
        default:
            glBindTexture(GL_TEXTURE_2D, textures[0]);
            shaderManager.UseStockShader(GLT_SHADER_TEXTURE_MODULATE, transformPipeline.GetModelViewProjectionMatrix(), vFloor, 0);
            break;
        }
    
    // Rysowanie pod³ogi
    floorBatch.Draw();

    
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_STENCIL_TEST);
}



///////////////////////////////////////////////////////////////////////////////
// Rysuje scenê
void RenderScene(RenderContext *rcx)
{
    // Czyœci okno bie¿¹cym kolorem czyszczenia
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    modelViewMatrix.PushMatrix();
        M3DMatrix44f mCamera;
        cameraFrame.GetCameraMatrix(mCamera);
        modelViewMatrix.MultMatrix(mCamera);

        // Krok odbicia... rysowanie szeœcianu do góry nogami z
        // pod³og¹ na jego górnej p³aszczyŸnie
        if(nStep == 5)
        {
            glDisable(GL_CULL_FACE);
            modelViewMatrix.PushMatrix();
            modelViewMatrix.Scale(1.0f, -1.0f, 1.0f);
            modelViewMatrix.Translate(0.0f, 2.0f, 0.0f);
            modelViewMatrix.Rotate(35.0f, 0.0f, 1.0f, 0.0f);
            RenderBlock();
            modelViewMatrix.PopMatrix();
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            RenderFloor();
            glDisable(GL_BLEND);			
        }

        modelViewMatrix.PushMatrix();
            // Rysowanie normalne
            modelViewMatrix.Rotate(35.0f, 0.0f, 1.0f, 0.0f);
            RenderBlock();
        modelViewMatrix.PopMatrix();
        
        
    // Jeœli nie odbicie, rysuje pod³ogê na koñcu
    if(nStep != 5)
        RenderFloor();

        
    modelViewMatrix.PopMatrix();


    // Opró¿nienie bufora poleceñ rysowania
    glXSwapBuffers(rcx->dpy, rcx->win);    
}


///////////////////////////////////////////////////////////////////////////////
// Naciœniêto normalny klawisz znaku ASCII.
// W takim razie, jeœli zostanie naciœniêta spacja, w³¹cz nastêpn¹ scenê
void KeyPressFunc(unsigned char key)
{
    if(key == 65)
    {
        nStep++;

        if(nStep > 5)
            nStep = 0;
    }
}

///////////////////////////////////////////////////////////////////////////////
// Zmieni³ siê rozmiar okna  albo okno zosta³o dopiero utworzone. W obu przypadkach musimy
// u¿yæ rozmiarów okna do ustawienia widoku i macierzy rzutowania.
void ChangeSize(int w, int h)
{
    glViewport(0, 0, w, h);
    viewFrustum.SetPerspective(35.0f, float(w) / float(h), 1.0f, 500.0f);
    projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
    modelViewMatrix.LoadIdentity();
}

void Cleanup(RenderContext *rcx)
{
    glDeleteTextures(4,textures);

    // Zerwanie wi¹zania przed usuniêciem
    glXMakeCurrent(rcx->dpy, None, NULL);

    glXDestroyContext(rcx->dpy, rcx->ctx);
    rcx->ctx = NULL;

    XDestroyWindow(rcx->dpy, rcx->win);
    rcx->win = (Window)NULL;

    XCloseDisplay(rcx->dpy);
    rcx->dpy = 0;
}

///////////////////////////////////////////////////////////////////////////////
// Punkt wejœciowy programu
int main(int argc, char* argv[])
{
    Bool bWinMapped = False;
    RenderContext rcx;

    gltSetWorkingDirectory(argv[0]);
    GLuint vertexArrayObject;

    EarlyInitGLXfnPointers();

    // Ustawienie pocz¹tkowego rozmiaru okna
    rcx.nWinWidth  = 800;
    rcx.nWinHeight = 600;

    // Konfiguracja okna X i kontekstu GLX

    CreateWindow(&rcx);
    SetupRC(&rcx);
    ChangeSize(rcx.nWinWidth, rcx.nWinHeight);

    // Rysowanie pierwszej klatki przed sprawdzeniem komunikatów
    RenderScene(&rcx);

    // Pêtla bêdzie wykonywana przez ca³y czas dzia³ania programu
    for(;;)
    {
        XEvent newEvent;
        XWindowAttributes winData;

        // Pilnuje nowych zdarzeñ X
        XNextEvent(rcx.dpy, &newEvent);

        switch(newEvent.type)
        {
        case UnmapNotify:
            bWinMapped = False;
            break;
        case MapNotify :
            bWinMapped = True;
        case ConfigureNotify:
            XGetWindowAttributes(rcx.dpy, rcx.win, &winData);
            rcx.nWinHeight = winData.height;
            rcx.nWinWidth = winData.width;
            ChangeSize(rcx.nWinWidth, rcx.nWinHeight);
            break;
        case KeyPress:
            KeyPressFunc(newEvent.xkey.keycode);
            break;
        case DestroyNotify:
            Cleanup(&rcx);
            exit(0);
            break;
        }

        if(bWinMapped)
        {
            RenderScene(&rcx);
        }
    }

    Cleanup(&rcx);
    
    return 0;
}
