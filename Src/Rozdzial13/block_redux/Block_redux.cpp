// Block_redux.cpp
// OpenGL. Ksi�ga eksperta, Rozdzia� 13
// Demonstruje podstwowe techniki konfiguracji okna w systemie Windows
// Autor programu: Richard S. Wright Jr. and Nick Haemel

#include <GLTools.h>	// OpenGL
#include <GLMatrixStack.h>
#include <GLFrame.h>
#include <GLFrustum.h>
#include <GLBatch.h>
#include <GLGeometryTransform.h>
#include <gl\wglew.h>

#include <math.h>

/////////////////////////////////////////////////////////////////////////////////
// Potrzebne klasy
GLShaderManager		shaderManager;
GLMatrixStack		modelViewMatrix;
GLMatrixStack		projectionMatrix;
GLFrame				cameraFrame;
GLFrustum			viewFrustum;
GLBatch				cubeBatch;
GLBatch				floorBatch;
GLBatch				topBlock;
GLBatch				frontBlock;
GLBatch				leftBlock;

GLGeometryTransform	transformPipeline;
M3DMatrix44f		shadowMatrix;

HWND         g_hWnd;
HGLRC        g_hRC;
HDC          g_hDC;
HINSTANCE    g_hInstance;
WNDCLASS     g_windClass; 
RECT         g_windowRect;
bool         g_ContinueRendering;

int nStep = 0;

// Dane o�wietlenia
GLfloat lightAmbient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
GLfloat lightDiffuse[] = { 0.7f, 0.7f, 0.7f, 1.0f };
GLfloat lightSpecular[] = { 0.9f, 0.9f, 0.9f };
GLfloat vLightPos[] = { -8.0f, 20.0f, 100.0f, 1.0f };

GLuint textures[4];


///////////////////////////////////////////////////////////////////////////////
// Tworzenie sze�cianu z ka�dej serii tr�jk�t�w. Wsp�rz�dne teksturowe i normalne
// r�wnie� s� dostarczone.
void MakeCube(GLBatch& cubeBatch)
{
    cubeBatch.Begin(GL_TRIANGLES, 36, 1);

    /////////////////////////////////////////////
    // Wierzch sze�cianu
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
    // Podstawa sze�cianu
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
    // Lewy bok sze�cianu
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

    // Prawy bok sze�cianu
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

    // Prz�d i ty�
    // Prz�d
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

    // Ty�
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
// Tworzenie pod�ogi, tylko wierzcho�ki i tekstura, �adnych normalnych
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


///////////////////////////////////////////////////////////////////////////////
// Funkcja inicjalizuj�ca kontekst renderowania 
// Pierwsza okazja do wykonania jakich� czynno�ci zwi�zanych z OpenGL
void SetupRC()
{
    GLbyte *pBytes;
    GLint nWidth, nHeight, nComponents;
    GLenum format;

    shaderManager.InitializeStockShaders();

    // Czarne t�o
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

    // Tworzenie g�ry
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

    // Za�adowanie czterech tekstur
    glGenTextures(4, textures);
        
    // Pod�oga drewniana
    pBytes = gltReadTGABits("floor.tga", &nWidth, &nHeight, &nComponents, &format);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D,0,nComponents,nWidth, nHeight, 0,
        format, GL_UNSIGNED_BYTE, pBytes);
    free(pBytes);

    // Jeden z bok�w bloku
    pBytes = gltReadTGABits("Block4.tga", &nWidth, &nHeight, &nComponents, &format);
    glBindTexture(GL_TEXTURE_2D, textures[1]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D,0,nComponents,nWidth, nHeight, 0,
        format, GL_UNSIGNED_BYTE, pBytes);
    free(pBytes);

    // Inna �ciana bloku
    pBytes = gltReadTGABits("block5.tga", &nWidth, &nHeight, &nComponents, &format);
        glBindTexture(GL_TEXTURE_2D, textures[2]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D,0,nComponents,nWidth, nHeight, 0,
        format, GL_UNSIGNED_BYTE, pBytes);
    free(pBytes);

    // Kolejny bok bloku
    pBytes = gltReadTGABits("block6.tga", &nWidth, &nHeight, &nComponents, &format);
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

            // Rysowanie sze�cianu
            cubeBatch.Draw();

            break;

        // Rama, ale nie tylna �ciana... chcemy, aby blok by� tak�e w buforze szablonu
        case 1:
            shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vRed);

            // Rysowanie (z ty�u) bloku w buforze szablonu
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
            
            // Rysowanie sze�cianu
            cubeBatch.Draw();
            break;

        // Lit
        case 3:
            shaderManager.UseStockShader(GLT_SHADER_POINT_LIGHT_DIFF, modelViewMatrix.GetMatrix(),
                projectionMatrix.GetMatrix(), vLightPos, vRed);

            // Rysowanie sze�cianu
            cubeBatch.Draw();
            break;

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
// Renderowanie pod�ogi
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
    
    // Rysowanie pod�ogi
    floorBatch.Draw();

    
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_STENCIL_TEST);
}



