// Orthographic.cpp
// OpenGL. Ksi�ga eksperta
// Demonstruje rzutowanie prostopad�e OpenGL
// Autor programu: Richard S. Wright Jr.
#include <GLTools.h>	// Biblioteka OpenGL
#include <GLMatrixStack.h>
#include <GLFrame.h>
#include <GLFrustum.h>
#include <GLGeometryTransform.h>
#include <GLBatch.h>

#include <math.h>
#ifdef __APPLE__
#include <glut/glut.h>
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>
#endif


GLFrame             viewFrame;
GLFrustum           viewFrustum;
GLBatch             tubeBatch;
GLBatch             innerBatch;
GLMatrixStack       modelViewMatix;
GLMatrixStack       projectionMatrix;
GLGeometryTransform transformPipeline;
GLShaderManager     shaderManager;



// Rysowanie sceny
void RenderScene(void)
	{
	// Czyszczenie okna i bufor g��bi
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

		
    modelViewMatix.PushMatrix(viewFrame);
            
    GLfloat vRed[] = { 1.0f, 0.0f, 0.0f, 1.0f };
    GLfloat vGray[] = { 0.75f, 0.75f, 0.75f, 1.0f };
    shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(), transformPipeline.GetProjectionMatrix(), vRed);
    tubeBatch.Draw();


    shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(), transformPipeline.GetProjectionMatrix(), vGray);
    innerBatch.Draw();

    modelViewMatix.PopMatrix();


    glutSwapBuffers();
	}