///////////////////////////////////////////////////////////////////////////////
// Rysuje scen�
void RenderScene(void)
{
    if(!g_ContinueRendering)
        return;

    // Czy�ci okno bie��cym kolorem czyszczenia
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    modelViewMatrix.PushMatrix();
        M3DMatrix44f mCamera;
        cameraFrame.GetCameraMatrix(mCamera);
        modelViewMatrix.MultMatrix(mCamera);

        // Krok odbicia... rysowanie sze�cianu do g�ry nogami z
        // pod�og� na jego g�rnej p�aszczy�nie
        if(nStep == 5) {
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
        
    // Je�li nie odbicie, rysuje pod�og� na ko�cu
    if(nStep != 5)
        RenderFloor();

    modelViewMatrix.PopMatrix();

    // Opr�nienie bufora polece� rysowania
    SwapBuffers(g_hDC);
}


///////////////////////////////////////////////////////////////////////////////
// Zmieni� si� rozmiar okna  albo okno zosta�o dopiero utworzone. W obu przypadkach musimy
// u�y� rozmiar�w okna do ustawienia widoku i macierzy rzutowania.
void ChangeSize(int w, int h)
{
    glViewport(0, 0, w, h);
    viewFrustum.SetPerspective(35.0f, float(w) / float(h), 1.0f, 500.0f);
    projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
    modelViewMatrix.LoadIdentity();
}

///////////////////////////////////////////////////////////////////////////////
// Funkcje zwrotne do obs�ugi wszystkich funkcji okna, kt�rymi zajmuje si� ta aplikacja
// Po zako�czeniu, przekazujemy komunikat do nast�pnej aplikacji
LRESULT CALLBACK WndProc(	HWND	hWnd,		// Uchwyt do tego okna
                            UINT	uMsg,		// Komunikat dla tego okna
                            WPARAM	wParam,		// Dodatkowe informacje komunikatu
                            LPARAM	lParam)		// Dodatkowe informacje komunikatu
{
    unsigned int key = 0;
    
    // Indywidualna obs�uga poszczeg�lnych komunikat�w
    switch(uMsg)
    {
    case WM_ACTIVATE:
    case WM_SETFOCUS:
        RenderScene();
        return 0;
    case WM_SIZE:
        ChangeSize(LOWORD(lParam),HIWORD(lParam));
        RenderScene();
        break;
    case WM_CLOSE:
        g_ContinueRendering = false;
        PostQuitMessage(0);
        return 0;
    case WM_KEYDOWN:
        key = (unsigned int)wParam;
        if(key == 32)
        {
            nStep++;
            if(nStep > 5)
                nStep = 0;
        }
        return 0;
    default:
        // Nie ma nic do roboty
        break;
    }

    // Przekazanie wszystkich nieobs�u�onych komunikat�w do DefWindowProc
    return DefWindowProc(hWnd,uMsg,wParam,lParam);
}



///////////////////////////////////////////////////////////////////////////////
// Konfiguracja okna i jego stanu.
// Utworzenie okna, znalezienie formatu piksela i utworzenie kontekstu OpenGL
bool SetupWindow(int nWidth, int nHeight)
{
    bool bRetVal = true;

    int nWindowX = 0;
    int nWindowY = 0;
    int nPixelFormat  = -1;
    PIXELFORMATDESCRIPTOR pfd;

    DWORD dwExtStyle;
    DWORD dwWindStyle;

    HINSTANCE g_hInstance = GetModuleHandle(NULL);

    TCHAR szWindowName[50] =  TEXT("Block Redux");
    TCHAR szClassName[50]  =  TEXT("OGL_CLASS");

    // Klasa okna
    g_windClass.lpszClassName = szClassName;                // Nazwa klasy
    g_windClass.lpfnWndProc   = (WNDPROC)WndProc;
    g_windClass.hInstance     = g_hInstance;                // U�yj tego modu�u dla uchwytu modu�u
    g_windClass.hCursor       = LoadCursor(NULL, IDC_ARROW);/// Wyb�r domy�lnego kurosra myszy
    g_windClass.hIcon         = LoadIcon(NULL, IDI_WINLOGO);// Wyb�r domy�lnych ikon okien
    g_windClass.hbrBackground = NULL;                       // Brak t�a
    g_windClass.lpszMenuName  = NULL;                       // To okno nie ma menu
    g_windClass.style         = CS_HREDRAW | CS_OWNDC |     // Ustawienie styl�w dla tej klasy, a konkretnie w celu
                                CS_VREDRAW;                 // reprezentacji operacji ponownego rysowania okna, unikatowych DC i zmian rozmiaru
    g_windClass.cbClsExtra    = 0;                          // Dodatkowa pami�� klasy
    g_windClass.cbWndExtra    = 0;                          // Dodatkowa pami�� okna

    // Rejestracja nowej klasy
    if(!RegisterClass( &g_windClass ))
        bRetVal = false;

    dwExtStyle  = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
    dwWindStyle = WS_OVERLAPPEDWINDOW;
    ShowCursor(TRUE);

    g_windowRect.left   = nWindowX;
    g_windowRect.right  = nWindowX + nWidth;
    g_windowRect.top    = nWindowY;
    g_windowRect.bottom = nWindowY + nHeight;

    // Ustawienie szeroko�ci i wysoko�ci okna
    AdjustWindowRectEx(&g_windowRect, dwWindStyle, FALSE, dwExtStyle);

    // Dostosowanie do ozd�b
    int nWindowWidth = g_windowRect.right   - g_windowRect.left;
    int nWindowHeight = g_windowRect.bottom - g_windowRect.top;

    // Utworzenie okna
    g_hWnd = CreateWindowEx(dwExtStyle,     // Rozszerzony styl
                            szClassName,    // Nazwa klasy
                            szWindowName,   // Nazwa okna
                            dwWindStyle |        
                            WS_CLIPSIBLINGS | 
                            WS_CLIPCHILDREN,// Styl okna
                            nWindowX,       // Po�o�enie okna, x
                            nWindowY,       // Po�o�enie okna, y
                            nWindowWidth,   // Wysoko��
                            nWindowHeight,  // Szeroko��
                            NULL,           // Okno nadrz�dne
                            NULL,           // menu
                            g_hInstance,    // Egzemplarz
                            NULL);          // Przeka� to do WM_CREATE

    // Mamy ju� okno, wi�c przechodzimy do deskryptora formatu pikseli
    g_hDC = GetDC(g_hWnd);

    // Ustawienie zast�pczego formatu pikseli, aby uzyska� dost�p do funkcji WGL
    SetPixelFormat( g_hDC, 1,&pfd);
    // Utworzenie kontekstu OGL i ustawienie go jako bie��cy
    g_hRC = wglCreateContext( g_hDC );
    wglMakeCurrent( g_hDC, g_hRC );

    if (g_hDC == 0 ||
        g_hDC == 0)
    {
        bRetVal = false;
        printf("!!! Podczas tworzenia okna OpenGL wyst�pi� b��d.\n");
    }

    // Ustawienie biblioteki GLEW, kt�ra �aduje wska�niki funkcji OGL
    GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		/* Problem: wywo�anie funkcji glewInit nie powiod�o si� */
        bRetVal = false;
		printf("B��d: %s\n", glewGetErrorString(err));
	}
    const GLubyte *oglVersion = glGetString(GL_VERSION);
    printf("This system supports OpenGL Version %s.\n", oglVersion);

    // Rozszerzenia s� skonfigurowane. Usuwamy okno i zaczynamy od nowa wyb�r odpowiedniego formatu
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(g_hRC);
    ReleaseDC(g_hWnd, g_hDC);
    DestroyWindow(g_hWnd);

    // Create the window again
    g_hWnd = CreateWindowEx(dwExtStyle,     // Rozszerzony styl
                            szClassName,    // Nazwa klasy
                            szWindowName,   // Nazwa okna
                            dwWindStyle |        
                            WS_CLIPSIBLINGS | 
                            WS_CLIPCHILDREN,// Styl okna
                            nWindowX,       // Po�o�enie okna, x
                            nWindowY,       // Po�o�enie okna, y
                            nWindowWidth,   // Wysoko��
                            nWindowHeight,  // Szeroko��
                            NULL,           // Okno nadrz�dne
                            NULL,           // menu
                            g_hInstance,    // Egzemplarz
                            NULL);          // Przeka� to do WM_CREATE

    g_hDC = GetDC(g_hWnd);

    int nPixCount = 0;

    // Okre�lenie interesuj�cych nas atrybut�w
    int pixAttribs[] = { WGL_SUPPORT_OPENGL_ARB, 1, // Musi obs�ugiwa� rendering OGL
                         WGL_DRAW_TO_WINDOW_ARB, 1, // Format pikseli, kt�rego mo�na u�ywa� w oknie
                         WGL_ACCELERATION_ARB,   1, // Musi mie� wsparcie sprz�towe
                         WGL_RED_BITS_ARB,       8, // 8 bit�w dla kana�u czerwonego
                         WGL_GREEN_BITS_ARB,     8, // 8 bit�w dla kana�u zielonego
                         WGL_BLUE_BITS_ARB,      8, // 8 bit�w dla kana�u niebieskiego
                         WGL_DEPTH_BITS_ARB,     16, // 16 bit�w g��bi
                         WGL_PIXEL_TYPE_ARB,      WGL_TYPE_RGBA_ARB, // Format pikseli powinien by� typu RGBA
                         0}; // Zerowe zako�czenie

    // ��damy, aby OpenGL znalaz�a najlepszy format odpowiadaj�cy naszym wymaganiom
    // W odpowiedzi otrzymujemy tylko jeden format
    wglChoosePixelFormatARB(g_hDC, &pixAttribs[0], NULL, 1, &nPixelFormat, (UINT*)&nPixCount);

    if(nPixelFormat == -1) 
    {
        // Nie mo�na znale�� formatu
        g_hDC = 0;
        g_hDC = 0;
        bRetVal = false;
        printf("!!! Wyst�pi� b��d podczas szukania formatu pikseli o podanych atrybutach.\n");
    }
    else
    {
        // Mamy format, wi�c ustawiamy go jako bie��cy
        SetPixelFormat( g_hDC, nPixelFormat, &pfd );

        GLint attribs[] = {WGL_CONTEXT_MAJOR_VERSION_ARB,  3,
	                       WGL_CONTEXT_MINOR_VERSION_ARB,  1,
                           0 };
        
        g_hRC = wglCreateContextAttribsARB(g_hDC, 0, attribs);
        if (g_hRC == NULL)
        {
            printf("!!! Nie mo�na utworzy� kontekstu OpenGL 3.1.\n");
            attribs[3] = 0;
            g_hRC = wglCreateContextAttribsARB(g_hDC, 0, attribs);
            if (g_hRC == NULL)
            {
                printf("!!! Nie mo�na utworzy� kontekstu OpenGL 3.0.\n");
                printf("!!! Biblioteka OpenGL 3.0 i jej nowsze wersje nie s� obs�ugiwane.\n");
            }
        }

        wglMakeCurrent( g_hDC, g_hRC );
    }

    if (g_hDC == 0 ||
        g_hDC == 0)
    {
        bRetVal = false;
        printf("!!! Podczas tworzenia okna OpenGL wyst�pi� b��d.\n");
    }

    // Je�li wszystko posz�o zgodnie z planem, wy�wietl okno
    if( bRetVal )
    {
        ShowWindow( g_hWnd, SW_SHOW );
        SetForegroundWindow( g_hWnd );
        SetFocus( g_hWnd );
        g_ContinueRendering = true;
    }
    
    return bRetVal;
}

///////////////////////////////////////////////////////////////////////////////
// Porz�dkowanie okna, kontekstu OGL i zwi�zanego z nim stanu
// Wywo�ywana przy zamykaniu programu i po wyst�pieniu b��du
bool KillWindow( )
{ 
    bool bRetVal = true;

    glDeleteTextures(4,textures);
    
    // Czyszczenie OGL RC
    if(g_hRC) 
    {
        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(g_hRC);
        g_hRC = NULL;
    }

    // Zwolnienie DC
    if(g_hDC)
    {
        ReleaseDC(g_hWnd, g_hDC);
        g_hDC = NULL;
    }

    // Usuni�cie okna
    if(g_hWnd)
    {
        DestroyWindow(g_hWnd);
        g_hWnd = NULL;;
    }

    // Usuni�cie klasy okna
    TCHAR szClassName[50]  =  TEXT("OGL_CLASS");
    UnregisterClass(szClassName, g_hInstance);
    g_hInstance = NULL;
    ShowCursor(TRUE);
    return bRetVal;
}

///////////////////////////////////////////////////////////////////////////////
// G��wna p�tla renderuj�ca
// Sprawdza komunikaty okna i obs�uguje zdarzenia, a tak�e rysuje scen�
void mainLoop()
{
    MSG		msg;

    // Sprawdzenie komunikat�w
    if (PeekMessage(&msg,NULL,0,0,PM_REMOVE))
    {
	    if (msg.message==WM_QUIT)
	    {
            g_ContinueRendering = false;
	    }
	    else
	    {
            // Obs�uga komunikat�w
		    TranslateMessage(&msg);
		    DispatchMessage(&msg);
	    }
    }
    else
    {
        RenderScene();       
    }
}

///////////////////////////////////////////////////////////////////////////////
// G��wna p�tla renderuj�ca
// Sprawdza komunikaty okna i obs�uguje zdarzenia, a tak�e rysuje scen�
int main(int argc, char* argv[])
{
    if(SetupWindow(800, 600))
    {
        SetupRC();
        ChangeSize(800, 600);
        while (g_ContinueRendering)
        {   
            mainLoop();
            Sleep(0);
        }
    }
    KillWindow();
	return 0;
}