// Funkcja dokonuj�c� inicjalizacji w kontek�cie renderowania. 
void SetupRC()
	{
	// Czarne t�o
	glClearColor(0.0f, 0.0f, 0.75f, 1.0f );

//    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    shaderManager.InitializeStockShaders();
  
    
    tubeBatch.Begin(GL_QUADS, 200);
    
    float fZ = 100.0f;
    float bZ = -100.0f;

    tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
    tubeBatch.Normal3f(0.0f, 0.0f, 1.0f);
    tubeBatch.Vertex3f(-50.0f, 50.0f, 100.0f);
    
    tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
    tubeBatch.Normal3f(0.0f, 0.0f, 1.0f);
    tubeBatch.Vertex3f(-50.0f, -50.0f, fZ);

    tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
    tubeBatch.Normal3f(0.0f, 0.0f, 1.0f);
    tubeBatch.Vertex3f(-35.0f, -50.0f, fZ);
    
    tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
    tubeBatch.Normal3f(0.0f, 0.0f, 1.0f);
    tubeBatch.Vertex3f(-35.0f,50.0f,fZ);
    
    // Prawy p�at
    tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
    tubeBatch.Normal3f(0.0f, 0.0f, 1.0f);
    tubeBatch.Vertex3f(50.0f, 50.0f, fZ);
    
    tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
    tubeBatch.Normal3f(0.0f, 0.0f, 1.0f);
    tubeBatch.Vertex3f(35.0f, 50.0f, fZ);
    
    tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
    tubeBatch.Normal3f(0.0f, 0.0f, 1.0f);
    tubeBatch.Vertex3f(35.0f, -50.0f, fZ);

    tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
    tubeBatch.Normal3f(0.0f, 0.0f, 1.0f);
    tubeBatch.Vertex3f(50.0f,-50.0f,fZ);
    
    // G�rny p�at
    tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
    tubeBatch.Normal3f(0.0f, 0.0f, 1.0f);
    tubeBatch.Vertex3f(-35.0f, 50.0f, fZ);
    
    tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
    tubeBatch.Normal3f(0.0f, 0.0f, 1.0f);
    tubeBatch.Vertex3f(-35.0f, 35.0f, fZ);
    
    tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
    tubeBatch.Normal3f(0.0f, 0.0f, 1.0f);
    tubeBatch.Vertex3f(35.0f, 35.0f, fZ);

    tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
    tubeBatch.Normal3f(0.0f, 0.0f, 1.0f);
    tubeBatch.Vertex3f(35.0f, 50.0f,fZ);
    
    // Dolny p�at
    tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
    tubeBatch.Normal3f(0.0f, 0.0f, 1.0f);
    tubeBatch.Vertex3f(-35.0f, -35.0f, fZ);
    
    tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
    tubeBatch.Normal3f(0.0f, 0.0f, 1.0f);
    tubeBatch.Vertex3f(-35.0f, -50.0f, fZ);
    
    tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
    tubeBatch.Normal3f(0.0f, 0.0f, 1.0f);
    tubeBatch.Vertex3f(35.0f, -50.0f, fZ);
    
    tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
    tubeBatch.Normal3f(0.0f, 0.0f, 1.0f);
    tubeBatch.Vertex3f(35.0f, -35.0f,fZ);
    
    
    // Punkty normalnej na osi Y
    tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
    tubeBatch.Normal3f(0.0f, 1.0f, 0.0f);
    tubeBatch.Vertex3f(-50.0f, 50.0f, fZ);

    tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
    tubeBatch.Normal3f(0.0f, 1.0f, 0.0f);
    tubeBatch.Vertex3f(50.0f, 50.0f, fZ);
    
    tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
    tubeBatch.Normal3f(0.0f, 1.0f, 0.0f);
    tubeBatch.Vertex3f(50.0f, 50.0f, bZ);
    
    tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
    tubeBatch.Normal3f(0.0f, 1.0f, 0.0f);
    tubeBatch.Vertex3f(-50.0f,50.0f,bZ);
    
    // Dolna cz��
    tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
    tubeBatch.Normal3f(0.0f, -1.0f, 0.0f);
    tubeBatch.Vertex3f(-50.0f, -50.0f, fZ);

    tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
    tubeBatch.Normal3f(0.0f, -1.0f, 0.0f);
    tubeBatch.Vertex3f(-50.0f, -50.0f, bZ);
    
    tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
    tubeBatch.Normal3f(0.0f, -1.0f, 0.0f);
    tubeBatch.Vertex3f(50.0f, -50.0f, bZ);
    
    tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
    tubeBatch.Normal3f(0.0f, -1.0f, 0.0f);
    tubeBatch.Vertex3f(50.0f, -50.0f, fZ);
    
    // Lewa cz��
    tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
    tubeBatch.Normal3f(1.0f, 0.0f, 0.0f);
    tubeBatch.Vertex3f(50.0f, 50.0f, fZ);
    
    tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
    tubeBatch.Normal3f(1.0f, 0.0f, 0.0f);
    tubeBatch.Vertex3f(50.0f, -50.0f, fZ);
    
    tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
    tubeBatch.Normal3f(1.0f, 0.0f, 0.0f);
    tubeBatch.Vertex3f(50.0f, -50.0f, bZ);
    
    tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
    tubeBatch.Normal3f(1.0f, 0.0f, 0.0f);
    tubeBatch.Vertex3f(50.0f, 50.0f, bZ);
    
    // Prawa cz��
    tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
    tubeBatch.Normal3f(-1.0f, 0.0f, 0.0f);
    tubeBatch.Vertex3f(-50.0f, 50.0f, fZ);
    
    tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
    tubeBatch.Normal3f(-1.0f, 0.0f, 0.0f);
    tubeBatch.Vertex3f(-50.0f, 50.0f, bZ);
    
    tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
    tubeBatch.Normal3f(-1.0f, 0.0f, 0.0f);
    tubeBatch.Vertex3f(-50.0f, -50.0f, bZ);

    tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
    tubeBatch.Normal3f(-1.0f, 0.0f, 0.0f);
    tubeBatch.Vertex3f(-50.0f, -50.0f, fZ);
        
    
    // Kierunek osi z
    // Lewy p�at
    tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
    tubeBatch.Normal3f(0.0f, 0.0f, 1.0f);	
    tubeBatch.Vertex3f(-50.0f, 50.0f, fZ);
    
    tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
    tubeBatch.Normal3f(0.0f, 0.0f, 1.0f);	
    tubeBatch.Vertex3f(-50.0f, -50.0f, fZ);
    
        tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
    tubeBatch.Normal3f(0.0f, 0.0f, 1.0f);	
    tubeBatch.Vertex3f(-35.0f, -50.0f, fZ);
    
        tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
    tubeBatch.Normal3f(0.0f, 0.0f, 1.0f);	
    tubeBatch.Vertex3f(-35.0f,50.0f,fZ);
    
    // Prawy p�at
        tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
    tubeBatch.Normal3f(0.0f, 0.0f, 1.0f);	
    tubeBatch.Vertex3f(50.0f, 50.0f, fZ);
    
        tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
    tubeBatch.Normal3f(0.0f, 0.0f, 1.0f);	
    tubeBatch.Vertex3f(35.0f, 50.0f, fZ);
    
        tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
    tubeBatch.Normal3f(0.0f, 0.0f, 1.0f);	
    tubeBatch.Vertex3f(35.0f, -50.0f, fZ);
    
        tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
    tubeBatch.Normal3f(0.0f, 0.0f, 1.0f);	
    tubeBatch.Vertex3f(50.0f,-50.0f,fZ);
    
    // G�rny p�at
        tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
    tubeBatch.Normal3f(0.0f, 0.0f, 1.0f);	
    tubeBatch.Vertex3f(-35.0f, 50.0f, fZ);
    
        tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
    tubeBatch.Normal3f(0.0f, 0.0f, 1.0f);	
    tubeBatch.Vertex3f(-35.0f, 35.0f, fZ);
    
        tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
    tubeBatch.Normal3f(0.0f, 0.0f, 1.0f);	
    tubeBatch.Vertex3f(35.0f, 35.0f, fZ);

        tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
    tubeBatch.Normal3f(0.0f, 0.0f, 1.0f);	
    tubeBatch.Vertex3f(35.0f, 50.0f,fZ);
    
    // Dolny p�at
        tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
    tubeBatch.Normal3f(0.0f, 0.0f, 1.0f);	
    tubeBatch.Vertex3f(-35.0f, -35.0f, fZ);
    
        tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
    tubeBatch.Normal3f(0.0f, 0.0f, 1.0f);	
    tubeBatch.Vertex3f(-35.0f, -50.0f, fZ);
    
        tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
    tubeBatch.Normal3f(0.0f, 0.0f, 1.0f);	
    tubeBatch.Vertex3f(35.0f, -50.0f, fZ);
    
        tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
    tubeBatch.Normal3f(0.0f, 0.0f, 1.0f);	
    tubeBatch.Vertex3f(35.0f, -35.0f,fZ);
        
    // D�ugo�� wierzcho�ka ////////////////////////////
    // Punkty normalnej na osi Y
        tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
    tubeBatch.Normal3f(0.0f, 1.0f, 0.0f);
    tubeBatch.Vertex3f(-50.0f, 50.0f, fZ);

        tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
    tubeBatch.Normal3f(0.0f, 1.0f, 0.0f);
    tubeBatch.Vertex3f(50.0f, 50.0f, fZ);

        tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
    tubeBatch.Normal3f(0.0f, 1.0f, 0.0f);
    tubeBatch.Vertex3f(50.0f, 50.0f, bZ);

        tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
    tubeBatch.Normal3f(0.0f, 1.0f, 0.0f);
    tubeBatch.Vertex3f(-50.0f,50.0f,bZ);
    
    // Dolna cz��
        tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
    tubeBatch.Normal3f(0.0f, -1.0f, 0.0f);
    tubeBatch.Vertex3f(-50.0f, -50.0f, fZ);

        tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
    tubeBatch.Normal3f(0.0f, -1.0f, 0.0f);
    tubeBatch.Vertex3f(-50.0f, -50.0f, bZ);

        tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
    tubeBatch.Normal3f(0.0f, -1.0f, 0.0f);
    tubeBatch.Vertex3f(50.0f, -50.0f, bZ);

        tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
    tubeBatch.Normal3f(0.0f, -1.0f, 0.0f);
    tubeBatch.Vertex3f(50.0f, -50.0f, fZ);
    
    // Lewa cz��
        tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
    tubeBatch.Normal3f(1.0f, 0.0f, 0.0f);
    tubeBatch.Vertex3f(50.0f, 50.0f, fZ);

        tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
    tubeBatch.Normal3f(1.0f, 0.0f, 0.0f);
    tubeBatch.Vertex3f(50.0f, -50.0f, fZ);

        tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
    tubeBatch.Normal3f(1.0f, 0.0f, 0.0f);
    tubeBatch.Vertex3f(50.0f, -50.0f, bZ);

        tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
    tubeBatch.Normal3f(1.0f, 0.0f, 0.0f);
    tubeBatch.Vertex3f(50.0f, 50.0f, bZ);
    
    // Prawa cz��
        tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
    tubeBatch.Normal3f(-1.0f, 0.0f, 0.0f);
    tubeBatch.Vertex3f(-50.0f, 50.0f, fZ);

        tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
    tubeBatch.Normal3f(-1.0f, 0.0f, 0.0f);
    tubeBatch.Vertex3f(-50.0f, 50.0f, bZ);

        tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
    tubeBatch.Normal3f(-1.0f, 0.0f, 0.0f);
    tubeBatch.Vertex3f(-50.0f, -50.0f, bZ);

        tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
    tubeBatch.Normal3f(-1.0f, 0.0f, 0.0f);
    tubeBatch.Vertex3f(-50.0f, -50.0f, fZ);



        // Lewy p�at
        tubeBatch.Normal3f(0.0f, 0.0f, -1.0f);	
        tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
        tubeBatch.Vertex3f(-35.0f,50.0f,bZ);

        tubeBatch.Normal3f(0.0f, 0.0f, -1.0f);	
        tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
        tubeBatch.Vertex3f(-35.0f, -50.0f, bZ);
        
        tubeBatch.Normal3f(0.0f, 0.0f, -1.0f);	
        tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
        tubeBatch.Vertex3f(-50.0f, -50.0f, bZ);
        
        tubeBatch.Normal3f(0.0f, 0.0f, -1.0f);	
        tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
        tubeBatch.Vertex3f(-50.0f, 50.0f, bZ);
        
        // Prawy p�at
        tubeBatch.Normal3f(0.0f, 0.0f, -1.0f);	
        tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);

        tubeBatch.Vertex3f(50.0f,-50.0f,bZ);

        tubeBatch.Normal3f(0.0f, 0.0f, -1.0f);	
        tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);

        tubeBatch.Vertex3f(35.0f, -50.0f, bZ);

        tubeBatch.Normal3f(0.0f, 0.0f, -1.0f);	
        tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);

        tubeBatch.Vertex3f(35.0f, 50.0f, bZ);
        
        tubeBatch.Normal3f(0.0f, 0.0f, -1.0f);	
        tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
        
        tubeBatch.Vertex3f(50.0f, 50.0f, bZ);
        
        // G�rny p�at
        tubeBatch.Normal3f(0.0f, 0.0f, -1.0f);	
        tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
        tubeBatch.Vertex3f(35.0f, 50.0f, bZ);
        tubeBatch.Normal3f(0.0f, 0.0f, -1.0f);	
        tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
        tubeBatch.Vertex3f(35.0f, 35.0f, bZ);
        tubeBatch.Normal3f(0.0f, 0.0f, -1.0f);	
        tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
        tubeBatch.Vertex3f(-35.0f, 35.0f, bZ);


        tubeBatch.Normal3f(0.0f, 0.0f, -1.0f);	
        tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
        tubeBatch.Vertex3f(-35.0f, 50.0f, bZ);
    
        // Dolny p�at
        tubeBatch.Normal3f(0.0f, 0.0f, -1.0f);	
        tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
        tubeBatch.Vertex3f(35.0f, -35.0f,bZ);
        tubeBatch.Normal3f(0.0f, 0.0f, -1.0f);	
        tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
        tubeBatch.Vertex3f(35.0f, -50.0f, bZ);
        tubeBatch.Normal3f(0.0f, 0.0f, -1.0f);	
        tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
        tubeBatch.Vertex3f(-35.0f, -50.0f, bZ);


        tubeBatch.Normal3f(0.0f, 0.0f, -1.0f);	
        tubeBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
        tubeBatch.Vertex3f(-35.0f, -35.0f, bZ);
    
        tubeBatch.End();


        innerBatch.Begin(GL_QUADS, 40);
 

        
        // �ciany wewn�trzne /////////////////////////////
        // Punkty normalnej na osi Y
        innerBatch.Color4f(0.75f, 0.75f, 0.75f, 1.0f);
        innerBatch.Normal3f(0.0f, 1.0f, 0.0f);
        innerBatch.Vertex3f(-35.0f, 35.0f, fZ);
        innerBatch.Color4f(0.75f, 0.75f, 0.75f, 1.0f);
        innerBatch.Normal3f(0.0f, 1.0f, 0.0f);
        innerBatch.Vertex3f(35.0f, 35.0f, fZ);
        innerBatch.Color4f(0.75f, 0.75f, 0.75f, 1.0f);
        innerBatch.Normal3f(0.0f, 1.0f, 0.0f);
        innerBatch.Vertex3f(35.0f, 35.0f, bZ);
        innerBatch.Color4f(0.75f, 0.75f, 0.75f, 1.0f);
        innerBatch.Normal3f(0.0f, 1.0f, 0.0f);
        innerBatch.Vertex3f(-35.0f,35.0f,bZ);
		
        // Dolna cz��
        innerBatch.Color4f(0.75f, 0.75f, 0.75f, 1.0f);
        innerBatch.Normal3f(0.0f, 1.0f, 0.0f);
        innerBatch.Vertex3f(-35.0f, -35.0f, fZ);
        innerBatch.Color4f(0.75f, 0.75f, 0.75f, 1.0f);
        innerBatch.Normal3f(0.0f, 1.0f, 0.0f);
        innerBatch.Vertex3f(-35.0f, -35.0f, bZ);
        innerBatch.Color4f(0.75f, 0.75f, 0.75f, 1.0f);
        innerBatch.Normal3f(0.0f, 1.0f, 0.0f);
        innerBatch.Vertex3f(35.0f, -35.0f, bZ);
        innerBatch.Color4f(0.75f, 0.75f, 0.75f, 1.0f);
        innerBatch.Normal3f(0.0f, 1.0f, 0.0f);
        innerBatch.Vertex3f(35.0f, -35.0f, fZ);
        
        // Lewa cz��
        innerBatch.Color4f(0.75f, 0.75f, 0.75f, 1.0f);
        innerBatch.Normal3f(1.0f, 0.0f, 0.0f);
        innerBatch.Vertex3f(-35.0f, 35.0f, fZ);
        innerBatch.Color4f(0.75f, 0.75f, 0.75f, 1.0f);
        innerBatch.Normal3f(1.0f, 0.0f, 0.0f);
        innerBatch.Vertex3f(-35.0f, 35.0f, bZ);
        innerBatch.Color4f(0.75f, 0.75f, 0.75f, 1.0f);
        innerBatch.Normal3f(1.0f, 0.0f, 0.0f);
        innerBatch.Vertex3f(-35.0f, -35.0f, bZ);
        innerBatch.Color4f(0.75f, 0.75f, 0.75f, 1.0f);
        innerBatch.Normal3f(1.0f, 0.0f, 0.0f);
        innerBatch.Vertex3f(-35.0f, -35.0f, fZ);
        
        // Prawa cz��
        innerBatch.Color4f(0.75f, 0.75f, 0.75f, 1.0f);
        innerBatch.Normal3f(-1.0f, 0.0f, 0.0f);
        innerBatch.Vertex3f(35.0f, 35.0f, fZ);
        innerBatch.Color4f(0.75f, 0.75f, 0.75f, 1.0f);
        innerBatch.Normal3f(-1.0f, 0.0f, 0.0f);
        innerBatch.Vertex3f(35.0f, -35.0f, fZ);
        innerBatch.Color4f(0.75f, 0.75f, 0.75f, 1.0f);
        innerBatch.Normal3f(-1.0f, 0.0f, 0.0f);
        innerBatch.Vertex3f(35.0f, -35.0f, bZ);
        innerBatch.Color4f(0.75f, 0.75f, 0.75f, 1.0f);
        innerBatch.Normal3f(-1.0f, 0.0f, 0.0f);
        innerBatch.Vertex3f(35.0f, 35.0f, bZ);
        
        innerBatch.End();

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

    viewFrustum.SetOrthographic(-130.0f, 130.0f, -130.0f, 130.0f, -130.0f, 130.0f);
    
    projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
    transformPipeline.SetMatrixStacks(modelViewMatix, projectionMatrix);
	}

///////////////////////////////////////////////////////////////////////////////
// Punkt rozpocz�cia dzia�ania programu
int main(int argc, char* argv[])
    {
	gltSetWorkingDirectory(argv[0]);
	
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Rzutowanie prostopad�e");
    glutReshapeFunc(ChangeSize);
    glutSpecialFunc(SpecialKeys);
    glutDisplayFunc(RenderScene);
        
    
	GLenum err = glewInit();
	if (GLEW_OK != err) {
		fprintf(stderr, "B��d GLEW: %s\n", glewGetErrorString(err));
		return 1;
    }
	
	SetupRC();
    
	glutMainLoop();
	return 0;
    }
